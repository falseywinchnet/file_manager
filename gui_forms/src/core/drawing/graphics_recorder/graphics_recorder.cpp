#include "../support/drawing_support.hpp"

GraphicsStateToken GraphicsRecorder::save() {
    require_recordable();
    if (saved_.size() >= maximum_state_depth) {
        throw std::length_error("GUI.Drawing state stack limit exceeded");
    }
    const GraphicsStateToken token{next_token_++};
    saved_.push_back({token, state_});
    DrawingCommand command;
    command.kind = CommandKind::save;
    command.state = state_;
    command.token = token;
    append(std::move(command));
    return token;
}

void GraphicsRecorder::restore(GraphicsStateToken token) {
    require_recordable();
    const auto found = std::find_if(saved_.rbegin(), saved_.rend(),
                                    [token](const SavedState& value) {
                                        return value.token == token;
                                    });
    if (token.value == 0 || found == saved_.rend()) {
        throw std::invalid_argument("graphics state token is invalid or already restored");
    }
    state_ = found->state;
    saved_.erase(found.base() - 1, saved_.end());
    DrawingCommand command;
    command.kind = CommandKind::restore;
    command.state = state_;
    command.token = token;
    append(std::move(command));
}

void GraphicsRecorder::translate(double x, double y) {
    require_recordable();
    state_.transform = state_.transform.followed_by(Matrix::translation(x, y));
    DrawingCommand command;
    command.kind = CommandKind::translate;
    command.state = state_;
    command.first = {x, y};
    append(std::move(command));
}

void GraphicsRecorder::set_transform(Matrix transform) {
    require_recordable();
    if (!transform.finite()) throw std::invalid_argument("graphics transform must be finite");
    state_.transform = transform;
    DrawingCommand command;
    command.kind = CommandKind::set_transform;
    command.state = state_;
    append(std::move(command));
}

void GraphicsRecorder::set_clip(RectF clip) {
    require_recordable();
    require_finite(clip, "graphics clip");
    state_.clip = state_.transform.transform_bounds(clip);
    DrawingCommand command;
    command.kind = CommandKind::set_clip;
    command.state = state_;
    command.rect = clip;
    append(std::move(command));
}

void GraphicsRecorder::reset_clip() {
    require_recordable();
    state_.clip.reset();
    DrawingCommand command;
    command.kind = CommandKind::reset_clip;
    command.state = state_;
    append(std::move(command));
}

void GraphicsRecorder::set_quality(SmoothingMode smoothing,
                                   InterpolationMode interpolation,
                                   PixelOffsetMode pixel_offset,
                                   CompositingMode compositing,
                                   CompositingQuality compositing_quality) {
    require_recordable();
    require_enum(smoothing, 4U, "smoothing mode");
    require_enum(interpolation, 7U, "interpolation mode");
    require_enum(pixel_offset, 4U, "pixel offset mode");
    require_enum(compositing, 1U, "compositing mode");
    require_enum(compositing_quality, 4U, "compositing quality");
    state_.smoothing = smoothing;
    state_.interpolation = interpolation;
    state_.pixel_offset = pixel_offset;
    state_.compositing = compositing;
    state_.compositing_quality = compositing_quality;
    DrawingCommand command;
    command.kind = CommandKind::set_quality;
    command.state = state_;
    append(std::move(command));
}

GraphicsState GraphicsRecorder::current_state() const {
    require_alive();
    return state_;
}

bool GraphicsRecorder::is_visible(PointF point) const {
    require_alive();
    require_finite(point, "visibility point");
    return !state_.clip || state_.clip->contains(state_.transform.transform(point));
}

SizeF GraphicsRecorder::measure_string(std::string_view utf8, const Font& font,
                                       const StringFormat& format,
                                       const TextMetricsProvider& provider) const {
    require_alive();
    if (utf8.size() > maximum_text_bytes || !valid_utf8(utf8)) {
        throw std::invalid_argument("measured text must be bounded valid UTF-8");
    }
    const SizeF result = provider.measure(utf8, font.snapshot(), format.snapshot());
    if (!std::isfinite(result.width) || !std::isfinite(result.height) ||
        result.width < 0.0 || result.height < 0.0) {
        throw std::logic_error("text metrics provider returned invalid dimensions");
    }
    return result;
}

void GraphicsRecorder::clear(Color color) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::clear;
    command.state = state_;
    command.color = color;
    append(std::move(command));
}

void GraphicsRecorder::fill_rectangle(const Brush& brush, RectF rect) {
    require_recordable();
    require_finite(rect, "fill rectangle");
    DrawingCommand command;
    command.kind = CommandKind::fill_rectangle;
    command.state = state_;
    command.rect = rect;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    append(std::move(command));
}

void GraphicsRecorder::draw_rectangle(const Pen& pen, RectF rect) {
    require_recordable();
    require_finite(rect, "stroke rectangle");
    DrawingCommand command;
    command.kind = CommandKind::draw_rectangle;
    command.state = state_;
    command.rect = rect;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_line(const Pen& pen, PointF from, PointF to) {
    require_recordable();
    require_finite(from, "line start");
    require_finite(to, "line end");
    DrawingCommand command;
    command.kind = CommandKind::draw_line;
    command.state = state_;
    command.first = from;
    command.second = to;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_string(std::string_view utf8, const Font& font,
                                   const SolidBrush& brush, PointF origin,
                                   const StringFormat& format) {
    require_recordable();
    require_finite(origin, "text origin");
    if (utf8.size() > maximum_text_bytes || !valid_utf8(utf8)) {
        throw std::invalid_argument("drawn text must be bounded valid UTF-8");
    }
    DrawingCommand command;
    command.kind = CommandKind::draw_string;
    command.state = state_;
    command.first = origin;
    command.color = brush.color();
    command.font = font.snapshot();
    command.format = format.snapshot();
    command.text.assign(utf8);
    append(std::move(command));
}

void GraphicsRecorder::draw_ellipse(const Pen& pen, RectF bounds) {
    require_recordable();
    require_finite(bounds, "stroke ellipse");
    DrawingCommand command;
    command.kind = CommandKind::draw_ellipse;
    command.state = state_;
    command.rect = bounds;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::fill_ellipse(const Brush& brush, RectF bounds) {
    require_recordable();
    require_finite(bounds, "fill ellipse");
    DrawingCommand command;
    command.kind = CommandKind::fill_ellipse;
    command.state = state_;
    command.rect = bounds;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    append(std::move(command));
}

void GraphicsRecorder::fill_polygon(const Brush& brush,
                                    std::span<const PointF> points,
                                    FillMode fill_mode) {
    require_recordable();
    require_enum(fill_mode, 1U, "polygon fill mode");
    if (points.size() < 3U || points.size() > GraphicsPath::maximum_elements) {
        throw std::invalid_argument("polygon must contain 3 through 1000000 points");
    }
    for (const PointF point : points) require_finite(point, "polygon point");
    DrawingCommand command;
    command.kind = CommandKind::fill_polygon;
    command.state = state_;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    command.path.fill_mode = fill_mode;
    command.points.assign(points.begin(), points.end());
    append(std::move(command));
}

void GraphicsRecorder::draw_path(const Pen& pen, const GraphicsPath& path) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::draw_path;
    command.state = state_;
    command.pen = pen.snapshot();
    command.path = path.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::fill_path(const Brush& brush,
                                 const GraphicsPath& path) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::fill_path;
    command.state = state_;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    command.path = path.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_image(const ImageReference& image,
                                  RectF destination, RectF source,
                                  const ImageAttributes& attributes) {
    require_recordable();
    require_finite(destination, "image destination");
    require_finite(source, "image source");
    DrawingCommand command;
    command.kind = CommandKind::draw_image;
    command.state = state_;
    command.rect = destination;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.image = image.snapshot();
    command.image_attributes = attributes.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_image(const Bitmap& image, RectF destination,
                                  RectF source,
                                  const ImageAttributes& attributes) {
    require_recordable();
    require_finite(destination, "bitmap destination");
    require_finite(source, "bitmap source");
    DrawingCommand command;
    command.kind = CommandKind::draw_image;
    command.state = state_;
    command.rect = destination;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.image = image.snapshot();
    command.image_attributes = attributes.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::close() {
    require_alive();
    closed_ = true;
    saved_.clear();
}

bool GraphicsRecorder::closed() const {
    require_alive();
    return closed_;
}

std::span<const DrawingCommand> GraphicsRecorder::commands() const {
    require_alive();
    return commands_;
}

std::string GraphicsRecorder::deterministic_trace() const {
    require_alive();
    std::string trace = "gui.drawing.trace/v1\n";
    for (std::size_t index = 0; index < commands_.size(); ++index) {
        const DrawingCommand& command = commands_[index];
        trace += std::to_string(index) + "|" + command_name(command.kind);
        switch (command.kind) {
        case CommandKind::save:
        case CommandKind::restore:
            trace += "|token=" + std::to_string(command.token.value);
            break;
        case CommandKind::translate:
            trace += "|offset=" + point_text(command.first);
            break;
        case CommandKind::set_clip:
            trace += "|rect=" + rect_text(command.rect);
            break;
        case CommandKind::clear:
            trace += "|color=" + color_text(command.color);
            break;
        case CommandKind::fill_rectangle:
            trace += "|rect=" + rect_text(command.rect) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_rectangle:
            trace += "|rect=" + rect_text(command.rect) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::draw_line:
            trace += "|from=" + point_text(command.first) +
                     "|to=" + point_text(command.second) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::draw_string:
            trace += "|at=" + point_text(command.first) +
                     "|font=" + escaped(command.font.family) + "," +
                     number(command.font.size) + "," +
                     std::to_string(command.font.style) +
                     "|brush=" + color_text(command.color) +
                     "|format=" + std::to_string(static_cast<unsigned>(command.format.alignment)) +
                     "," + std::to_string(static_cast<unsigned>(command.format.line_alignment)) +
                     "," + std::to_string(static_cast<unsigned>(command.format.trimming)) +
                     "," + std::to_string(command.format.flags) +
                     "|text=" + escaped(command.text);
            break;
        case CommandKind::draw_ellipse:
            trace += "|bounds=" + rect_text(command.rect) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::fill_ellipse:
            trace += "|bounds=" + rect_text(command.rect) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::fill_polygon:
            trace += "|fill=" +
                     std::to_string(static_cast<unsigned>(command.path.fill_mode)) +
                     "|points=" + points_text(command.points) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_path:
            trace += "|path=" + path_text(command.path) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::fill_path:
            trace += "|path=" + path_text(command.path) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_image:
            trace += "|image=" + std::to_string(command.image.stable_id) + "," +
                     std::to_string(command.image.width) + "," +
                     std::to_string(command.image.height) + "," +
                     std::to_string(static_cast<unsigned>(command.image.pixel_format)) +
                     "," + std::to_string(command.image.generation) +
                     "|destination=" + rect_text(command.rect) +
                     "|source=" + point_text(command.first) + "," +
                     point_text(command.second) +
                     "|attributes=" + image_attributes_text(command.image_attributes);
            break;
        case CommandKind::set_transform:
        case CommandKind::reset_clip:
        case CommandKind::set_quality:
            break;
        }
        trace += "|transform=" + matrix_text(command.state.transform);
        trace += command.state.clip ? "|clip=" + rect_text(*command.state.clip) : "|clip=none";
        trace += "|quality=" +
                 std::to_string(static_cast<unsigned>(command.state.smoothing)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.interpolation)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.pixel_offset)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.compositing)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.compositing_quality));
        trace += "\n";
    }
    trace += "end|commands=" + std::to_string(commands_.size()) +
             "|closed=" + std::string(closed_ ? "1" : "0") + "\n";
    return trace;
}

void GraphicsRecorder::on_dispose() noexcept {
    closed_ = true;
    saved_.clear();
    commands_.clear();
}

void GraphicsRecorder::require_recordable() const {
    require_alive();
    if (closed_) throw std::logic_error("GUI.Drawing recorder is closed");
    if (commands_.size() >= maximum_commands) {
        throw std::length_error("GUI.Drawing command limit exceeded");
    }
}

void GraphicsRecorder::append(DrawingCommand command) {
    if (commands_.size() >= maximum_commands) {
        throw std::length_error("GUI.Drawing command limit exceeded");
    }
    commands_.push_back(std::move(command));
}

} // namespace gui_drawing

