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

	"filemanager/engine/api"
	"filemanager/engine/internal/service"
)

const (
	LocalProtocol string = "engine.local.v1"
	frameMagic    string = "ENG1"
	frameHeader   int    = 8

	maxQueryConnections int           = 32
	maxAdminConnections int           = 4
	handshakeTimeout    time.Duration = 5 * time.Second
	idleFrameTimeout    time.Duration = 30 * time.Second
	requestTimeout      time.Duration = 30 * time.Second
	writeTimeout        time.Duration = 5 * time.Second
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
	var files EndpointFiles = EndpointFiles{
		RuntimeDir: runtimeDir,
		QuerySock:  filepath.Join(runtimeDir, "query.sock"), AdminSock: filepath.Join(runtimeDir, "admin.sock"),
		QueryToken: filepath.Join(runtimeDir, "query.token"), AdminToken: filepath.Join(runtimeDir, "admin.token"),
		Discovery: filepath.Join(runtimeDir, "discovery.json"),
	}
	return files
}

// ServeLocal creates distinct same-user query/admin endpoints. Credentials are
// random per process instance and are never accepted on the other endpoint.
func ServeLocal(ctx context.Context, runtimeDir string, engine *service.Service) error {
	var failure error = ServeLocalWithOptions(ctx, runtimeDir, engine, LocalOptions{})
	return failure
}

func ServeLocalWithOptions(ctx context.Context, runtimeDir string, engine *service.Service, options LocalOptions) error {
	if runtime.GOOS == "windows" {
		var failure error = serveWindows(ctx, runtimeDir, engine, options)
		return failure
	}
	if engine == nil {
		var failure error = errors.New("engine service is required")
		return failure
	}
	var files EndpointFiles = Files(runtimeDir)
	{
		var err error = nil
		err = verifyRuntimeDirectory(runtimeDir)
		if err != nil {
			return err
		}
	}
	var query net.Listener = nil
	var err error = nil
	query, err = listenPrivate(files.QuerySock)
	if err != nil {
		return err
	}
	defer query.Close()
	defer os.Remove(files.QuerySock)
	var admin net.Listener = nil
	admin, err = listenPrivate(files.AdminSock)
	if err != nil {
		return err
	}
	defer admin.Close()
	defer os.Remove(files.AdminSock)
	defer cleanupRuntime(files)
	var queryToken string = ""
	queryToken, err = rotateToken(files.QueryToken)
	if err != nil {
		return err
	}
	var adminToken string = ""
	adminToken, err = rotateToken(files.AdminToken)
	if err != nil {
		return err
	}
	var version api.VersionInfo = engine.Version()
	var started time.Time = time.Now()
	started = started.UTC()
	var discovery Discovery = Discovery{
		Protocol: LocalProtocol, InstanceID: version.InstanceID, UID: os.Getuid(), StartedAt: started,
		QuerySocket: files.QuerySock, QueryTokenFile: files.QueryToken,
		AdminSocket: files.AdminSock, AdminTokenFile: files.AdminToken,
	}
	{
		var err error = nil
		err = writePrivateJSON(files.Discovery, discovery)
		if err != nil {
			return err
		}
	}
	var serverCtx context.Context = nil
	var cancel context.CancelFunc = nil
	serverCtx, cancel = context.WithCancel(ctx)
	defer cancel()
	var shutdown context.CancelFunc = cancel
	var errorsOut chan error = make(chan error, 2)
	var connections *connectionSet = newConnectionSet()
	var acceptors sync.WaitGroup = sync.WaitGroup{}
	var handlers sync.WaitGroup = sync.WaitGroup{}
	acceptors.Add(2)
	var queryTask endpointTask = endpointTask{
		context: serverCtx, listener: query, engine: engine, authority: AuthorityQuery,
		token: queryToken, maximumConnections: maxQueryConnections, connections: connections,
		handlers: &handlers, acceptors: &acceptors, shutdown: shutdown, errorsOut: errorsOut, options: options,
	}
	var adminTask endpointTask = endpointTask{
		context: serverCtx, listener: admin, engine: engine, authority: AuthorityAdmin,
		token: adminToken, maximumConnections: maxAdminConnections, connections: connections,
		handlers: &handlers, acceptors: &acceptors, shutdown: shutdown, errorsOut: errorsOut, options: options,
	}
	go queryTask.Run()
	go adminTask.Run()
	var serveErr error = nil
	select {
	case <-serverCtx.Done():
		if ctx.Err() != nil {
			serveErr = ctx.Err()
		}
	case serveErr = <-errorsOut:
	}
	cancel()
	_ = query.Close()
	_ = admin.Close()
	acceptors.Wait()
	connections.CloseAll()
	handlers.Wait()
	return serveErr
}

// endpointTask borrows the listener/service and shared connection registry.
// The server joins acceptors, closes registered connections, then joins handlers
// before releasing any borrowed state. Each endpoint owns its bounded slot pool.
type endpointTask struct {
	context            context.Context
	listener           net.Listener
	engine             *service.Service
	authority          Authority
	token              string
	maximumConnections int
	connections        *connectionSet
	handlers           *sync.WaitGroup
	acceptors          *sync.WaitGroup
	shutdown           context.CancelFunc
	errorsOut          chan<- error
	options            LocalOptions
}

func (task *endpointTask) Run() {
	defer task.acceptors.Done()
	var slots chan struct{} = make(chan struct{}, task.maximumConnections)
	for {
		var connection net.Conn = nil
		var err error = nil
		connection, err = task.listener.Accept()
		if err != nil {
			if task.context.Err() != nil || errors.Is(err, net.ErrClosed) {
				return
			}
			select {
			case task.errorsOut <- err:
			default:
			}
			return
		}
		select {
		case slots <- struct{}{}:
			task.connections.Add(connection)
			task.handlers.Add(1)
			var session connectionTask = connectionTask{endpoint: task, connection: connection, slots: slots}
			go session.Run()
		default:
			_ = connection.Close()
		}
	}
}

// connectionTask owns one accepted connection and its slot until Run exits.
// endpoint remains alive until the server's handler join completes.
type connectionTask struct {
	endpoint   *endpointTask
	connection net.Conn
	slots      chan struct{}
}

func (task *connectionTask) ReleaseSlot() { <-task.slots }

func (task *connectionTask) Run() {
	defer task.endpoint.handlers.Done()
	defer task.ReleaseSlot()
	defer task.endpoint.connections.Remove(task.connection)
	handleLocal(task.endpoint.context, task.connection, task.endpoint.engine,
		task.endpoint.authority, task.endpoint.token, task.endpoint.shutdown, task.endpoint.options)
}

type connectionSet struct {
	mu          sync.Mutex
	connections map[net.Conn]struct{}
}

func newConnectionSet() *connectionSet {
	var registry *connectionSet = &connectionSet{connections: make(map[net.Conn]struct{})}
	return registry
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
	var connections []net.Conn = make([]net.Conn, 0, len(s.connections))
	{
		var connection net.Conn = nil
		for connection = range s.connections {
			connections = append(connections, connection)
		}
	}
	s.mu.Unlock()
	{
		var connection net.Conn = nil
		for _, connection = range connections {
			_ = connection.Close()
		}
	}
}

func handleLocal(ctx context.Context, connection net.Conn, engine *service.Service, authority Authority, token string, shutdown func(), options LocalOptions) {
	defer connection.Close()
	{
		var err error = nil
		var now time.Time = time.Now()
		var deadline time.Time = now.Add(handshakeTimeout)
		err = connection.SetDeadline(deadline)
		if err != nil {
			return
		}
	}
	var uid int = 0
	var err error = nil
	uid, err = peerUID(connection)
	if err != nil || uid != os.Getuid() {
		return
	}
	var frames frameReader = frameReader{}
	var incoming hello = hello{}
	{
		var err error = nil
		err = frames.Read(connection, &incoming)
		if err != nil {
			return
		}
	}
	var accepted bool = incoming.Protocol == LocalProtocol && incoming.Authority == authority && constantToken(incoming.Token, token)
	var response helloResponse = helloResponse{Accepted: accepted, Protocol: LocalProtocol}
	if !accepted {
		response.Error = "session authentication failed"
	}
	{
		var err error = nil
		err = writeFrame(connection, response)
		if err != nil || !accepted {
			return
		}
	}
	{
		var err error = nil
		err = connection.SetDeadline(time.Time{})
		if err != nil {
			return
		}
	}
	for {
		{
			var err error = nil
			var now time.Time = time.Now()
			var deadline time.Time = now.Add(idleFrameTimeout)
			err = connection.SetReadDeadline(deadline)
			if err != nil {
				return
			}
		}
		var request Request = Request{}
		{
			var err error = nil
			err = frames.Read(connection, &request)
			if err != nil {
				return
			}
		}
		var requestCtx context.Context = nil
		var cancel context.CancelFunc = nil
		requestCtx, cancel = context.WithTimeout(ctx, requestTimeout)
		var outgoing Response = Response{}
		var stop bool = false
		outgoing, stop = dispatchAuthorized(requestCtx, engine, request, authority)
		cancel()
		if outgoing.Error == nil && options.AfterSuccessfulDispatch != nil {
			{
				var err error = nil
				err = options.AfterSuccessfulDispatch(request, outgoing)
				if err != nil {
					outgoing.Result = nil
					outgoing.Error = publicFault(err)
					stop = false
				}
			}
		}
		{
			var err error = nil
			var now time.Time = time.Now()
			var deadline time.Time = now.Add(writeTimeout)
			err = connection.SetWriteDeadline(deadline)
			if err != nil {
				return
			}
		}
		{
			var err error = nil
			err = writeFrame(connection, outgoing)
			if err != nil {
				return
			}
		}
		if stop {
			shutdown()
			return
		}
	}
}

func CallLocal(ctx context.Context, runtimeDir string, authority Authority, request Request) (Response, error) {
	if runtime.GOOS == "windows" {
		var emptyResponse Response = Response{}
		var failure error = nil
		emptyResponse, failure = callWindows(ctx, runtimeDir, authority, request)
		return emptyResponse, failure
	}
	var files EndpointFiles = Files(runtimeDir)
	var socket string = ""
	var tokenPath string = ""
	socket, tokenPath = files.QuerySock, files.QueryToken
	if authority == AuthorityAdmin {
		socket, tokenPath = files.AdminSock, files.AdminToken
	} else if authority != AuthorityQuery {
		var emptyResponse Response = Response{}
		var failure error = errors.New("authority must be query or admin")
		return emptyResponse, failure
	}
	var token string = ""
	var err error = nil
	token, err = readToken(tokenPath)
	if err != nil {
		var emptyResponse Response = Response{}
		var failure error = fmt.Errorf("read endpoint token: %w", err)
		return emptyResponse, failure
	}
	var dialer net.Dialer = net.Dialer{}
	var connection net.Conn = nil
	connection, err = dialer.DialContext(ctx, "unix", socket)
	if err != nil {
		var emptyResponse Response = Response{}
		return emptyResponse, err
	}
	defer connection.Close()
	{
		var deadline time.Time = time.Time{}
		var ok bool = false
		deadline, ok = ctx.Deadline()
		if ok {
			_ = connection.SetDeadline(deadline)
		}
	}
	{
		var err error = nil
		err = writeFrame(connection, hello{Protocol: LocalProtocol, Authority: authority, Token: token})
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
	if !accepted.Accepted {
		var emptyResponse Response = Response{}
		var failure error = errors.New(accepted.Error)
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
	return response, nil
}

// frameReader owns connection-local scratch. payload length is the current
// frame's byte count; capacity is reused up to MaxJSONLFrameBytes. Decode must
// finish before Read is called again. No scratch is shared between connections.
type frameReader struct {
	header  [frameHeader]byte
	payload []byte
}

func readFrame(reader io.Reader, target any) error {
	var frames frameReader = frameReader{}
	var err error = frames.Read(reader, target)
	return err
}

func (frames *frameReader) Read(reader io.Reader, target any) error {
	var err error = nil
	_, err = io.ReadFull(reader, frames.header[:])
	if err != nil {
		return err
	}
	if string(frames.header[:4]) != frameMagic {
		var failure error = errors.New("invalid Engine frame magic")
		return failure
	}
	var size uint32 = binary.BigEndian.Uint32(frames.header[4:])
	if size == 0 || size > MaxJSONLFrameBytes {
		var failure error = errors.New("invalid Engine frame size")
		return failure
	}
	var count int = int(size)
	if cap(frames.payload) < count {
		frames.payload = make([]byte, count)
	} else {
		frames.payload = frames.payload[:count]
	}
	_, err = io.ReadFull(reader, frames.payload)
	if err != nil {
		return err
	}
	var input *bytes.Reader = bytes.NewReader(frames.payload)
	var decoder *json.Decoder = json.NewDecoder(input)
	err = decoder.Decode(target)
	if err != nil {
		return err
	}
	var trailing any = nil
	err = decoder.Decode(&trailing)
	if err != io.EOF {
		var failure error = errors.New("Engine frame contains trailing JSON")
		return failure
	}
	return nil
}

func writeFrame(writer io.Writer, value any) error {
	var payload []byte = nil
	var err error = nil
	payload, err = json.Marshal(value)
	if err != nil {
		return err
	}
	var count int = len(payload)
	if count == 0 || count > MaxJSONLFrameBytes {
		var failure error = errors.New("Engine frame exceeds the bounded payload")
		return failure
	}
	var header [frameHeader]byte = [frameHeader]byte{}
	copy(header[:], frameMagic)
	var wireCount uint32 = uint32(count)
	binary.BigEndian.PutUint32(header[4:], wireCount)
	err = writeFrameBytes(writer, header[:])
	if err != nil {
		return err
	}
	err = writeFrameBytes(writer, payload)
	return err
}

// writeFrameBytes consumes borrowed bytes synchronously and retains no storage.
func writeFrameBytes(writer io.Writer, payload []byte) error {
	for len(payload) != 0 {
		var written int = 0
		var err error = nil
		written, err = writer.Write(payload)
		if err != nil {
			return err
		}
		if written <= 0 || written > len(payload) {
			return io.ErrShortWrite
		}
		payload = payload[written:]
	}
	return nil
}

func rotateToken(path string) (string, error) {
	var raw []byte = make([]byte, 32)
	{
		var err error = nil
		_, err = rand.Read(raw)
		if err != nil {
			return "", err
		}
	}
	var token string = hex.EncodeToString(raw)
	{
		var err error = nil
		err = writePrivate(path, []byte(token))
		if err != nil {
			return "", err
		}
	}
	return token, nil
}

func readToken(path string) (string, error) {
	var info os.FileInfo = nil
	var err error = nil
	info, err = os.Lstat(path)
	if err != nil {
		return "", err
	}
	var mode os.FileMode = info.Mode()
	if !mode.IsRegular() || mode.Perm() != 0o600 || info.Size() != sha256.Size*2 {
		var failure error = errors.New("endpoint token must be a regular 0600 256-bit hexadecimal credential")
		return "", failure
	}
	var payload []byte = nil
	payload, err = os.ReadFile(path)
	if err != nil {
		return "", err
	}
	var decoded []byte = make([]byte, sha256.Size)
	{
		var err error = nil
		_, err = hex.Decode(decoded, payload)
		if err != nil {
			var failure error = errors.New("endpoint token is not a 256-bit hexadecimal credential")
			return "", failure
		}
	}
	var result string = string(payload)
	return result, nil
}

func constantToken(left, right string) bool {
	var matches bool = len(left) == len(right) && subtle.ConstantTimeCompare([]byte(left), []byte(right)) == 1
	return matches
}

func verifyRuntimeDirectory(path string) error {
	var info os.FileInfo = nil
	var err error = nil
	info, err = os.Stat(path)
	if err != nil {
		return err
	}
	var mode os.FileMode = info.Mode()
	if !info.IsDir() || mode.Perm()&0o077 != 0 {
		var failure error = errors.New("runtime directory must be private")
		return failure
	}
	return nil
}

func listenPrivate(path string) (net.Listener, error) {
	{
		var info os.FileInfo = nil
		var err error = nil
		info, err = os.Lstat(path)
		if err == nil {
			if info.Mode()&os.ModeSocket == 0 {
				var failure error = errors.New("refusing to replace a non-socket endpoint")
				return nil, failure
			}
			var probe net.Conn = nil
			var probeErr error = nil
			probe, probeErr = net.DialTimeout("unix", path, 100*time.Millisecond)
			if probeErr == nil {
				probe.Close()
				var failure error = errors.New("Engine endpoint is already accepting connections")
				return nil, failure
			}
			{
				var err error = nil
				err = os.Remove(path)
				if err != nil {
					return nil, err
				}
			}
		} else if !errors.Is(err, os.ErrNotExist) {
			return nil, err
		}
	}
	var listener net.Listener = nil
	var err error = nil
	listener, err = net.Listen("unix", path)
	if err != nil {
		return nil, err
	}
	{
		var err error = nil
		err = os.Chmod(path, 0o600)
		if err != nil {
			listener.Close()
			return nil, err
		}
	}
	return listener, nil
}

func writePrivateJSON(path string, value any) error {
	var payload []byte = nil
	var err error = nil
	payload, err = json.MarshalIndent(value, "", "  ")
	if err != nil {
		return err
	}
	payload = append(payload, '\n')
	var failure error = writePrivate(path, payload)
	return failure
}

func writePrivate(path string, payload []byte) error {
	var file *os.File = nil
	var err error = nil
	file, err = os.CreateTemp(filepath.Dir(path), "."+filepath.Base(path)+"-*.new")
	if err != nil {
		return err
	}
	var temporary string = file.Name()
	defer os.Remove(temporary)
	{
		var err error = nil
		err = file.Chmod(0o600)
		if err != nil {
			_ = file.Close()
			return err
		}
	}
	_, err = file.Write(payload)
	if err == nil {
		err = file.Sync()
	}
	var closeErr error = file.Close()
	if err == nil {
		err = closeErr
	}
	if err != nil {
		return err
	}
	var failure error = os.Rename(temporary, path)
	return failure
}

func cleanupRuntime(files EndpointFiles) {
	{
		var path string = ""
		for _, path = range []string{files.QuerySock, files.AdminSock, files.QueryToken, files.AdminToken, files.Discovery} {
			_ = os.Remove(path)
		}
	}
}
