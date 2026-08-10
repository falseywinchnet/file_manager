//go:build !darwin

package deployment

import "errors"

func hostUUID() (string, error) {
	return "", errors.New("installed Engine manifest is currently supported only on macOS")
}
