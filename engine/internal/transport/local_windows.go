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
	"time"

	winio "github.com/Microsoft/go-winio"
	"golang.org/x/sys/windows"

	"filemanager/engine/api"
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
	var native interface{ Fd() uintptr } = nil
	var ok bool = false
	native, ok = connection.(interface{ Fd() uintptr })
	if !ok {
		var failure error = errors.New("connection is not a Windows pipe")
		return 0, failure
	}
	var handle windows.Handle = windows.Handle(native.Fd())
	return handle, nil
}

func verifyPipePeer(connection net.Conn, server bool, expectedPID uint32) error {
	var handle windows.Handle = 0
	var err error = nil
	handle, err = pipeHandle(connection)
	if err != nil {
		return err
	}
	var pid uint32 = 0
	if server {
		err = windows.GetNamedPipeServerProcessId(handle, &pid)
	} else {
		err = windows.GetNamedPipeClientProcessId(handle, &pid)
	}
	if err != nil {
		return err
	}
	if expectedPID != 0 && pid != expectedPID {
		var failure error = errors.New("pipe server process differs from discovery")
		return failure
	}
	var peerSID string = ""
	peerSID, err = windowssecure.ProcessSID(pid)
	if err != nil {
		return err
	}
	var sid string = ""
	sid, err = windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	if peerSID != sid {
		var failure error = errors.New("pipe peer belongs to another user")
		return failure
	}
	return nil
}

func randomPipeToken() (string, error) {
	var raw []byte = make([]byte, 32)
	{
		var err error = nil
		_, err = rand.Read(raw)
		if err != nil {
			return "", err
		}
	}
	var result string = hex.EncodeToString(raw)
	return result, nil
}

func serveWindows(ctx context.Context, runtimeDir string, engine *service.Service, options LocalOptions) error {
	if engine == nil {
		var failure error = errors.New("engine service is required")
		return failure
	}
	var directory *os.File = nil
	var err error = nil
	directory, err = windowssecure.Open(runtimeDir, true)
	if err != nil {
		var failure error = fmt.Errorf("private runtime directory: %w", err)
		return failure
	}
	defer directory.Close()
	// A held exclusive file prevents two servers from rotating one discovery.
	var lock *os.File = nil
	lock, err = windowssecure.Lock(filepath.Join(runtimeDir, "server.lock"))
	if err != nil {
		var failure error = fmt.Errorf("runtime already in use: %w", err)
		return failure
	}
	defer lock.Close()
	var sid string = ""
	sid, err = windowssecure.CurrentSID()
	if err != nil {
		return err
	}
	var version api.VersionInfo = engine.Version()
	var instance string = version.InstanceID
	var queryPipe string = `\\.\pipe\filemanager-engine-` + instance + "-query"
	var adminPipe string = `\\.\pipe\filemanager-engine-` + instance + "-admin"
	var config *winio.PipeConfig = &winio.PipeConfig{SecurityDescriptor: "O:" + sid + "D:P(A;;GA;;;" + sid + ")", InputBufferSize: 65536, OutputBufferSize: 65536}
	var query net.Listener = nil
	query, err = winio.ListenPipe(queryPipe, config)
	if err != nil {
		return err
	}
	defer query.Close()
	var admin net.Listener = nil
	admin, err = winio.ListenPipe(adminPipe, config)
	if err != nil {
		return err
	}
	defer admin.Close()
	var files EndpointFiles = Files(runtimeDir)
	var queryToken string = ""
	queryToken, err = randomPipeToken()
	if err != nil {
		return err
	}
	var adminToken string = ""
	adminToken, err = randomPipeToken()
	if err != nil {
		return err
	}
	{
		var err error = nil
		err = windowssecure.Write(files.QueryToken, []byte(queryToken))
		if err != nil {
			return err
		}
	}
	defer os.Remove(files.QueryToken)
	{
		var err error = nil
		err = windowssecure.Write(files.AdminToken, []byte(adminToken))
		if err != nil {
			return err
		}
	}
	defer os.Remove(files.AdminToken)
	var discovery WindowsDiscovery = WindowsDiscovery{Protocol: LocalProtocol, Transport: "windows_named_pipe", InstanceID: instance,
		UserSID: sid, ServerPID: uint32(os.Getpid()), QueryPipe: queryPipe, AdminPipe: adminPipe,
		QueryTokenFile: files.QueryToken, AdminTokenFile: files.AdminToken}
	var payload []byte = nil
	payload, err = json.MarshalIndent(discovery, "", "  ")
	if err != nil {
		return err
	}
	{
		var err error = nil
		payload = append(payload, '\n')
		err = windowssecure.Write(files.Discovery, payload)
		if err != nil {
			return err
		}
	}
	defer os.Remove(files.Discovery)
	var serverCtx context.Context = nil
	var cancel context.CancelFunc = nil
	serverCtx, cancel = context.WithCancel(ctx)
	defer cancel()
	var connections *connectionSet = newConnectionSet()
	var errorsOut chan error = make(chan error, 2)
	var acceptors, handlers sync.WaitGroup = sync.WaitGroup{}, sync.WaitGroup{}
	acceptors.Add(2)
	var queryTask endpointTask = endpointTask{
		context: serverCtx, listener: query, engine: engine, authority: AuthorityQuery,
		token: queryToken, maximumConnections: maxQueryConnections, connections: connections,
		handlers: &handlers, acceptors: &acceptors, shutdown: cancel, errorsOut: errorsOut, options: options,
	}
	var adminTask endpointTask = endpointTask{
		context: serverCtx, listener: admin, engine: engine, authority: AuthorityAdmin,
		token: adminToken, maximumConnections: maxAdminConnections, connections: connections,
		handlers: &handlers, acceptors: &acceptors, shutdown: cancel, errorsOut: errorsOut, options: options,
	}
	go queryTask.Run()
	go adminTask.Run()
	var serveErr error = nil
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
	var discovery WindowsDiscovery = WindowsDiscovery{}
	var directory *os.File = nil
	var err error = nil
	directory, err = windowssecure.Open(runtimeDir, true)
	if err != nil {
		return discovery, err
	}
	defer directory.Close()
	var payload []byte = nil
	var files EndpointFiles = Files(runtimeDir)
	payload, err = windowssecure.Read(files.Discovery, 16384)
	if err != nil {
		return discovery, err
	}
	{
		var err error = nil
		err = json.Unmarshal(payload, &discovery)
		if err != nil {
			return discovery, err
		}
	}
	var sid string = ""
	sid, err = windowssecure.CurrentSID()
	if err != nil {
		return discovery, err
	}
	if discovery.Protocol != LocalProtocol || discovery.Transport != "windows_named_pipe" || discovery.UserSID != sid || discovery.ServerPID == 0 || len(discovery.InstanceID) != 32 {
		var failure error = errors.New("invalid Windows Engine discovery identity")
		return discovery, failure
	}
	{
		var err error = nil
		_, err = hex.DecodeString(discovery.InstanceID)
		if err != nil {
			return discovery, err
		}
	}
	var expected string = `\\.\pipe\filemanager-engine-` + discovery.InstanceID
	if discovery.QueryPipe != expected+"-query" || discovery.AdminPipe != expected+"-admin" ||
		!strings.EqualFold(discovery.QueryTokenFile, files.QueryToken) || !strings.EqualFold(discovery.AdminTokenFile, files.AdminToken) {
		var failure error = errors.New("Windows Engine discovery paths differ from private endpoint layout")
		return discovery, failure
	}
	return discovery, nil
}

func callWindows(ctx context.Context, runtimeDir string, authority Authority, request Request) (Response, error) {
	if authority != AuthorityQuery && authority != AuthorityAdmin {
		var emptyResponse Response = Response{}
		var failure error = errors.New("authority must be query or admin")
		return emptyResponse, failure
	}
	var directory *os.File = nil
	var err error = nil
	directory, err = windowssecure.Open(runtimeDir, true)
	if err != nil {
		var emptyResponse Response = Response{}
		return emptyResponse, err
	}
	defer directory.Close()
	var discovery WindowsDiscovery = WindowsDiscovery{}
	discovery, err = loadWindowsDiscovery(runtimeDir)
	if err != nil {
		var emptyResponse Response = Response{}
		return emptyResponse, err
	}
	var pipe string = ""
	var tokenFile string = ""
	pipe, tokenFile = discovery.QueryPipe, discovery.QueryTokenFile
	if authority == AuthorityAdmin {
		pipe, tokenFile = discovery.AdminPipe, discovery.AdminTokenFile
	}
	var token []byte = nil
	token, err = windowssecure.Read(tokenFile, 64)
	if err != nil {
		var emptyResponse Response = Response{}
		return emptyResponse, err
	}
	if len(token) != 64 {
		var emptyResponse Response = Response{}
		var failure error = errors.New("invalid endpoint token length")
		return emptyResponse, failure
	}
	{
		var err error = nil
		_, err = hex.DecodeString(string(token))
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	var bounded context.Context = nil
	var cancel context.CancelFunc = nil
	bounded, cancel = context.WithTimeout(ctx, requestTimeout+handshakeTimeout)
	defer cancel()
	var connection net.Conn = nil
	connection, err = winio.DialPipeAccessImpLevel(bounded, pipe, windows.GENERIC_READ|windows.GENERIC_WRITE, winio.PipeImpLevelIdentification)
	if err != nil {
		var emptyResponse Response = Response{}
		return emptyResponse, err
	}
	defer connection.Close()
	var cancellation connectionCancellation = connectionCancellation{
		connection: connection, finished: make(chan struct{}), stop: nil,
	}
	cancellation.stop = context.AfterFunc(bounded, cancellation.CloseConnection)
	defer cancellation.Revoke()
	{
		var err error = nil
		err = verifyPipePeer(connection, true, discovery.ServerPID)
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	var deadline time.Time = time.Time{}
	deadline, _ = bounded.Deadline()
	{
		var err error = nil
		err = connection.SetDeadline(deadline)
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	{
		var err error = nil
		err = writeFrame(connection, hello{Protocol: LocalProtocol, Authority: authority, Token: string(token)})
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	var accepted helloResponse = helloResponse{}
	{
		var err error = nil
		err = readFrame(connection, &accepted)
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	if !accepted.Accepted || accepted.Protocol != LocalProtocol {
		var emptyResponse Response = Response{}
		var failure error = errors.New("Engine handshake rejected")
		return emptyResponse, failure
	}
	{
		var err error = nil
		err = writeFrame(connection, request)
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	var response Response = Response{}
	{
		var err error = nil
		err = readFrame(connection, &response)
		if err != nil {
			var emptyResponse Response = Response{}
			return emptyResponse, err
		}
	}
	if response.ID != request.ID {
		var emptyResponse Response = Response{}
		var failure error = errors.New("Engine response request identity mismatch")
		return emptyResponse, failure
	}
	return response, nil
}

// connectionCancellation borrows the caller-owned connection. Revoke either
// prevents execution or joins the callback before the caller releases its state.
// Close is intentionally safe to repeat with the caller's deferred Close.
type connectionCancellation struct {
	connection net.Conn
	finished   chan struct{}
	stop       func() bool
}

func (cancellation *connectionCancellation) CloseConnection() {
	defer close(cancellation.finished)
	_ = cancellation.connection.Close()
}

func (cancellation *connectionCancellation) Revoke() {
	var prevented bool = cancellation.stop()
	if !prevented {
		<-cancellation.finished
	}
	cancellation.stop = nil
}
