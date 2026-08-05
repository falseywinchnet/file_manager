# Rejected Windows directory flush primitive 001

Status: **REJECTED technique; Wine observation, not NTFS proof**.

The first Windows publication adapter opened the store directory with
`FILE_FLAG_BACKUP_SEMANTICS` and attempted `FlushFileBuffers` after `os.Rename`.
The Windows/amd64 crash suite under Wine 11.10 returned `Access denied` at every
segment-directory durability boundary.

Microsoft documents that `FlushFileBuffers` requires a handle with
`GENERIC_WRITE`; directory-handle documentation does not establish this as the
portable metadata-commit primitive. Microsoft separately documents
`MOVEFILE_WRITE_THROUGH` as not returning until the move is on disk.

The candidate now flushes the temporary file and publishes it with
`MoveFileExW(MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING)`. The same crash,
corruption, fallback, and pinned-reader suite passes under Wine.

Reversal requires a native-NTFS test showing a stronger documented primitive or
showing that write-through rename fails the declared power-loss campaign. Wine
execution cannot close that gate.

- [FlushFileBuffers](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers)
- [MoveFileEx](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa)
