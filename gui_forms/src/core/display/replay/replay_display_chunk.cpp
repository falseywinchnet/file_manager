#include "replay_display_chunk.hpp"
#if defined(GUI_FORMS_PREPARED_TEXT)
#include "../../text/prepared/prepared_storage.hpp"
#endif

namespace gui_forms::detail {

std::uint64_t replay_display_chunk(const DisplayChunk& chunk, Painter& painter) {
    for (const DisplayCommand& command : chunk.commands()) {
        switch (command.operation) {
#if defined(GUI_FORMS_PREPARED_TEXT)
        case DisplayOperation::draw_prepared_text: {
            if (!command.prepared_text || !prepared_authority_current(*command.prepared_text, command.prepared_authority)) {
                throw PreparedTextPaintFailure(PreparedTextStatus::stale);
            }
            PreparedTextLayout layout = PreparedTextAccess::retained_layout(command.prepared_text);
            const PreparedTextPaintResult result = painter.draw_prepared_text(layout, command.prepared_authority,
                command.first, command.color);
            if (result.status != PreparedTextStatus::success || result.disposition == PreparedTextPaintDisposition::refused) {
                throw PreparedTextPaintFailure(result.status);
            }
            break;
        }
#endif
        case DisplayOperation::save: painter.save(); break;
        case DisplayOperation::restore: painter.restore(); break;
        case DisplayOperation::translate: painter.translate(command.first); break;
        case DisplayOperation::clip_rect: painter.clip_rect(command.rect); break;
        case DisplayOperation::clip_rounded_rect:
            painter.clip_rounded_rect(command.rect, command.scalar);
            break;
        case DisplayOperation::fill_rect:
            painter.fill_rect(command.rect, command.color);
            break;
        case DisplayOperation::fill_rounded_rect:
            painter.fill_rounded_rect(command.rect, command.scalar,
                                      command.color);
            break;
        case DisplayOperation::stroke_rect:
            painter.stroke_rect(command.rect, command.color, command.scalar);
            break;
        case DisplayOperation::stroke_rounded_rect:
            painter.stroke_rounded_rect(command.rect, command.scalar,
                                        command.color,
                                        command.secondary_scalar);
            break;
        case DisplayOperation::fill_linear_gradient:
            painter.fill_linear_gradient(command.rect, command.first,
                                         command.second,
                                         command.gradient_stops);
            break;
        case DisplayOperation::fill_linear_gradient_spread:
            painter.fill_linear_gradient_spread(
                command.rect, command.first, command.second,
                command.gradient_stops, command.gradient_spread);
            break;
        case DisplayOperation::fill_radial_gradient:
            painter.fill_radial_gradient(
                command.rect, command.first,
                {command.second.x, command.second.y}, command.gradient_stops);
            break;
        case DisplayOperation::draw_box_shadow:
            painter.draw_box_shadow(
                command.rect, command.scalar, command.first,
                command.secondary_scalar, command.tertiary_scalar,
                command.color);
            break;
        case DisplayOperation::draw_inset_box_shadow:
            painter.draw_inset_box_shadow(
                command.rect, command.scalar, command.first,
                command.secondary_scalar, command.tertiary_scalar,
                command.color);
            break;
        case DisplayOperation::draw_line:
            painter.draw_line(command.first, command.second, command.color,
                              command.scalar);
            break;
        case DisplayOperation::draw_text_utf8:
            painter.draw_text_utf8(command.first, command.text, command.font,
                                   command.color);
            break;
        case DisplayOperation::draw_image:
            painter.draw_image(command.image, command.rect, command.scalar);
            break;
        case DisplayOperation::draw_live_surface:
            painter.draw_live_surface(
                command.live_surface, command.rect, command.scalar);
            break;
        case DisplayOperation::draw_image_region:
            painter.draw_image_region(
                command.image,
                {command.first.x, command.first.y,
                 command.second.x, command.second.y},
                command.rect, command.scalar);
            break;
        case DisplayOperation::draw_image_region_sampled:
            painter.draw_image_region_sampled(
                command.image,
                {command.first.x, command.first.y,
                 command.second.x, command.second.y},
                command.rect, command.image_sampling, command.scalar);
            break;
        case DisplayOperation::fill_image_pattern:
            painter.fill_image_pattern(
                command.image, {command.first.x, command.first.y},
                command.rect, {command.second.x, command.second.y},
                command.image_pattern_wrap, command.scalar);
            break;
        }
    }
    return chunk.commands().size();
}

} // namespace gui_forms::detail
