#pragma once

#include <cstddef>
#include <cstdint>

namespace web_forms {
namespace generated {

enum class SurfaceMode : std::uint8_t {
    reveal_parent = 0U,
    own_surface = 1U,
    baked_into_parent = 2U
};

enum class StyleExposure : std::uint8_t {
    baked = 0U,
    exposed = 1U
};

struct Property final {
    std::uint32_t property_id;
    const char* debug_name;
    enum class ValueKind : std::uint8_t {
        keyword = 0U,
        scalar = 1U,
        color = 2U,
        edge_list = 3U,
        track_list = 4U,
        position_list = 5U,
        font_family = 6U,
        gradient = 7U,
        border = 8U,
        shadow_list = 9U,
        transform = 10U,
        filter = 11U,
        string = 12U
    } value_kind;
    struct ValueToken final {
        enum class Kind : std::uint8_t {
            keyword = 0U,
            number = 1U,
            logical_px = 2U,
            percent = 3U,
            fraction = 4U,
            angle_deg = 5U,
            duration_ms = 6U,
            color_rgba = 7U,
            string = 8U,
            separator = 9U
        } kind;
        double number;
        std::uint32_t data;
        const char* text;
    };
    const ValueToken* tokens;
    std::size_t token_count;
};

struct StyleRecord final {
    const Property* properties;
    std::size_t property_count;
};

struct NodeRecord final {
    std::size_t source_index;
    std::size_t parent_index;
    bool has_parent;
    bool runtime;
    const char* tag;
    const char* stable_id;
    const char* control_kind;
    const char* text;
    std::size_t geometry_style;
    std::size_t typography_style;
    std::size_t material_style;
    SurfaceMode surface;
    StyleExposure style_exposure;
};

struct VariantRecord final {
    std::size_t owner_index;
    const char* states;
    std::size_t geometry_style;
    std::size_t typography_style;
    std::size_t material_style;
};

struct DecorationRecord final {
    std::size_t owner_index;
    const char* pseudo_element;
    const char* states;
    const char* content;
    std::size_t geometry_style;
    std::size_t typography_style;
    std::size_t material_style;
};

struct RequirementSet final {
    const char* const* values;
    std::size_t count;
};

struct FormDescriptor final {
    const char* schema;
    const char* profile;
    const char* source_digest;
    const char* title;
    const NodeRecord* nodes;
    std::size_t node_count;
    const StyleRecord* geometry_styles;
    std::size_t geometry_style_count;
    const StyleRecord* typography_styles;
    std::size_t typography_style_count;
    const StyleRecord* material_styles;
    std::size_t material_style_count;
    const VariantRecord* variants;
    std::size_t variant_count;
    const DecorationRecord* decorations;
    std::size_t decoration_count;
    RequirementSet elements;
    RequirementSet control_kinds;
    RequirementSet properties;
    RequirementSet states;
    RequirementSet features;
};

}  // namespace generated
}  // namespace web_forms
