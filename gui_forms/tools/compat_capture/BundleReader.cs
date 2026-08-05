using System.Buffers.Binary;
using System.IO.Compression;
using System.Security.Cryptography;
using System.Text;

namespace GuiForms.CompatCapture;

internal sealed class BundleImage
{
    public required int MajorVersion { get; init; }
    public required int MinorVersion { get; init; }
    public required string BundleId { get; init; }
    public required IReadOnlyList<BundleEntry> Entries { get; init; }
}

internal sealed class BundleEntry
{
    public required string RelativePath { get; init; }
    public required BundleFileType FileType { get; init; }
    public required long Offset { get; init; }
    public required long Size { get; init; }
    public required long CompressedSize { get; init; }
    public long StoredSize => CompressedSize == 0 ? Size : CompressedSize;
}

internal enum BundleFileType : byte
{
    Unknown = 0,
    Assembly = 1,
    NativeBinary = 2,
    DepsJson = 3,
    RuntimeConfigJson = 4,
    Symbols = 5,
}

internal static class BundleReader
{
    private static ReadOnlySpan<byte> BundleSignature =>
    [
        0x8b, 0x12, 0x02, 0xb9, 0x6a, 0x61, 0x20, 0x38,
        0x72, 0x7b, 0x93, 0x02, 0x14, 0xd7, 0xa0, 0x32,
        0x13, 0xf5, 0xb9, 0xe6, 0xef, 0xae, 0x33, 0x18,
        0xee, 0x3b, 0x2d, 0xce, 0x24, 0xb3, 0x6a, 0xae,
    ];

    public static BundleImage? TryRead(byte[] image)
    {
        var marker = image.AsSpan().IndexOf(BundleSignature);
        if (marker < sizeof(long))
        {
            return null;
        }

        var headerOffset = BinaryPrimitives.ReadInt64LittleEndian(
            image.AsSpan(marker - sizeof(long), sizeof(long)));
        if (headerOffset < 0 || headerOffset >= image.LongLength)
        {
            throw new InvalidDataException("Bundle header offset is outside the input image.");
        }

        using var stream = new MemoryStream(image, writable: false);
        using var reader = new BinaryReader(stream, Encoding.UTF8, leaveOpen: true);
        stream.Position = headerOffset;

        var major = reader.ReadInt32();
        var minor = reader.ReadInt32();
        var fileCount = reader.ReadInt32();
        if (major is < 1 or > 6 || minor < 0 || fileCount is < 0 or > 100_000)
        {
            throw new InvalidDataException(
                $"Unsupported or invalid bundle header {major}.{minor} with {fileCount} files.");
        }

        var bundleId = ReadBundleString(reader);
        if (major >= 2)
        {
            _ = reader.ReadInt64(); // deps.json offset
            _ = reader.ReadInt64(); // deps.json size
            _ = reader.ReadInt64(); // runtimeconfig.json offset
            _ = reader.ReadInt64(); // runtimeconfig.json size
            _ = reader.ReadUInt64(); // bundle flags
        }

        var entries = new List<BundleEntry>(fileCount);
        for (var index = 0; index < fileCount; ++index)
        {
            var offset = reader.ReadInt64();
            var size = reader.ReadInt64();
            var compressedSize = major >= 6 ? reader.ReadInt64() : 0;
            var type = (BundleFileType)reader.ReadByte();
            var path = ReadBundleString(reader);
            var storedSize = compressedSize == 0 ? size : compressedSize;
            if (offset < 0 || size < 0 || compressedSize < 0 ||
                storedSize > image.LongLength || offset > image.LongLength - storedSize)
            {
                throw new InvalidDataException($"Bundle entry {path} is outside the input image.");
            }

            entries.Add(new BundleEntry
            {
                RelativePath = NormalizeRelativePath(path),
                FileType = type,
                Offset = offset,
                Size = size,
                CompressedSize = compressedSize,
            });
        }

        return new BundleImage
        {
            MajorVersion = major,
            MinorVersion = minor,
            BundleId = bundleId,
            Entries = entries.OrderBy(entry => entry.RelativePath, StringComparer.Ordinal).ToArray(),
        };
    }

    public static byte[] ReadEntry(byte[] image, BundleEntry entry)
    {
        var stored = image.AsSpan(checked((int)entry.Offset), checked((int)entry.StoredSize));
        if (entry.CompressedSize == 0)
        {
            return stored.ToArray();
        }

        using var input = new MemoryStream(stored.ToArray(), writable: false);
        using var inflater = new DeflateStream(input, CompressionMode.Decompress);
        using var output = new MemoryStream(checked((int)entry.Size));
        inflater.CopyTo(output);
        if (output.Length != entry.Size)
        {
            throw new InvalidDataException(
                $"Bundle entry {entry.RelativePath} expanded to {output.Length}, expected {entry.Size}.");
        }
        return output.ToArray();
    }

    public static string Sha256(byte[] bytes) =>
        Convert.ToHexStringLower(SHA256.HashData(bytes));

    private static string ReadBundleString(BinaryReader reader)
    {
        var length = Read7BitEncodedInt(reader);
        if (length is < 0 or > 1_048_576)
        {
            throw new InvalidDataException($"Invalid bundle string length {length}.");
        }
        var bytes = reader.ReadBytes(length);
        if (bytes.Length != length)
        {
            throw new EndOfStreamException("Truncated bundle string.");
        }
        return new UTF8Encoding(false, true).GetString(bytes);
    }

    private static int Read7BitEncodedInt(BinaryReader reader)
    {
        uint value = 0;
        for (var shift = 0; shift < 35; shift += 7)
        {
            var current = reader.ReadByte();
            value |= (uint)(current & 0x7f) << shift;
            if ((current & 0x80) == 0)
            {
                return checked((int)value);
            }
        }
        throw new InvalidDataException("Invalid 7-bit encoded integer.");
    }

    private static string NormalizeRelativePath(string path)
    {
        var normalized = path.Replace('\\', '/');
        if (normalized.StartsWith('/') || normalized.Split('/').Any(part => part == ".."))
        {
            throw new InvalidDataException($"Unsafe bundle-relative path {path}.");
        }
        return normalized;
    }
}
