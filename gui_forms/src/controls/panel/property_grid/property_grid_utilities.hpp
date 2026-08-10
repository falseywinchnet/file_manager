#pragma once

#include "gui_forms/inspection/inspection_types.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace gui_forms::detail {

struct CompoundFieldSpec final {
    CompoundFieldSpec(std::string_view authored_name,
                      BindingValueKind authored_kind,
                      PropertyEditorKind authored_editor =
                          PropertyEditorKind::text,
                      std::vector<std::string> authored_choices = {})
        : name(authored_name), kind(authored_kind), editor(authored_editor),
          choices(std::move(authored_choices)) {}

    std::string_view name;
    BindingValueKind kind;
    PropertyEditorKind editor{PropertyEditorKind::text};
    std::vector<std::string> choices;
};

inline std::vector<CompoundFieldSpec> compound_fields(BindingValueKind kind) {
    switch (kind) {
    case BindingValueKind::point:
        return {{"X", BindingValueKind::number},
                {"Y", BindingValueKind::number}};
    case BindingValueKind::size:
        return {{"Width", BindingValueKind::number},
                {"Height", BindingValueKind::number}};
    case BindingValueKind::rectangle:
        return {{"X", BindingValueKind::number},
                {"Y", BindingValueKind::number},
                {"Width", BindingValueKind::number},
                {"Height", BindingValueKind::number}};
    case BindingValueKind::insets:
        return {{"Left", BindingValueKind::number},
                {"Top", BindingValueKind::number},
                {"Right", BindingValueKind::number},
                {"Bottom", BindingValueKind::number}};
    case BindingValueKind::color:
        return {{"Red", BindingValueKind::unsigned_integer},
                {"Green", BindingValueKind::unsigned_integer},
                {"Blue", BindingValueKind::unsigned_integer},
                {"Alpha", BindingValueKind::unsigned_integer}};
    case BindingValueKind::font:
        return {{"Role", BindingValueKind::text, PropertyEditorKind::choice,
                 {"Control", "Content", "Monospace"}},
                {"Size", BindingValueKind::number},
                {"Weight", BindingValueKind::unsigned_integer},
                {"Italic", BindingValueKind::boolean,
                 PropertyEditorKind::boolean},
                {"LetterSpacing", BindingValueKind::number}};
    default:
        return {};
    }
}

inline std::optional<BindingValue> compound_field_value(
    const BindingValue& value, std::string_view field_name) {
    const std::string field = canonical_binding_name(field_name);
    if (const gui_forms::Point* item = std::get_if<Point>(&value)) {
        if (field == "x") return BindingValue{(*item).x};
        if (field == "y") return BindingValue{(*item).y};
    } else if (const gui_forms::Size* item = std::get_if<Size>(&value)) {
        if (field == "width") return BindingValue{(*item).width};
        if (field == "height") return BindingValue{(*item).height};
    } else if (const gui_forms::Rect* item = std::get_if<Rect>(&value)) {
        if (field == "x") return BindingValue{(*item).x};
        if (field == "y") return BindingValue{(*item).y};
        if (field == "width") return BindingValue{(*item).width};
        if (field == "height") return BindingValue{(*item).height};
    } else if (const gui_forms::Insets* item = std::get_if<Insets>(&value)) {
        if (field == "left") return BindingValue{(*item).left};
        if (field == "top") return BindingValue{(*item).top};
        if (field == "right") return BindingValue{(*item).right};
        if (field == "bottom") return BindingValue{(*item).bottom};
    } else if (const gui_forms::Color* item = std::get_if<Color>(&value)) {
        if (field == "red") return BindingValue{static_cast<std::uint64_t>((*item).red)};
        if (field == "green") return BindingValue{static_cast<std::uint64_t>((*item).green)};
        if (field == "blue") return BindingValue{static_cast<std::uint64_t>((*item).blue)};
        if (field == "alpha") return BindingValue{static_cast<std::uint64_t>((*item).alpha)};
    } else if (const gui_forms::FontSpec* item = std::get_if<FontSpec>(&value)) {
        if (field == "role") {
            switch ((*item).role) {
            case FontRole::control: return BindingValue{std::string("Control")};
            case FontRole::content: return BindingValue{std::string("Content")};
            case FontRole::monospace: return BindingValue{std::string("Monospace")};
            }
        }
        if (field == "size") return BindingValue{(*item).size};
        if (field == "weight") {
            return BindingValue{static_cast<std::uint64_t>((*item).weight)};
        }
        if (field == "italic") return BindingValue{(*item).italic};
        if (field == "letterspacing") return BindingValue{(*item).letter_spacing};
    }
    return {};
}

inline std::optional<BindingValue> replace_compound_field(
    BindingValue value, std::string_view field_name,
    const BindingValue& replacement) {
    const std::string field = canonical_binding_name(field_name);
    if (gui_forms::Point* item = std::get_if<Point>(&value)) {
        const std::optional<BindingValue> converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "x") (*item).x = std::get<double>(*converted);
        else if (field == "y") (*item).y = std::get<double>(*converted);
        else return {};
    } else if (gui_forms::Size* item = std::get_if<Size>(&value)) {
        const std::optional<BindingValue> converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "width") (*item).width = std::get<double>(*converted);
        else if (field == "height") (*item).height = std::get<double>(*converted);
        else return {};
    } else if (gui_forms::Rect* item = std::get_if<Rect>(&value)) {
        const std::optional<BindingValue> converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "x") (*item).x = std::get<double>(*converted);
        else if (field == "y") (*item).y = std::get<double>(*converted);
        else if (field == "width") (*item).width = std::get<double>(*converted);
        else if (field == "height") (*item).height = std::get<double>(*converted);
        else return {};
    } else if (gui_forms::Insets* item = std::get_if<Insets>(&value)) {
        const std::optional<BindingValue> converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "left") (*item).left = std::get<double>(*converted);
        else if (field == "top") (*item).top = std::get<double>(*converted);
        else if (field == "right") (*item).right = std::get<double>(*converted);
        else if (field == "bottom") (*item).bottom = std::get<double>(*converted);
        else return {};
    } else if (gui_forms::Color* item = std::get_if<Color>(&value)) {
        const std::optional<BindingValue> converted = convert_binding_value(
            replacement, BindingValueKind::unsigned_integer);
        if (!converted || std::get<std::uint64_t>(*converted) > 255U) return {};
        const std::uint8_t channel = static_cast<std::uint8_t>(
            std::get<std::uint64_t>(*converted));
        if (field == "red") (*item).red = channel;
        else if (field == "green") (*item).green = channel;
        else if (field == "blue") (*item).blue = channel;
        else if (field == "alpha") (*item).alpha = channel;
        else return {};
    } else if (gui_forms::FontSpec* item = std::get_if<FontSpec>(&value)) {
        if (field == "role") {
            const std::string role = canonical_binding_name(
                binding_value_to_string(replacement));
            if (role == "control") (*item).role = FontRole::control;
            else if (role == "content") (*item).role = FontRole::content;
            else if (role == "monospace") (*item).role = FontRole::monospace;
            else return {};
        } else if (field == "size" || field == "letterspacing") {
            const std::optional<BindingValue> converted = convert_binding_value(
                replacement, BindingValueKind::number);
            if (!converted) return {};
            if (field == "size") (*item).size = std::get<double>(*converted);
            else (*item).letter_spacing = std::get<double>(*converted);
        } else if (field == "weight") {
            const std::optional<BindingValue> converted = convert_binding_value(
                replacement, BindingValueKind::unsigned_integer);
            if (!converted || std::get<std::uint64_t>(*converted) == 0U ||
                std::get<std::uint64_t>(*converted) > 1000U) return {};
            (*item).weight = static_cast<std::uint16_t>(
                std::get<std::uint64_t>(*converted));
        } else if (field == "italic") {
            const std::optional<BindingValue> converted = convert_binding_value(
                replacement, BindingValueKind::boolean);
            if (!converted) return {};
            (*item).italic = std::get<bool>(*converted);
        } else {
            return {};
        }
    } else {
        return {};
    }
    return value;
}

} // namespace gui_forms::detail
