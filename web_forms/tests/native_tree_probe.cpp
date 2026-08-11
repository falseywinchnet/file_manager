#include "standard_shell_sapphire.gui_tree.wf.hpp"

#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        web_forms_generated_standard_shell_sapphire::NativeForm form =
            web_forms_generated_standard_shell_sapphire::make_native_form();
        require(form.root_control() == form.standard_app,
                "typed root must be the retained body control");
        require(form.standard_app->stable_id().value() ==
                    std::string_view("standard-app"),
                "root StableId must preserve source identity");
        require(form.standard_app->children().size() == 1U &&
                    form.standard_app->children()[0] == form.standard_app_shell,
                "body must own the shell rather than overlay it");
        require(form.standard_app_shell->children().size() == 4U,
                "shell must retain title, menu, workspace, and status in DOM order");
        require(form.standard_app_shell_workspace->children().size() == 2U,
                "workspace grid must retain its sidebar and content cells");
        require(form.standard_app_shell_workspace_content_cards->children().size() ==
                    2U,
                "card grid must retain both authored cards");
        require(form.standard_app_shell_workspace_content_heading_new->stable_id().value() ==
                    std::string_view(
                        "standard-app.shell.workspace.content.heading.new"),
                "typed command member must preserve its hierarchical id");

        gui_forms::Window window(form.root_control(), {1080.0, 720.0});
        window.perform_layout();
        const gui_forms::Rect shell =
            form.standard_app_shell->committed_arranged_bounds();
        require(shell.x == 34.0 && shell.y == 34.0 && shell.width == 1012.0 &&
                    shell.height >= 650.0,
                "CSS padding, horizontal auto margins, width, and min-height must compose");
        const gui_forms::Rect workspace =
            form.standard_app_shell_workspace->committed_arranged_bounds();
        const gui_forms::Rect sidebar =
            form.standard_app_shell_workspace_sidebar->committed_arranged_bounds();
        const gui_forms::Rect content =
            form.standard_app_shell_workspace_content->committed_arranged_bounds();
        if (!(workspace.width == shell.width && sidebar.width == 210.0 &&
              content.x == 210.0 &&
              content.width == workspace.width - 210.0)) {
            std::cerr << "workspace=" << workspace.x << ',' << workspace.y << ','
                      << workspace.width << ',' << workspace.height
                      << " sidebar=" << sidebar.x << ',' << sidebar.y << ','
                      << sidebar.width << ',' << sidebar.height
                      << " content=" << content.x << ',' << content.y << ','
                      << content.width << ',' << content.height << '\n';
        }
        require(workspace.width == shell.width && sidebar.width == 210.0 &&
                    content.x == 210.0 &&
                    content.width == workspace.width - 210.0,
                "grid tracks must relax from 210px plus one fraction");
        std::cout << "web_forms_native_tree_probe: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "web_forms_native_tree_probe: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
