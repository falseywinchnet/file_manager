//go:build windows

package transport

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"
	"sync"

	winio "github.com/Microsoft/go-winio"
	"golang.org/x/sys/windows"

	"filemanager/engine/internal/service"
	"filemanager/engine/internal/windowssecure"
)

type WindowsDiscovery struct {
	Protocol       string `json:"protocol"`
	Transport      string `json:"transport"`
	InstanceID     string `json:"instance_id"`
	UserSID        string `json:"user_sid"`
	ServerPID      uint32 `json:"server_pid"`
	QueryPipe      string `json:"query_pipe"`
	AdminPipe      string `json:"admin_pipe"`
	QueryTokenFile string `json:"query_token_file"`
	AdminTokenFile string `json:"admin_token_file"`
}

func pipeHandle(connection net.Conn) (windows.Handle, error) {
	native, ok := connection.(interface{ Fd() uintptr })
	if !ok {
		return 0, errors.New("connection is not a Windows pipe")
	}
	return windows.Handle(native.Fd()), nil
}

func verifyPipePeer(connection net.Conn, server bool, expectedPID uint32) error {
	handle, err := pipeHandle(connection)
	if err != nil {
		return err
	}
	var pid uint32
	if server {
		err = windows.GetNamedPipeServerProcessId(handle, &pid)
	} else {
		err = windows.GetNamedPipeClientProcessId(handle, &pid)
	}
	if err != nil {
		return err
	}
	if expectedPID != 0 && pid != expectedPID {
		return errors.New("pipe server process differs from discovery")
	}
	peerSID, err := windowssecure.ProcessSID(pid)
	if err != nil {
		return err
	}
	sid, err := windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	if peerSID != sid {
		return errors.New("pipe peer belongs to another user")
	}
	return nil
}

func randomPipeToken() (string, error) {
	raw := make([]byte, 32)
	if _, err := rand.Read(raw); err != nil {
		return "", err
	}
	return hex.EncodeToString(raw), nil
}

func serveWindows(ctx context.Context, runtimeDir string, engine *service.Service, options LocalOptions) error {
	if engine == nil {
		return errors.New("engine service is required")
	}
	directory, err := windowssecure.Open(runtimeDir, true)
	if err != nil {
		return fmt.Errorf("private runtime directory: %w", err)
	}
	defer directory.Close()
	// A held exclusive file prevents two servers from rotating one discovery.
	lock, err := windowssecure.Lock(filepath.Join(runtimeDir, "server.lock"))
	if err != nil {
		return fmt.Errorf("runtime already in use: %w", err)
	}
	defer lock.Close()
	sid, err := windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	instance := engine.Version().InstanceID
	queryPipe := `\\.\pipe\filemanager-engine-` + instance + "-query"
	adminPipe := `\\.\pipe\filemanager-engine-` + instance + "-admin"
	config := &winio.PipeConfig{SecurityDescriptor: "O:" + sid + "D:P(A;;GA;;;" + sid + ")", InputBufferSize: 65536, OutputBufferSize: 65536}
	query, err := winio.ListenPipe(queryPipe, config)
	if err != nil {
		return err
	}
	defer query.Close()
	admin, err := winio.ListenPipe(adminPipe, config)
	if err != nil {
		return err
	}
	defer admin.Close()
	files := Files(runtimeDir)
	queryToken, err := randomPipeToken()
	if err != nil {
		return err
	}
	adminToken, err := randomPipeToken()
	if err != nil {
		return err
	}
	if err := windowssecure.Write(files.QueryToken, []byte(queryToken)); err != nil {
		return err
	}
	defer os.Remove(files.QueryToken)
	if err := windowssecure.Write(files.AdminToken, []byte(adminToken)); err != nil {
		return err
	}
	defer os.Remove(files.AdminToken)
	discovery := WindowsDiscovery{Protocol: LocalProtocol, Transport: "windows_named_pipe", InstanceID: instance,
		UserSID: sid, ServerPID: uint32(os.Getpid()), QueryPipe: queryPipe, AdminPipe: adminPipe,
		QueryTokenFile: files.QueryToken, AdminTokenFile: files.AdminToken}
	payload, err := json.MarshalIndent(discovery, "", "  ")
	if err != nil {
		return err
	}
	if err := windowssecure.Write(files.Discovery, append(payload, '\n')); err != nil {
		return err
	}
	defer os.Remove(files.Discovery)
	serverCtx, cancel := context.WithCancel(ctx)
	defer cancel()
	connections := newConnectionSet()
	errorsOut := make(chan error, 2)
	var acceptors, handlers sync.WaitGroup
	acceptors.Add(2)
	go func() {
		defer acceptors.Done()
		serveEndpoint(serverCtx, query, engine, AuthorityQuery, queryToken, maxQueryConnections, connections, &handlers, cancel, errorsOut, options)
	}()
	go func() {
		defer acceptors.Done()
		serveEndpoint(serverCtx, admin, engine, AuthorityAdmin, adminToken, maxAdminConnections, connections, &handlers, cancel, errorsOut, options)
	}()
	var serveErr error
	select {
	case <-serverCtx.Done():
		serveErr = ctx.Err()
	case serveErr = <-errorsOut:
	}
	cancel()
	query.Close()
	admin.Close()
	acceptors.Wait()
	connections.CloseAll()
	handlers.Wait()
	return serveErr
}

func loadWindowsDiscovery(runtimeDir string) (WindowsDiscovery, error) {
	var discovery WindowsDiscovery
	directory, err := windowssecure.Open(runtimeDir, true)
	if err != nil {
		return discovery, err
	}
	defer directory.Close()
	payload, err := windowssecure.Read(Files(runtimeDir).Discovery, 16384)
	if err != nil {
		return discovery, err
	}
	if err := json.Unmarshal(payload, &discovery); err != nil {
		return discovery, err
	}
	sid, err := windowssecure.CurrentSID()
	if err != nil {
		return discovery, err
	}
	files := Files(runtimeDir)
	if discovery.Protocol != LocalProtocol || discovery.Transport != "windows_named_pipe" || discovery.UserSID != sid || discovery.ServerPID == 0 || len(discovery.InstanceID) != 32 {
		return discovery, errors.New("invalid Windows Engine discovery identity")
	}
	if _, err := hex.DecodeString(discovery.InstanceID); err != nil {
		return discovery, err
	}
	expected := `\\.\pipe\filemanager-engine-` + discovery.InstanceID
	if discovery.QueryPipe != expected+"-query" || discovery.AdminPipe != expected+"-admin" ||
		!strings.EqualFold(discovery.QueryTokenFile, files.QueryToken) || !strings.EqualFold(discovery.AdminTokenFile, files.AdminToken) {
		return discovery, errors.New("Windows Engine discovery paths differ from private endpoint layout")
	}
	return discovery, nil
}

func callWindows(ctx context.Context, runtimeDir string, authority Authority, request Request) (Response, error) {
	if authority != AuthorityQuery && authority != AuthorityAdmin {
		return Response{}, errors.New("authority must be query or admin")
	}
	directory, err := windowssecure.Open(runtimeDir, true)
	if err != nil {
		return Response{}, err
	}
	defer directory.Close()
	discovery, err := loadWindowsDiscovery(runtimeDir)
	if err != nil {
		return Response{}, err
	}
	pipe, tokenFile := discovery.QueryPipe, discovery.QueryTokenFile
	if authority == AuthorityAdmin {
		pipe, tokenFile = discovery.AdminPipe, discovery.AdminTokenFile
	}
	token, err := windowssecure.Read(tokenFile, 64)
	if err != nil {
		return Response{}, err
	}
	if len(token) != 64 {
		return Response{}, errors.New("invalid endpoint token length")
	}
	if _, err := hex.DecodeString(string(token)); err != nil {
		return Response{}, err
	}
	bounded, cancel := context.WithTimeout(ctx, requestTimeout+handshakeTimeout)
	defer cancel()
	connection, err := winio.DialPipeAccessImpLevel(bounded, pipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err != nil {
		return Response{}, err
	}
	defer connection.Close()
	stop := context.AfterFunc(bounded, func() { connection.Close() })
	defer stop()
	if err := verifyPipePeer(connection, true, discovery.ServerPID); err != nil {
		return Response{}, err
	}
	deadline, _ := bounded.Deadline()
	if err := connection.SetDeadline(deadline); err != nil {
		return Response{}, err
	}
	if err := writeFrame(connection, hello{Protocol: LocalProtocol, Authority: authority, Token: string(token)}); err != nil {
		return Response{}, err
	}
	var accepted helloResponse
	if err := readFrame(connection, &accepted); err != nil {
		return Response{}, err
	}
	if !accepted.Accepted || accepted.Protocol != LocalProtocol {
		return Response{}, errors.New("Engine handshake rejected")
	}
	if err := writeFrame(connection, request); err != nil {
		return Response{}, err
	}
	var response Response
	if err := readFrame(connection, &response); err != nil {
		return Response{}, err
	}
	if response.ID != request.ID {
		return Response{}, errors.New("Engine response request identity mismatch")
	}
	return response, nil
}
