//go:build !windows

package transport

import (
	"context"
	"errors"
	"filemanager/engine/internal/service"
)

func serveWindows(context.Context, string, *service.Service, LocalOptions) error {
	var failure error = errors.New("Windows named pipes unavailable")
	return failure
}
func callWindows(context.Context, string, Authority, Request) (Response, error) {
	var emptyResponse Response = Response{}
	var failure error = errors.New("Windows named pipes unavailable")
	return emptyResponse, failure
}
