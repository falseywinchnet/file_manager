package transport

import (
	"bytes"
	"encoding/binary"
	"io"
	"testing"
)

func TestFrameReaderReusesStorageAndRejectsOversize(t *testing.T) {
	var wire bytes.Buffer = bytes.Buffer{}
	var err error = writeFrame(&wire, "longer first payload")
	if err != nil {
		t.Fatal(err)
	}
	err = writeFrame(&wire, "x")
	if err != nil {
		t.Fatal(err)
	}
	var frames frameReader = frameReader{}
	var value string = ""
	err = frames.Read(&wire, &value)
	if err != nil || value != "longer first payload" {
		t.Fatalf("first value=%q err=%v", value, err)
	}
	var first *byte = &frames.payload[0]
	var capacity int = cap(frames.payload)
	err = frames.Read(&wire, &value)
	if err != nil || value != "x" {
		t.Fatalf("second value=%q err=%v", value, err)
	}
	if &frames.payload[0] != first || cap(frames.payload) != capacity || len(frames.payload) != 3 {
		t.Fatal("smaller frame did not reuse the original storage at its active length")
	}
	var header [frameHeader]byte = [frameHeader]byte{}
	copy(header[:], frameMagic)
	binary.BigEndian.PutUint32(header[4:], MaxJSONLFrameBytes+1)
	var input *bytes.Reader = bytes.NewReader(header[:])
	err = frames.Read(input, &value)
	if err == nil || cap(frames.payload) != capacity {
		t.Fatal("oversized frame allocated storage or was accepted")
	}
}

type shortFrameWriter struct {
	bytes   bytes.Buffer
	maximum int
}

func (writer *shortFrameWriter) Write(payload []byte) (int, error) {
	var count int = min(len(payload), writer.maximum)
	var written int = 0
	var err error = nil
	written, err = writer.bytes.Write(payload[:count])
	return written, err
}

func TestFrameWriterPreservesWireWithShortWrites(t *testing.T) {
	var writer shortFrameWriter = shortFrameWriter{bytes: bytes.Buffer{}, maximum: 2}
	var err error = writeFrame(&writer, "hello")
	if err != nil {
		t.Fatal(err)
	}
	var expected []byte = []byte{'E', 'N', 'G', '1', 0, 0, 0, 7, '"', 'h', 'e', 'l', 'l', 'o', '"'}
	if !bytes.Equal(writer.bytes.Bytes(), expected) {
		t.Fatalf("wire=%x", writer.bytes.Bytes())
	}
	writer.maximum = 0
	err = writeFrame(&writer, "hello")
	if err != io.ErrShortWrite {
		t.Fatalf("zero progress=%v", err)
	}
}

func BenchmarkFrameStorageReusable(b *testing.B) { benchmarkFrameStorage(b, true) }
func BenchmarkFrameStorageOneShot(b *testing.B)  { benchmarkFrameStorage(b, false) }
func benchmarkFrameStorage(b *testing.B, reuse bool) {
	var wire bytes.Buffer = bytes.Buffer{}
	var err error = writeFrame(&wire, "fixture payload")
	if err != nil {
		b.Fatal(err)
	}
	var payload []byte = wire.Bytes()
	var input bytes.Reader = bytes.Reader{}
	var frames frameReader = frameReader{}
	var value string = ""
	var index int = 0
	b.ReportAllocs()
	b.ResetTimer()
	if reuse {
		for index = 0; index < b.N; index++ {
			input.Reset(payload)
			err = frames.Read(&input, &value)
			if err != nil {
				b.Fatal(err)
			}
		}
	} else {
		for index = 0; index < b.N; index++ {
			input.Reset(payload)
			err = readFrame(&input, &value)
			if err != nil {
				b.Fatal(err)
			}
		}
	}
}
