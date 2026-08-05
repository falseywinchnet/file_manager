// Package identity extracts exact platform object keys from filesystem
// observations. Paths never participate in object identity.
package identity

import (
	"fmt"
	"os"
	"strconv"
	"strings"
)

type Platform uint8

const (
	PlatformUnknown Platform = iota
	PlatformDarwin
	PlatformLinux
	PlatformWindows
	PlatformFixture
)

type Observation struct {
	Platform    Platform
	Volume      uint64
	Object      uint64
	Incarnation Incarnation
}

type Incarnation struct {
	A         uint64
	B         uint32
	Available bool
}

type VolumeKey struct {
	Platform Platform
	Native   uint64
}

type Key struct {
	Platform    Platform
	Volume      uint64
	Object      uint64
	Incarnation Incarnation
}

func (o Observation) Key() Key {
	return Key{Platform: o.Platform, Volume: o.Volume, Object: o.Object, Incarnation: o.Incarnation}
}

func (o Observation) VolumeKey() VolumeKey {
	return VolumeKey{Platform: o.Platform, Native: o.Volume}
}

func (o Observation) ObjectID() string {
	if o.Incarnation.Available {
		return fmt.Sprintf("%s:%016x:%016x:%016x:%08x", o.Namespace(), o.Volume, o.Object, o.Incarnation.A, o.Incarnation.B)
	}
	return fmt.Sprintf("%s:%016x:%016x", o.Namespace(), o.Volume, o.Object)
}

func (o Observation) Fields() map[string]string {
	return map[string]string{
		"namespace": o.Namespace(),
		"volume":    fmt.Sprintf("%016x", o.Volume),
		"object":    fmt.Sprintf("%016x", o.Object),
	}
}

func (o Observation) Namespace() string {
	switch o.Platform {
	case PlatformDarwin:
		return "darwin"
	case PlatformLinux:
		return "linux"
	case PlatformWindows:
		return "windows"
	case PlatformFixture:
		return "fixture"
	default:
		return "unknown"
	}
}

func (o Observation) IncarnationString() string {
	if !o.Incarnation.Available {
		return ""
	}
	return fmt.Sprintf("observed:%016x:%08x", o.Incarnation.A, o.Incarnation.B)
}

func ParseObjectID(encoded string) (Observation, error) {
	parts := strings.Split(encoded, ":")
	if len(parts) != 3 && len(parts) != 5 {
		return Observation{}, fmt.Errorf("object id has %d fields, want 3 or 5", len(parts))
	}
	var platform Platform
	switch parts[0] {
	case "darwin":
		platform = PlatformDarwin
	case "linux":
		platform = PlatformLinux
	case "windows":
		platform = PlatformWindows
	case "fixture":
		platform = PlatformFixture
	default:
		return Observation{}, fmt.Errorf("unknown object identity namespace %q", parts[0])
	}
	volume, err := strconv.ParseUint(parts[1], 16, 64)
	if err != nil {
		return Observation{}, fmt.Errorf("parse object volume: %w", err)
	}
	object, err := strconv.ParseUint(parts[2], 16, 64)
	if err != nil {
		return Observation{}, fmt.Errorf("parse object number: %w", err)
	}
	result := Observation{Platform: platform, Volume: volume, Object: object}
	if len(parts) == 5 {
		incarnationA, err := strconv.ParseUint(parts[3], 16, 64)
		if err != nil {
			return Observation{}, fmt.Errorf("parse object incarnation: %w", err)
		}
		incarnationB, err := strconv.ParseUint(parts[4], 16, 32)
		if err != nil {
			return Observation{}, fmt.Errorf("parse object incarnation detail: %w", err)
		}
		result.Incarnation = Incarnation{A: incarnationA, B: uint32(incarnationB), Available: true}
	}
	return result, nil
}

func Compare(left, right Observation) int {
	if left.Platform != right.Platform {
		if left.Platform < right.Platform {
			return -1
		}
		return 1
	}
	if left.Volume != right.Volume {
		if left.Volume < right.Volume {
			return -1
		}
		return 1
	}
	if left.Object < right.Object {
		return -1
	}
	if left.Object > right.Object {
		return 1
	}
	if left.Incarnation.Available != right.Incarnation.Available {
		if !left.Incarnation.Available {
			return -1
		}
		return 1
	}
	if left.Incarnation.A < right.Incarnation.A {
		return -1
	}
	if left.Incarnation.A > right.Incarnation.A {
		return 1
	}
	if left.Incarnation.B < right.Incarnation.B {
		return -1
	}
	if left.Incarnation.B > right.Incarnation.B {
		return 1
	}
	return 0
}

// Observe is implemented per target because an os.FileInfo alone does not
// expose a portable authoritative object key. root and name keep any handle
// acquisition confined to the approved root.
func Observe(root *os.Root, name string, info os.FileInfo) (Observation, error) {
	return observe(root, name, info)
}
