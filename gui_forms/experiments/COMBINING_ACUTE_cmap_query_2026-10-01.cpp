#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>
#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

struct Library final {
    FT_Library value{};
    Library() {
        const FT_Error error = FT_Init_FreeType(&value);
        if (error != 0) { throw std::runtime_error("FT init"); }
    }
    ~Library() { if (value != nullptr) { FT_Done_FreeType(value); } }
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;
};
struct Face final {
    FT_Face value{};
    Face() = default;
    ~Face() { if (value != nullptr) { FT_Done_Face(value); } }
    Face(const Face&) = delete;
    Face& operator=(const Face&) = delete;
};
int main(int argc, char** argv) {
    try {
        if (argc != 2) { throw std::runtime_error("font directory required"); }
        Library library{};
        constexpr std::array<const char*, 4> names{"Carlito-Regular.ttf", "NotoSansArabic-Regular.ttf",
            "NotoSansHebrew-Regular.ttf", "NotoEmoji-Regular.ttf"};
        constexpr std::array<std::string_view, 4> samples{"a\xcc\x81", "\xc3\xa1", "a", "\xcc\x81"};
        const std::filesystem::path root(argv[1]);
        std::cout << "samples,0=U0061+U0301,1=U00E1,2=U0061,3=U0301\n";
        for (const char* name : names) {
            const std::filesystem::path path = root / name;
            const std::string native_path = path.string();
            Face face{};
            const FT_Error opened = FT_New_Face(library.value, native_path.c_str(), 0, &face.value);
            if (opened != 0) { throw std::runtime_error("FT face"); }
            const FT_Error selected = FT_Select_Charmap(face.value, FT_ENCODING_UNICODE);
            const FT_Error sized = FT_Set_Char_Size(face.value, 0, 20 * 64, 72, 72);
            if (selected != 0 || sized != 0) { throw std::runtime_error("FT profile"); }
            std::cout << name << ",cmap_a=" << FT_Get_Char_Index(face.value, 0x61)
                      << ",cmap_acute=" << FT_Get_Char_Index(face.value, 0x301)
                      << ",cmap_aacute=" << FT_Get_Char_Index(face.value, 0xe1)
                      << ",cmap_0350=" << FT_Get_Char_Index(face.value, 0x350) << '\n';
            using FontOwner = std::unique_ptr<hb_font_t, void (*)(hb_font_t*)>;
            hb_font_t* acquired = hb_ft_font_create_referenced(face.value);
            FontOwner font(acquired, hb_font_destroy);
            if (acquired == hb_font_get_empty() || hb_ft_font_get_ft_face(acquired) != face.value) {
                throw std::runtime_error("HB font unavailable");
            }
            hb_ft_font_set_load_flags(font.get(), FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
            std::size_t sample_index = 0;
            for (const std::string_view input : samples) {
                using BufferOwner = std::unique_ptr<hb_buffer_t, void (*)(hb_buffer_t*)>;
                hb_buffer_t* acquired_buffer = hb_buffer_create();
                BufferOwner buffer(acquired_buffer, hb_buffer_destroy);
                if (!hb_buffer_allocation_successful(acquired_buffer)) { throw std::runtime_error("HB buffer unavailable"); }
                const int length = static_cast<int>(input.size());
                hb_buffer_add_utf8(buffer.get(), input.data(), length, 0, length);
                hb_buffer_set_cluster_level(buffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
                hb_buffer_set_direction(buffer.get(), HB_DIRECTION_LTR);
                hb_buffer_guess_segment_properties(buffer.get());
                const hb_bool_t shaped = hb_shape_full(font.get(), buffer.get(), nullptr, 0, nullptr);
                if (!shaped || !hb_buffer_allocation_successful(buffer.get())) { throw std::runtime_error("HB failed"); }
                unsigned count = 0;
                const hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer.get(), &count);
                std::cout << "sample," << sample_index << ",glyphs";
                for (unsigned index = 0; index < count; ++index) {
                    std::cout << ',' << glyphs[index].codepoint << '@' << glyphs[index].cluster;
                }
                std::cout << '\n';
                ++sample_index;
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
