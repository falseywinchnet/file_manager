package generation

import (
	"errors"
	"io"
	"os"
	"sync"
)

const (
	readBlockSize  = 32 << 10
	readCacheSlots = 256
	ReadCacheLimit = readBlockSize * readCacheSlots
)

type readBlock struct {
	mu     sync.RWMutex
	valid  bool
	number uint64
	length int
	data   []byte
}

// readCache is direct-mapped and lazily allocated. This bounds retained bytes
// and avoids a global LRU lock on concurrent queries. A collision costs one
// ReadAt and cannot change correctness.
type readCache struct {
	file     *os.File
	fileSize uint64
	slots    [readCacheSlots]readBlock
}

func (c *readCache) bytes() uint64 {
	var result uint64
	for index := range c.slots {
		block := &c.slots[index]
		block.mu.RLock()
		result += uint64(cap(block.data))
		block.mu.RUnlock()
	}
	return result
}

func (c *readCache) readAt(output []byte, offset uint64) error {
	if offset > c.fileSize || uint64(len(output)) > c.fileSize-offset {
		return io.ErrUnexpectedEOF
	}
	for len(output) != 0 {
		blockNumber := offset / readBlockSize
		within := int(offset % readBlockSize)
		count := readBlockSize - within
		if count > len(output) {
			count = len(output)
		}
		block := &c.slots[blockNumber%readCacheSlots]
		block.mu.RLock()
		if block.valid && block.number == blockNumber && within+count <= block.length {
			copy(output[:count], block.data[within:within+count])
			block.mu.RUnlock()
		} else {
			block.mu.RUnlock()
			block.mu.Lock()
			if !block.valid || block.number != blockNumber {
				if block.data == nil {
					block.data = make([]byte, readBlockSize)
				}
				n, err := c.file.ReadAt(block.data, int64(blockNumber*readBlockSize))
				if err != nil && !errors.Is(err, io.EOF) {
					block.mu.Unlock()
					return err
				}
				block.number = blockNumber
				block.length = n
				block.valid = true
			}
			if within+count > block.length {
				block.mu.Unlock()
				return io.ErrUnexpectedEOF
			}
			copy(output[:count], block.data[within:within+count])
			block.mu.Unlock()
		}
		offset += uint64(count)
		output = output[count:]
	}
	return nil
}
