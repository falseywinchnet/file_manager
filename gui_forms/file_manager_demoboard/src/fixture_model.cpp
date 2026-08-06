#include "file_manager_demoboard/fixture_model.hpp"

#include <algorithm>
#include <array>

namespace file_manager_demoboard {

FixtureCatalogue::FixtureCatalogue()
    : id_("file-manager-demoboard-001"), generation_(86),
      current_path_("/Users/quentin/Work/Projects"),
      project_objects_({
          {"obj-file-manager", "File Manager", "Folder", "1.3G", "folder-blue", false},
          {"obj-orchard-study", "Orchard Study", "Folder", "250M", "folder-green", false},
          {"obj-field-recordings", "Field Recordings", "Folder", "3.6G", "folder", false},
          {"obj-north-shore", "North Shore", "Folder", "725M", "folder", false},
          {"obj-print-masters", "Print Masters", "Folder", "90M", "folder", false},
          {"obj-facade-study", "Facade Study.png", "PNG image", "2.8 MB", "image", true},
          {"obj-material-notes", "Material notes.txt", "Plain text", "18 KB", "document", false},
          {"obj-palette-study", "palette-study.css", "Stylesheet", "31 KB", "code", false},
          {"obj-survey-plates", "Survey plates.pdf", "PDF document", "6.4 MB", "pdf", false},
          {"obj-room-tone", "Room tone 03.aiff", "Audio", "48 MB", "music", false},
          {"obj-delivery-archive", "Delivery 2026-07.zip", "ZIP archive", "12.7 MB", "archive", false},
          {"obj-invoice-quartz", "invoice quartz.txt", "Plain text", "42 KB", "document", false},
          {"obj-map-detail", "Map detail 17.tif", "TIFF image", "14.2 MB", "image", false},
          {"obj-index-fixture", "index fixture.json", "JSON document", "9 KB", "code", false},
      }),
      current_path_stack_({"projects", "/Users/quentin/Work/Projects",
          {"Macintosh HD", "Users", "quentin", "Work", "Projects"}, true}),
      recent_paths_({
          {"orchard-study", "/Users/quentin/Work/Projects/Orchard Study",
           {"Macintosh HD", "Users", "quentin", "Work", "Projects",
            "Orchard Study"}, true},
          {"legal-2026", "/Users/quentin/Documents/Legal/2026",
           {"Macintosh HD", "Users", "quentin", "Documents", "Legal",
            "2026"}, true},
          {"archive-material-library",
           "/Volumes/Archive 04/Reference/Material Library",
           {"Archive 04", "Reference", "Material Library"}, false},
          {"pictures-scans", "/Users/quentin/Pictures/Scans",
           {"Macintosh HD", "Users", "quentin", "Pictures", "Scans"}, true},
          {"reference-materials",
           "/Users/quentin/Work/Reference/Materials",
           {"Macintosh HD", "Users", "quentin", "Work", "Reference",
            "Materials"}, true},
      }),
      path_completions_({
          {"orchard-study", "/Users/quentin/Work/Projects/Orchard Study/"},
          {"north-shore", "/Users/quentin/Work/Projects/North Shore/"},
          {"print-masters", "/Users/quentin/Work/Projects/Print Masters/"},
      }) {}

const FixtureCatalogue& FixtureCatalogue::instance() {
    static const FixtureCatalogue catalogue;
    return catalogue;
}

FixturePathResolution FixtureCatalogue::resolve_path(
    std::string_view input) const {
    std::string expanded(input);
    const auto replace_prefix = [&expanded](std::string_view prefix,
                                            std::string_view replacement) {
        if (!expanded.starts_with(prefix)) return false;
        expanded.replace(0, prefix.size(), replacement);
        return true;
    };
    if (expanded.starts_with("$UNKNOWN") || expanded.starts_with("${UNKNOWN}")) {
        return {false, expanded, {},
                "Unknown variable $UNKNOWN · use ~, $HOME, or $PROJECTS"};
    }
    static_cast<void>(replace_prefix("${PROJECTS}",
                                      "/Users/quentin/Work/Projects"));
    static_cast<void>(replace_prefix("$PROJECTS",
                                      "/Users/quentin/Work/Projects"));
    static_cast<void>(replace_prefix("${HOME}", "/Users/quentin"));
    static_cast<void>(replace_prefix("$HOME", "/Users/quentin"));
    static_cast<void>(replace_prefix("~", "/Users/quentin"));
    while (expanded.size() > 1U && expanded.back() == '/') expanded.pop_back();

    const auto match = [&expanded](const FixturePath& path) {
        return path.path == expanded;
    };
    if (current_path_stack_.path == expanded) {
        return {true, expanded, current_path_stack_.id,
                "Resolved current fixture location"};
    }
    if (const auto found = std::find_if(recent_paths_.begin(),
                                        recent_paths_.end(), match);
        found != recent_paths_.end()) {
        return {true, expanded, found->id,
                found->available ? "Resolved fixture location"
                                 : "Resolved stored path · source volume offline"};
    }
    for (const auto& completion : path_completions_) {
        std::string path = completion.path;
        while (path.size() > 1U && path.back() == '/') path.pop_back();
        if (path == expanded) {
            return {true, expanded, completion.id,
                    "Resolved fixture location"};
        }
    }
    return {false, expanded, {},
            "No fixture destination matches “" + expanded + "”"};
}

std::vector<FixturePathCompletion> FixtureCatalogue::complete_path(
    std::string_view input) const {
    const FixturePathResolution expanded = resolve_path(input);
    std::string prefix = expanded.expanded_path;
    if (!expanded.valid) {
        prefix.assign(input);
        const auto expand_prefix = [&prefix](std::string_view token,
                                             std::string_view replacement) {
            if (prefix.starts_with(token)) prefix.replace(0, token.size(), replacement);
        };
        expand_prefix("${PROJECTS}", "/Users/quentin/Work/Projects");
        expand_prefix("$PROJECTS", "/Users/quentin/Work/Projects");
        expand_prefix("${HOME}", "/Users/quentin");
        expand_prefix("$HOME", "/Users/quentin");
        expand_prefix("~", "/Users/quentin");
    }
    std::vector<FixturePathCompletion> matches;
    for (const auto& completion : path_completions_) {
        if (completion.path.starts_with(prefix)) matches.push_back(completion);
    }
    return matches;
}

std::vector<CapabilityEntry> initial_capability_report() {
    return {
        {"FM-X01", "supported", "product shell uses only public C++ controls"},
        {"FM-X03", "supported", "retained controls and fixture objects have stable IDs"},
        {"FM-LY01", "supported", "table, flow, dock, anchor and split composition"},
        {"FM-LY05", "measured-partial", "split panes provide live bounded resize, useful maximums, remembered extents, automatic/user origin, and focus-safe seam tabs"},
        {"FM-T01", "measured-partial", "bundled Portsmouth, Carlito and Cousine specimen pack"},
        {"FM-O01", "measured-partial", "public TreeView has stable hierarchy, expansion, keyboard, type-select and bounded semantics"},
        {"FM-O02", "measured-partial", "public ObjectView provides stable virtual icon/details projections and bounded realization"},
        {"FM-O03", "measured-partial", "public ObjectView preserves stable multi-selection, primary, anchor and independent focus across view/sort changes"},
        {"FM-O11", "measured-partial", "public PropertyList owns grouped read-only/text/choice rows, inline validation, disclosures and one preview/property scroll plane"},
        {"FM-O12", "measured-partial", "Selection preview disclosure and session Name/handler commits are retained and accessible"},
        {"FM-C03", "measured-partial", "public Command and tokenized accelerators bind menu, ribbon, status and navigation presentations; complete shelf remains open"},
        {"FM-C05", "measured-partial", "public MenuStrip and ContextMenu provide retained switching, nested menus, keyboard focus scopes, edge avoidance and native menu semantics"},
        {"FM-W03", "unavailable", "custom host chrome/title participation is not yet public"},
        {"FM-A07", "unavailable", "native accessibility publisher closure remains open"},
    };
}

} // namespace file_manager_demoboard
