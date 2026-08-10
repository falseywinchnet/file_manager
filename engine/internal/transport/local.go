package transport

import (
	"context"
	"crypto/rand"
	"crypto/subtle"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net"
	"os"
	"path/filepath"
	"sync"
	"time"

	"filemanager/engine/internal/service"
)

const (
	LocalProtocol = "engine.local.v1"
	frameMagic    = "ENG1"
	frameHeader   = 8
)

type EndpointFiles struct {
	RuntimeDir string
	QuerySock  string
	AdminSock  string
	QueryToken string
	AdminToken string
	Discovery  string
}

type Discovery struct {
	Protocol       string    `json:"protocol"`
	InstanceID     string    `json:"instance_id"`
	UID            int       `json:"uid"`
	StartedAt      time.Time `json:"started_at"`
	QuerySocket    string    `json:"query_socket"`
	QueryTokenFile string    `json:"query_token_file"`
	AdminSocket    string    `json:"admin_socket"`
	AdminTokenFile string    `json:"admin_token_file"`
}

type hello struct {
	Protocol  string    `json:"protocol"`
	Authority Authority `json:"authority"`
	Token     string    `json:"token"`
}

type helloResponse struct {
	Accepted bool   `json:"accepted"`
	Protocol string `json:"protocol"`
	Error    string `json:"error,omitempty"`
}

func Files(runtimeDir string) EndpointFiles {
	return EndpointFiles{
		RuntimeDir: runtimeDir,
		QuerySock:  filepath.Join(runtimeDir, "query.sock"), AdminSock: filepath.Join(runtimeDir, "admin.sock"),
		QueryToken: filepath.Join(runtimeDir, "query.token"), AdminToken: filepath.Join(runtimeDir, "admin.token"),
		Discovery: filepath.Join(runtimeDir, "discovery.json"),
	}
}

// ServeLocal creates distinct same-user query/admin endpoints. Credentials are
// random per process instance and are never accepted on the other endpoint.
func ServeLocal(ctx context.Context, runtimeDir string, engine *service.Service) error {
	if engine == nil {
		return errors.New("engine service is required")
	}
	files := Files(runtimeDir)
	if err := verifyRuntimeDirectory(runtimeDir); err != nil {
		return err
	}
	queryToken, err := rotateToken(files.QueryToken)
	if err != nil {
		return err
	}
	adminToken, err := rotateToken(files.AdminToken)
	if err != nil {
		return err
	}
	query, err := listenPrivate(files.QuerySock)
	if err != nil {
		return err
	}
	defer query.Close()
	admin, err := listenPrivate(files.AdminSock)
	if err != nil {
		return err
	}
	defer admin.Close()
	defer cleanupRuntime(files)
	discovery := Discovery{
		Protocol: LocalProtocol, InstanceID: engine.Version().InstanceID, UID: os.Getuid(), StartedAt: time.Now().UTC(),
		QuerySocket: files.QuerySock, QueryTokenFile: files.QueryToken,
		AdminSocket: files.AdminSock, AdminTokenFile: files.AdminToken,
	}
	if err := writePrivateJSON(files.Discovery, discovery); err != nil {
		return err
	}
	serverCtx, cancel := context.WithCancel(ctx)
	defer cancel()
	var once sync.Once
	shutdown := func() { once.Do(cancel) }
	errorsOut := make(chan error, 2)
	go serveEndpoint(serverCtx, query, engine, AuthorityQuery, queryToken, shutdown, errorsOut)
	go serveEndpoint(serverCtx, admin, engine, AuthorityAdmin, adminToken, shutdown, errorsOut)
	select {
	case <-serverCtx.Done():
		_ = query.Close()
		_ = admin.Close()
		if errors.Is(serverCtx.Err(), context.Canceled) {
			return nil
		}
		return serverCtx.Err()
	case err := <-errorsOut:
		cancel()
		_ = query.Close()
		_ = admin.Close()
		return err
	}
}

func serveEndpoint(ctx context.Context, listener net.Listener, engine *service.Service, authority Authority, token string, shutdown func(), errorsOut chan<- error) {
	for {
		connection, err := listener.Accept()
		if err != nil {
			if ctx.Err() != nil || errors.Is(err, net.ErrClosed) {
				return
			}
			select {
			case errorsOut <- err:
			default:
			}
			return
		}
		go handleLocal(ctx, connection, engine, authority, token, shutdown)
	}
}

func handleLocal(ctx context.Context, connection net.Conn, engine *service.Service, authority Authority, token string, shutdown func()) {
	defer connection.Close()
	_ = connection.SetDeadline(time.Now().Add(30 * time.Second))
	uid, err := peerUID(connection)
	if err != nil || uid != os.Getuid() {
		return
	}
	var incoming hello
	if err := readFrame(connection, &incoming); err != nil {
		return
	}
	accepted := incoming.Protocol == LocalProtocol && incoming.Authority == authority && constantToken(incoming.Token, token)
	response := helloResponse{Accepted: accepted, Protocol: LocalProtocol}
	if !accepted {
		response.Error = "session authentication failed"
	}
	if err := writeFrame(connection, response); err != nil || !accepted {
		return
	}
	_ = connection.SetDeadline(time.Time{})
	for {
		var request Request
		if err := readFrame(connection, &request); err != nil {
			return
		}
		requestCtx, cancel := context.WithTimeout(ctx, 30*time.Second)
		outgoing, stop := dispatchAuthorized(requestCtx, engine, request, authority)
		cancel()
		if err := writeFrame(connection, outgoing); err != nil {
			return
		}
		if stop {
			shutdown()
			return
		}
	}
}

func CallLocal(ctx context.Context, runtimeDir string, authority Authority, request Request) (Response, error) {
	files := Files(runtimeDir)
	socket, tokenPath := files.QuerySock, files.QueryToken
	if authority == AuthorityAdmin {
		socket, tokenPath = files.AdminSock, files.AdminToken
	} else if authority != AuthorityQuery {
		return Response{}, errors.New("authority must be query or admin")
	}
	token, err := os.ReadFile(tokenPath)
	if err != nil {
		return Response{}, fmt.Errorf("read endpoint token: %w", err)
	}
	dialer := net.Dialer{}
	connection, err := dialer.DialContext(ctx, "unix", socket)
	if err != nil {
		return Response{}, err
	}
	defer connection.Close()
	if deadline, ok := ctx.Deadline(); ok {
		_ = connection.SetDeadline(deadline)
	}
	if err := writeFrame(connection, hello{Protocol: LocalProtocol, Authority: authority, Token: string(token)}); err != nil {
		return Response{}, err
	}
	var accepted helloResponse
	if err := readFrame(connection, &accepted); err != nil {
		return Response{}, err
	}
	if !accepted.Accepted {
		return Response{}, errors.New(accepted.Error)
	}
	if err := writeFrame(connection, request); err != nil {
		return Response{}, err
	}
	var response Response
	if err := readFrame(connection, &response); err != nil {
		return Response{}, err
	}
	return response, nil
}

func readFrame(reader io.Reader, target any) error {
	header := make([]byte, frameHeader)
	if _, err := io.ReadFull(reader, header); err != nil {
		return err
	}
	if string(header[:4]) != frameMagic {
		return errors.New("invalid Engine frame magic")
	}
	size := binary.BigEndian.Uint32(header[4:])
	if size == 0 || size > MaxJSONLFrameBytes {
		return errors.New("invalid Engine frame size")
	}
	payload := make([]byte, size)
	if _, err := io.ReadFull(reader, payload); err != nil {
		return err
	}
	decoder := json.NewDecoder(bytesReader(payload))
	if err := decoder.Decode(target); err != nil {
		return err
	}
	var trailing any
	if err := decoder.Decode(&trailing); err != io.EOF {
		return errors.New("Engine frame contains trailing JSON")
	}
	return nil
}

func writeFrame(writer io.Writer, value any) error {
	payload, err := json.Marshal(value)
	if err != nil {
		return err
	}
	if len(payload) == 0 || len(payload) > MaxJSONLFrameBytes {
		return errors.New("Engine frame exceeds the bounded payload")
	}
	header := make([]byte, frameHeader)
	copy(header, frameMagic)
	binary.BigEndian.PutUint32(header[4:], uint32(len(payload)))
	if _, err := writer.Write(header); err != nil {
		return err
	}
	_, err = writer.Write(payload)
	return err
}

func bytesReader(payload []byte) *byteReader { return &byteReader{payload: payload} }

type byteReader struct{ payload []byte }

func (r *byteReader) Read(p []byte) (int, error) {
	if len(r.payload) == 0 {
		return 0, io.EOF
	}
	n := copy(p, r.payload)
	r.payload = r.payload[n:]
	return n, nil
}

func rotateToken(path string) (string, error) {
	raw := make([]byte, 32)
	if _, err := rand.Read(raw); err != nil {
		return "", err
	}
	token := hex.EncodeToString(raw)
	if err := writePrivate(path, []byte(token)); err != nil {
		return "", err
	}
	return token, nil
}

func constantToken(left, right string) bool {
	return len(left) == len(right) && subtle.ConstantTimeCompare([]byte(left), []byte(right)) == 1
}

func verifyRuntimeDirectory(path string) error {
	info, err := os.Stat(path)
	if err != nil {
		return err
	}
	if !info.IsDir() || info.Mode().Perm()&0o077 != 0 {
		return errors.New("runtime directory must be private")
	}
	return nil
}

func listenPrivate(path string) (net.Listener, error) {
	if info, err := os.Lstat(path); err == nil {
		if info.Mode()&os.ModeSocket == 0 {
			return nil, errors.New("refusing to replace a non-socket endpoint")
		}
		probe, probeErr := net.DialTimeout("unix", path, 100*time.Millisecond)
		if probeErr == nil {
			probe.Close()
			return nil, errors.New("Engine endpoint is already accepting connections")
		}
		if err := os.Remove(path); err != nil {
			return nil, err
		}
	} else if !errors.Is(err, os.ErrNotExist) {
		return nil, err
	}
	listener, err := net.Listen("unix", path)
	if err != nil {
		return nil, err
	}
	if err := os.Chmod(path, 0o600); err != nil {
		listener.Close()
		return nil, err
	}
	return listener, nil
}

func writePrivateJSON(path string, value any) error {
	payload, err := json.MarshalIndent(value, "", "  ")
	if err != nil {
		return err
	}
	payload = append(payload, '\n')
	return writePrivate(path, payload)
}

func writePrivate(path string, payload []byte) error {
	temporary := path + ".new"
	file, err := os.OpenFile(temporary, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0o600)
	if err != nil {
		return err
	}
	if _, err = file.Write(payload); err == nil {
		err = file.Sync()
	}
	closeErr := file.Close()
	if err == nil {
		err = closeErr
	}
	if err != nil {
		_ = os.Remove(temporary)
		return err
	}
	if err := os.Chmod(temporary, 0o600); err != nil {
		_ = os.Remove(temporary)
		return err
	}
	return os.Rename(temporary, path)
}

func cleanupRuntime(files EndpointFiles) {
	for _, path := range []string{files.QuerySock, files.AdminSock, files.QueryToken, files.AdminToken, files.Discovery} {
		_ = os.Remove(path)
	}
}
