using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Text.Json;

static object Size(SizeF value) => new
{
    width = Math.Round(value.Width, 3),
    height = Math.Round(value.Height, 3),
};

static object Rect(RectangleF value) => new
{
    x = Math.Round(value.X, 3),
    y = Math.Round(value.Y, 3),
    width = Math.Round(value.Width, 3),
    height = Math.Round(value.Height, 3),
};

static string Outcome(Func<object?> operation)
{
    try
    {
        var value = operation();
        return value is null ? "ok" : $"ok:{value}";
    }
    catch (Exception error)
    {
        return error.GetType().FullName ?? error.GetType().Name;
    }
}

using var bitmap = new Bitmap(180, 52, PixelFormat.Format32bppPArgb);
using var graphics = Graphics.FromImage(bitmap);
using var font = new Font("Portsmouth Rapids", 12f, FontStyle.Regular,
    GraphicsUnit.Point, 1);
using var format = new StringFormat();
using var brush = new SolidBrush(Color.Black);

var narrow = graphics.MeasureString("iiii", font);
var wide = graphics.MeasureString("WWWW", font);
var radio = graphics.MeasureString("Radio", font);
var unbounded = graphics.MeasureString("alpha beta gamma", font);
var wrapped = graphics.MeasureString("alpha beta gamma", font,
    Math.Max(1, (int)(unbounded.Width / 2f)), format);
var familyMeasures = new Dictionary<string, object>(StringComparer.Ordinal);
foreach (var family in new[]
         {
             "Lucida Grande", "Segoe UI", "Tahoma", "Helvetica",
             "Lucida Console", "Segoe UI Symbol", "Microsoft Sans Serif",
             "Portsmouth Rapids",
         })
{
    using var requested = new Font(family, 12f, FontStyle.Regular,
        GraphicsUnit.Point, 1);
    familyMeasures[family] = new
    {
        requested.Height,
        measured = Size(graphics.MeasureString("Radio 100.5 MHz", requested)),
    };
}

using var path = new GraphicsPath();
path.AddRectangle(new RectangleF(3.5f, 5.25f, 40f, 20f));
path.AddEllipse(new RectangleF(31f, 9f, 28f, 28f));
path.AddLine(new PointF(-4f, 12f), new PointF(8f, 41f));
var pathBounds = path.GetBounds();
var pathPoints = path.PathPoints;
var visibleInside = path.IsVisible(new Point(10, 10));
var visibleOutside = path.IsVisible(new Point(80, 80));
using var transformedPath = (GraphicsPath)path.Clone();
using var transform = new Matrix();
transform.Translate(7.25f, -3.5f);
transformedPath.Transform(transform);
var transformedBounds = transformedPath.GetBounds();
using var arcPath = new GraphicsPath();
arcPath.AddArc(new RectangleF(20f, 10f, 8f, 6f), -35f, 230f);
var arcPoints = arcPath.PathPoints;

graphics.Clear(Color.White);
graphics.DrawString("Radio 100.5 MHz", font, brush, PointF.Empty);
var inkCount = 0;
var inkLeft = bitmap.Width;
var inkTop = bitmap.Height;
var inkRight = -1;
var inkBottom = -1;
for (var y = 0; y < bitmap.Height; ++y)
{
    for (var x = 0; x < bitmap.Width; ++x)
    {
        if (bitmap.GetPixel(x, y).ToArgb() == Color.White.ToArgb()) continue;
        ++inkCount;
        inkLeft = Math.Min(inkLeft, x);
        inkTop = Math.Min(inkTop, y);
        inkRight = Math.Max(inkRight, x);
        inkBottom = Math.Max(inkBottom, y);
    }
}

var report = new
{
    schema = "gui.drawing.metrics-differential/v2",
    runtime = Environment.Version.ToString(),
    font = new { requestedFamily = "Portsmouth Rapids", size = 12, unit = "Point", font.Height },
    measure = new
    {
        narrow = Size(narrow),
        wide = Size(wide),
        radio = Size(radio),
        unbounded = Size(unbounded),
        wrapped = Size(wrapped),
        families = familyMeasures,
    },
    geometry = new
    {
        pathBounds = Rect(pathBounds),
        pointCount = pathPoints.Length,
        points = pathPoints.Select(point => new
        {
            x = Math.Round(point.X, 3),
            y = Math.Round(point.Y, 3),
        }).ToArray(),
        visibleInside,
        visibleOutside,
        translatedBounds = Rect(transformedBounds),
        arc = new
        {
            pointCount = arcPoints.Length,
            points = arcPoints.Select(point => new
            {
                x = Math.Round(point.X, 4),
                y = Math.Round(point.Y, 4),
            }).ToArray(),
        },
    },
    raster = new
    {
        inkCount,
        bounds = inkRight < inkLeft ? null : new { left = inkLeft, top = inkTop, right = inkRight, bottom = inkBottom },
    },
    exceptions = new
    {
        nullText = Outcome(() => graphics.MeasureString(null!, font)),
        nullFont = Outcome(() => graphics.MeasureString("x", null!)),
        nullFormat = Outcome(() => graphics.MeasureString("x", font, 20, null!)),
        negativeWidth = Outcome(() => graphics.MeasureString("x", font, -1, format)),
    },
};

Console.WriteLine(JsonSerializer.Serialize(report, new JsonSerializerOptions { WriteIndented = true }));
