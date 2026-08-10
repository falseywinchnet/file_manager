#pragma once

#include "../chunk/display_chunk.hpp"

namespace gui_forms::detail {

[[nodiscard]] std::uint64_t replay_display_chunk(const DisplayChunk& chunk,
                                                 Painter& painter);

} // namespace gui_forms::detail
