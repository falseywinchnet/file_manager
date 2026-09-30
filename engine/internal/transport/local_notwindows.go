//go:build !windows

package transport

import (
	"context"
	"errors"
	"filemanager/engine/internal/service"
)

func serveWindows(context.Context, string, *service.Service, LocalOptions) error {
	return errors.New("Windows named pipes unavailable")
}
func callWindows(context.Context, string, Authority, Request) (Response, error) {
	return Response{}, errors.New("Windows named pipes unavailable")
}
