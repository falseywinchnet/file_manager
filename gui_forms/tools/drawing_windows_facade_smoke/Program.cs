using System.Drawing;
using System.Runtime.InteropServices;

internal static class Program
{
    [DllImport("gdi32.dll", EntryPoint = "DeleteObject")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DeleteObject(nint handle);
    [DllImport("gdi32.dll")]
    private static extern nint CreateCompatibleDC(nint device);
    [DllImport("gdi32.dll")]
    private static extern nint SelectObject(nint device, nint drawingObject);
    [DllImport("gdi32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DeleteDC(nint device);
    [DllImport("gdi32.dll")]
    private static extern uint SetPixel(nint device, int x, int y, uint color);

    private static int Main()
    {
        using var source = new Bitmap(3, 2);
        var hbitmap = source.GetHbitmap(Color.White);
        if (hbitmap == 0) throw new InvalidOperationException("GetHbitmap returned null.");
        nint memory = 0;
        nint previous = 0;
        try
        {
            using var imported = Image.FromHbitmap(hbitmap);
            if (imported.Width != 3 || imported.Height != 2)
                throw new InvalidOperationException("Imported HBITMAP dimensions changed.");
            var pixel = imported.GetPixel(0, 0);
            if (pixel.ToArgb() != Color.White.ToArgb())
                throw new InvalidOperationException($"Expected opaque white, got {pixel.ToArgb():x8}.");

            memory = CreateCompatibleDC(0);
            if (memory == 0) throw new InvalidOperationException("CreateCompatibleDC failed.");
            previous = SelectObject(memory, hbitmap);
            if (previous == 0 || previous == -1)
                throw new InvalidOperationException("SelectObject failed.");
            using var graphics = Graphics.FromHdc(memory);
            if (graphics.ClipBounds.Width != 3 || graphics.ClipBounds.Height != 2)
                throw new InvalidOperationException("FromHdc did not capture the selected DIB bounds.");

            using var bitmapGraphics = Graphics.FromImage(source);
            var leased = bitmapGraphics.GetHdc();
            if (leased == 0) throw new InvalidOperationException("GetHdc returned null.");
            if (SetPixel(leased, 2, 1, 0x00ff0000) == 0xffffffff)
                throw new InvalidOperationException("SetPixel rejected bitmap-backed HDC.");
            bitmapGraphics.ReleaseHdc(leased);
            if (source.GetPixel(2, 1).B != 255)
                throw new InvalidOperationException("ReleaseHdc did not import GDI mutation.");
        }
        finally
        {
            if (memory != 0)
            {
                if (previous != 0 && previous != -1) SelectObject(memory, previous);
                DeleteDC(memory);
            }
            if (!DeleteObject(hbitmap))
                throw new InvalidOperationException("DeleteObject rejected caller-owned HBITMAP.");
        }
        Console.WriteLine("drawing-windows-facade-hbitmap=pass");
        return 0;
    }
}
