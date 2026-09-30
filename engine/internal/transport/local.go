package transport

import (
	"bytes"
	"context"
	"crypto/rand"
	"crypto/sha256"
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
	"runtime"
	"sync"
	"time"

	"filemanager/engine/internal/service"
)

const (
	LocalProtocol = "engine.local.v1"
	frameMagic    = "ENG1"
	frameHeader   = 8

	maxQueryConnections = 32
	maxAdminConnections = 4
	handshakeTimeout    = 5 * time.Second
	idleFrameTimeout    = 30 * time.Second
	requestTimeout      = 30 * time.Second
	writeTimeout        = 5 * time.Second
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

type LocalOptions struct {
	// AfterSuccessfulDispatch extends the acknowledgement boundary for an
	// installed projection. Returning an error replaces the successful result
	// with a redacted internal fault; the client may safely retry.
	AfterSuccessfulDispatch func(Request, Response) error
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
	return ServeLocalWithOptions(ctx, runtimeDir, engine, LocalOptions{})
}

func ServeLocalWithOptions(ctx context.Context, runtimeDir string, engine *service.Service, options LocalOptions) error {
	if runtime.GOOS == "windows" {
		return serveWindows(ctx, runtimeDir, engine, options)
	}
	if engine == nil {
		return errors.New("engine service is required")
	}
	files := Files(runtimeDir)
	if err := verifyRuntimeDirectory(runtimeDir); err != nil {
		return err
	}
	query, err := listenPrivate(files.QuerySock)
	if err != nil {
		return err
	}
	defer query.Close()
	defer os.Remove(files.QuerySock)
	admin, err := listenPrivate(files.AdminSock)
	if err != nil {
		return err
	}
	defer admin.Close()
	defer os.Remove(files.AdminSock)
	defer cleanupRuntime(files)
	queryToken, err := rotateToken(files.QueryToken)
	if err != nil {
		return err
	}
	adminToken, err := rotateToken(files.AdminToken)
	if err != nil {
		return err
	}
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
	connections := newConnectionSet()
	var acceptors sync.WaitGroup
	var handlers sync.WaitGroup
	acceptors.Add(2)
	go func() {
		defer acceptors.Done()
		serveEndpoint(serverCtx, query, engine, AuthorityQuery, queryToken, maxQueryConnections, connections, &handlers, shutdown, errorsOut, options)
	}()
	go func() {
		defer acceptors.Done()
		serveEndpoint(serverCtx, admin, engine, AuthorityAdmin, adminToken, maxAdminConnections, connections, &handlers, shutdown, errorsOut, options)
	}()
	var serveErr error
	select {
	case <-serverCtx.Done():
		if ctx.Err() != nil {
			serveErr = ctx.Err()
		}
	case err := <-errorsOut:
		serveErr = err
	}
	cancel()
	_ = query.Close()
	_ = admin.Close()
	acceptors.Wait()
	connections.CloseAll()
	handlers.Wait()
	return serveErr
}

func serveEndpoint(
	ctx context.Context,
	listener net.Listener,
	engine *service.Service,
	authority Authority,
	token string,
	maximumConnections int,
	connections *connectionSet,
	handlers *sync.WaitGroup,
	shutdown func(),
	errorsOut chan<- error,
	options LocalOptions,
) {
	slots := make(chan struct{}, maximumConnections)
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
		select {
		case slots <- struct{}{}:
			connections.Add(connection)
			handlers.Add(1)
			go func() {
				defer handlers.Done()
				defer func() { <-slots }()
				defer connections.Remove(connection)
				handleLocal(ctx, connection, engine, authority, token, shutdown, options)
			}()
		default:
			_ = connection.Close()
		}
	}
}

type connectionSet struct {
	mu          sync.Mutex
	connections map[net.Conn]struct{}
}

func newConnectionSet() *connectionSet {
	return &connectionSet{connections: make(map[net.Conn]struct{})}
}

func (s *connectionSet) Add(connection net.Conn) {
	s.mu.Lock()
	s.connections[connection] = struct{}{}
	s.mu.Unlock()
}

func (s *connectionSet) Remove(connection net.Conn) {
	s.mu.Lock()
	delete(s.connections, connection)
	s.mu.Unlock()
}

func (s *connectionSet) CloseAll() {
	s.mu.Lock()
	connections := make([]net.Conn, 0, len(s.connections))
	for connection := range s.connections {
		connections = append(connections, connection)
	}
	s.mu.Unlock()
	for _, connection := range connections {
		_ = connection.Close()
	}
}

func handleLocal(ctx context.Context, connection net.Conn, engine *service.Service, authority Authority, token string, shutdown func(), options LocalOptions) {
	defer connection.Close()
	if err := connection.SetDeadline(time.Now().Add(handshakeTimeout)); err != nil {
		return
	}
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
	if err := connection.SetDeadline(time.Time{}); err != nil {
		return
	}
	for {
		if err := connection.SetReadDeadline(time.Now().Add(idleFrameTimeout)); err != nil {
			return
		}
		var request Request
		if err := readFrame(connection, &request); err != nil {
			return
		}
		requestCtx, cancel := context.WithTimeout(ctx, requestTimeout)
		outgoing, stop := dispatchAuthorized(requestCtx, engine, request, authority)
		cancel()
		if outgoing.Error == nil && options.AfterSuccessfulDispatch != nil {
			if err := options.AfterSuccessfulDispatch(request, outgoing); err != nil {
				outgoing.Result = nil
				outgoing.Error = publicFault(err)
				stop = false
			}
		}
		if err := connection.SetWriteDeadline(time.Now().Add(writeTimeout)); err != nil {
			return
		}
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
	if runtime.GOOS == "windows" {
		return callWindows(ctx, runtimeDir, authority, request)
	}
	files := Files(runtimeDir)
	socket, tokenPath := files.QuerySock, files.QueryToken
	if authority == AuthorityAdmin {
		socket, tokenPath = files.AdminSock, files.AdminToken
	} else if authority != AuthorityQuery {
		return Response{}, errors.New("authority must be query or admin")
	}
	token, err := readToken(tokenPath)
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
	if err := writeFrame(connection, hello{Protocol: LocalProtocol, Authority: authority, Token: token}); err != nil {
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
	decoder := json.NewDecoder(bytes.NewReader(payload))
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
	frame := make([]byte, frameHeader+len(payload))
	copy(frame, frameMagic)
	binary.BigEndian.PutUint32(frame[4:frameHeader], uint32(len(payload)))
	copy(frame[frameHeader:], payload)
	for len(frame) != 0 {
		written, writeErr := writer.Write(frame)
		if written > 0 {
			frame = frame[written:]
		}
		if writeErr != nil {
			return writeErr
		}
		if written == 0 {
			return io.ErrShortWrite
		}
	}
	return nil
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

func readToken(path string) (string, error) {
	info, err := os.Lstat(path)
	if err != nil {
		return "", err
	}
	if !info.Mode().IsRegular() || info.Mode().Perm() != 0o600 || info.Size() != sha256.Size*2 {
		return "", errors.New("endpoint token must be a regular 0600 256-bit hexadecimal credential")
	}
	payload, err := os.ReadFile(path)
	if err != nil {
		return "", err
	}
	decoded := make([]byte, sha256.Size)
	if _, err := hex.Decode(decoded, payload); err != nil {
		return "", errors.New("endpoint token is not a 256-bit hexadecimal credential")
	}
	return string(payload), nil
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
	file, err := os.CreateTemp(filepath.Dir(path), "."+filepath.Base(path)+"-*.new")
	if err != nil {
		return err
	}
	temporary := file.Name()
	defer os.Remove(temporary)
	if err := file.Chmod(0o600); err != nil {
		_ = file.Close()
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
		return err
	}
	return os.Rename(temporary, path)
}

func cleanupRuntime(files EndpointFiles) {
	for _, path := range []string{files.QuerySock, files.AdminSock, files.QueryToken, files.AdminToken, files.Discovery} {
		_ = os.Remove(path)
	}
}
