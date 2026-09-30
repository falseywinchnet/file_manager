#include "linux_accessibility.hpp"
#include <atk/atk.h>
#include <atk-bridge.h>
#include <algorithm>
#include <cmath>
#include <locale>
#include <map>
#include <poll.h>
#include <sstream>
#include <unordered_map>
namespace gui_forms::host::linux_detail {
namespace {
struct NodeData {
    LinuxAccessibility::Impl* owner = nullptr;
    SemanticNode node;
    AtkObject* parent = nullptr;
    std::vector<AtkObject*> children;
    int index = 0;
    bool application = false, live = true;
    unsigned flags = 0;
};
struct NativeAccessible { AtkObject parent; NodeData* data; };
NodeData& data(gpointer object) { return *(*static_cast<NativeAccessible*>(object)).data; }
AtkObject* application_root = nullptr;
std::size_t bridge_count = 0;
AtkObject* root_object() { return application_root; }
const gchar* toolkit_name() { return "GUI.Forms"; }
const gchar* toolkit_version() { return "0.1"; }
AtkRole native_role(SemanticRole role) {
    switch (role) {
    case SemanticRole::button: return ATK_ROLE_PUSH_BUTTON;
    case SemanticRole::check_box: case SemanticRole::check_list_item: return ATK_ROLE_CHECK_BOX;
    case SemanticRole::radio_button: return ATK_ROLE_RADIO_BUTTON;
    case SemanticRole::text_box: return ATK_ROLE_ENTRY;
    case SemanticRole::numeric_field: return ATK_ROLE_SPIN_BUTTON;
    case SemanticRole::list: return ATK_ROLE_LIST;
    case SemanticRole::list_item: return ATK_ROLE_LIST_ITEM;
    case SemanticRole::combo_box: return ATK_ROLE_COMBO_BOX;
    case SemanticRole::slider: return ATK_ROLE_SLIDER;
    case SemanticRole::scroll_bar: return ATK_ROLE_SCROLL_BAR;
    case SemanticRole::progress_bar: return ATK_ROLE_PROGRESS_BAR;
    case SemanticRole::static_text: return ATK_ROLE_LABEL;
    case SemanticRole::image: return ATK_ROLE_IMAGE;
    case SemanticRole::tab: return ATK_ROLE_PAGE_TAB;
    case SemanticRole::tab_group: return ATK_ROLE_PAGE_TAB_LIST;
    case SemanticRole::menu_bar: return ATK_ROLE_MENU_BAR;
    case SemanticRole::menu: return ATK_ROLE_MENU;
    case SemanticRole::menu_item: case SemanticRole::menu_bar_item: return ATK_ROLE_MENU_ITEM;
    case SemanticRole::tool_tip: return ATK_ROLE_TOOL_TIP;
    case SemanticRole::separator: return ATK_ROLE_SEPARATOR;
    case SemanticRole::link: return ATK_ROLE_LINK;
    default: return ATK_ROLE_PANEL;
    }
}
void finalize(GObject* object) {
    delete (*reinterpret_cast<NativeAccessible*>(object)).data;
    GObjectClass* parent = G_OBJECT_CLASS(g_type_class_peek(ATK_TYPE_OBJECT));
    (*parent).finalize(object);
}
const gchar* name(AtkObject* object) { return data(object).node.name.c_str(); }
const gchar* description(AtkObject* object) { return data(object).node.description.c_str(); }
AtkObject* parent(AtkObject* object) { return data(object).live ? data(object).parent : nullptr; }
gint index_in_parent(AtkObject* object) { return data(object).live ? data(object).index : -1; }
gint child_count(AtkObject* object) { return data(object).live ? static_cast<gint>(data(object).children.size()) : 0; }
AtkObject* child_at(AtkObject* object, gint index) {
    const NodeData& value = data(object);
    if (!value.live || index < 0 || static_cast<std::size_t>(index) >= value.children.size()) { return nullptr; }
    return ATK_OBJECT(g_object_ref(value.children[index]));
}
AtkRole role(AtkObject* object) {
    const NodeData& value = data(object);
    return value.application ? ATK_ROLE_APPLICATION : value.node.runtime_id == 0 ? ATK_ROLE_FRAME :
        has_semantic_state(value.node.states, SemanticState::protected_content) ? ATK_ROLE_PASSWORD_TEXT : native_role(value.node.role);
}
AtkStateSet* states(AtkObject* object) {
    AtkStateSet* result = atk_state_set_new();
    const NodeData& value = data(object);
    if (!value.live) { atk_state_set_add_state(result, ATK_STATE_DEFUNCT); return result; }
    struct Mapping { SemanticState source; AtkStateType target; };
    const Mapping mappings[] = {{SemanticState::enabled, ATK_STATE_ENABLED}, {SemanticState::enabled, ATK_STATE_SENSITIVE},
        {SemanticState::visible, ATK_STATE_VISIBLE}, {SemanticState::visible, ATK_STATE_SHOWING},
        {SemanticState::focusable, ATK_STATE_FOCUSABLE}, {SemanticState::focused, ATK_STATE_FOCUSED},
        {SemanticState::selected, ATK_STATE_SELECTED}, {SemanticState::checked, ATK_STATE_CHECKED},
        {SemanticState::mixed, ATK_STATE_INDETERMINATE}, {SemanticState::expanded, ATK_STATE_EXPANDED},
        {SemanticState::busy, ATK_STATE_BUSY}};
    for (const Mapping& mapping : mappings) {
        if (has_semantic_state(value.node.states, mapping.source)) { atk_state_set_add_state(result, mapping.target); }
    }
    if (value.node.role == SemanticRole::text_box && !has_semantic_state(value.node.states, SemanticState::read_only)) {
        atk_state_set_add_state(result, ATK_STATE_EDITABLE);
    }
    return result;
}
void class_init(gpointer raw, gpointer) {
    AtkObjectClass& object = *static_cast<AtkObjectClass*>(raw);
    object.get_name = name; object.get_description = description; object.get_parent = parent;
    object.get_index_in_parent = index_in_parent; object.get_n_children = child_count;
    object.ref_child = child_at; object.get_role = role; object.ref_state_set = states;
    (*G_OBJECT_CLASS(raw)).finalize = finalize;
}
void instance_init(GTypeInstance* instance, gpointer) {
    (*reinterpret_cast<NativeAccessible*>(instance)).data = new NodeData;
}
gboolean perform(gpointer object, SemanticAction action, const std::string& value = {});
gint action_count(AtkAction* object) { return static_cast<gint>(data(object).node.actions.size()); }
const gchar* action_name(AtkAction* object, gint index) {
    const std::vector<SemanticAction>& actions = data(object).node.actions;
    return index >= 0 && static_cast<std::size_t>(index) < actions.size() ? semantic_action_name(actions[index]) : nullptr;
}
gboolean do_action(AtkAction* object, gint index) {
    const std::vector<SemanticAction>& actions = data(object).node.actions;
    if (index < 0 || static_cast<std::size_t>(index) >= actions.size()) { return false; }
    return perform(object, actions[index]);
}
void action_init(gpointer raw, gpointer) {
    AtkActionIface& face = *static_cast<AtkActionIface*>(raw);
    face.get_n_actions = action_count; face.get_name = action_name; face.get_localized_name = action_name;
    face.do_action = do_action;
}
void extents(AtkComponent* object, gint* x, gint* y, gint* width, gint* height, AtkCoordType coordinates);
gboolean contains(AtkComponent* object, gint x, gint y, AtkCoordType coordinates) {
    gint left = 0, top = 0, width = 0, height = 0; extents(object, &left, &top, &width, &height, coordinates);
    return x >= left && y >= top && x < left + width && y < top + height;
}
AtkObject* at_point(AtkComponent* object, gint x, gint y, AtkCoordType coordinates) {
    const NodeData& value = data(object);
    for (std::size_t i = value.children.size(); i > 0; --i) {
        AtkObject* child = value.children[i - 1];
        if (contains(ATK_COMPONENT(child), x, y, coordinates)) {
            AtkObject* deepest = at_point(ATK_COMPONENT(child), x, y, coordinates);
            return deepest ? deepest : ATK_OBJECT(g_object_ref(child));
        }
    }
    return contains(object, x, y, coordinates) ? ATK_OBJECT(g_object_ref(object)) : nullptr;
}
gboolean focus(AtkComponent* object) { return perform(object, SemanticAction::focus); }
AtkLayer layer(AtkComponent*) { return ATK_LAYER_WIDGET; }
void component_init(gpointer raw, gpointer) {
    AtkComponentIface& face = *static_cast<AtkComponentIface*>(raw);
    face.get_extents = extents; face.contains = contains; face.ref_accessible_at_point = at_point;
    face.grab_focus = focus; face.get_layer = layer;
}
void value_and_text(AtkValue* object, gdouble* value, gchar** text) {
    if (value) { *value = data(object).node.numeric_value.value_or(0); }
    if (text) { *text = g_strdup(data(object).node.value.c_str()); }
}
AtkRange* range(AtkValue* object) {
    const SemanticNode& node = data(object).node;
    return node.minimum_value && node.maximum_value ? atk_range_new(*node.minimum_value, *node.maximum_value, nullptr) : nullptr;
}
void set_value(AtkValue* object, gdouble value) {
    if (!std::isfinite(value)) { return; }
    std::ostringstream text; text.imbue(std::locale::classic()); text.precision(17); text << value;
    static_cast<void>(perform(object, SemanticAction::set_value, text.str()));
}
void value_init(gpointer raw, gpointer) {
    AtkValueIface& face = *static_cast<AtkValueIface*>(raw);
    face.get_value_and_text = value_and_text; face.get_range = range; face.set_value = set_value;
}
gchar* text(AtkText* object, gint start, gint end) {
    const std::string& value = data(object).node.value;
    const glong count = g_utf8_strlen(value.c_str(), value.size());
    start = std::clamp(start, 0, static_cast<int>(count));
    end = end < 0 ? static_cast<int>(count) : std::clamp(end, start, static_cast<int>(count));
    return g_utf8_substring(value.c_str(), start, end);
}
gint text_count(AtkText* object) { return static_cast<gint>(g_utf8_strlen(data(object).node.value.c_str(), -1)); }
gunichar character(AtkText* object, gint offset) {
    const std::string& value = data(object).node.value;
    return offset >= 0 && offset < text_count(object) ? g_utf8_get_char(g_utf8_offset_to_pointer(value.c_str(), offset)) : 0;
}
void text_init(gpointer raw, gpointer) {
    AtkTextIface& face = *static_cast<AtkTextIface*>(raw);
    face.get_text = text; face.get_character_count = text_count; face.get_character_at_offset = character;
}
void set_contents(AtkEditableText* object, const gchar* value) {
    if (value && g_utf8_validate(value, -1, nullptr)) { static_cast<void>(perform(object, SemanticAction::set_value, value)); }
}
void editable_init(gpointer raw, gpointer) { (*static_cast<AtkEditableTextIface*>(raw)).set_text_contents = set_contents; }
AtkObject* make_object(unsigned flags) {
    static std::map<unsigned, GType> types;
    std::map<unsigned, GType>::const_iterator found = types.find(flags);
    GType type;
    if (found == types.end()) {
        const std::string name = "GUIFormsAccessible" + std::to_string(flags);
        type = g_type_register_static_simple(ATK_TYPE_OBJECT, name.c_str(), sizeof(AtkObjectClass), class_init,
                                             sizeof(NativeAccessible), instance_init, static_cast<GTypeFlags>(0));
        const GInterfaceInfo component{component_init, nullptr, nullptr}; g_type_add_interface_static(type, ATK_TYPE_COMPONENT, &component);
        if (flags & 1) { const GInterfaceInfo face{action_init, nullptr, nullptr}; g_type_add_interface_static(type, ATK_TYPE_ACTION, &face); }
        if (flags & 2) { const GInterfaceInfo face{value_init, nullptr, nullptr}; g_type_add_interface_static(type, ATK_TYPE_VALUE, &face); }
        if (flags & 4) {
            const GInterfaceInfo face{text_init, nullptr, nullptr}; g_type_add_interface_static(type, ATK_TYPE_TEXT, &face);
            const GInterfaceInfo editable{editable_init, nullptr, nullptr}; g_type_add_interface_static(type, ATK_TYPE_EDITABLE_TEXT, &editable);
        }
        types.emplace(flags, type);
    } else { type = (*found).second; }
    return ATK_OBJECT(g_object_new(type, nullptr));
}
}
struct LinuxAccessibility::Impl {
    Display* display;
    ::Window native;
    Window* model;
    AtkObject* frame = nullptr;
    std::unordered_map<std::uint64_t, AtkObject*> objects;
    std::uint64_t generation = 0;
    void sync(const SemanticNode& node, AtkObject* parent, int index) {
        std::unordered_map<std::uint64_t, AtkObject*>::iterator found = objects.find(node.runtime_id);
        const unsigned flags = (node.actions.empty() ? 0 : 1) | (node.numeric_value ? 2 : 0) |
            (node.role == SemanticRole::text_box ? 4 : 0);
        if (found != objects.end() && data((*found).second).flags != flags) {
            AtkObject* stale = (*found).second;
            NodeData& old = data(stale); old.live = false; old.owner = nullptr; old.children.clear(); old.parent = nullptr;
            atk_object_notify_state_change(stale, ATK_STATE_DEFUNCT, true);
            g_object_unref(stale); objects.erase(found); found = objects.end();
        }
        const bool created = found == objects.end();
        AtkObject* object = created ? make_object(flags) : (*found).second;
        if (created) { objects.emplace(node.runtime_id, object); atk_object_set_accessible_id(object, node.stable_id.c_str()); }
        NodeData& value = data(object);
        const SemanticState old_states = value.node.states;
        const std::string old_value = value.node.value, old_name = value.node.name;
        value.flags = flags; value.owner = this; value.node = node; value.node.children.clear(); value.live = true;
        value.parent = parent; value.index = index; value.children.clear();
        data(parent).children.push_back(object);
        if (created) { g_signal_emit_by_name(parent, "children-changed::add", index, object, nullptr); }
        if (old_name != node.name) { g_object_notify(G_OBJECT(object), "accessible-name"); }
        if (old_value != node.value) { g_object_notify(G_OBJECT(object), "accessible-value"); }
        struct StateMapping { SemanticState source; AtkStateType target; };
        const StateMapping changes[] = {{SemanticState::focused, ATK_STATE_FOCUSED},
            {SemanticState::enabled, ATK_STATE_ENABLED}, {SemanticState::enabled, ATK_STATE_SENSITIVE},
            {SemanticState::visible, ATK_STATE_VISIBLE}, {SemanticState::visible, ATK_STATE_SHOWING},
            {SemanticState::checked, ATK_STATE_CHECKED}, {SemanticState::mixed, ATK_STATE_INDETERMINATE},
            {SemanticState::selected, ATK_STATE_SELECTED}, {SemanticState::expanded, ATK_STATE_EXPANDED},
            {SemanticState::busy, ATK_STATE_BUSY}};
        for (const StateMapping& mapping : changes) {
            const bool current = has_semantic_state(node.states, mapping.source);
            if (has_semantic_state(old_states, mapping.source) != current) {
                atk_object_notify_state_change(object, mapping.target, current);
            }
        }
        for (std::size_t i = 0; i < node.children.size(); ++i) { sync(node.children[i], object, static_cast<int>(i)); }
    }
};
namespace {
gboolean perform(gpointer object, SemanticAction action, const std::string& value) {
    NodeData& node = data(object);
    if (!node.live || !node.owner || !(*node.owner).model) { return false; }
    try {
        if (action == SemanticAction::focus) { XSetInputFocus((*node.owner).display, (*node.owner).native, RevertToParent, CurrentTime); }
        return (*(*node.owner).model).perform_semantic_action(node.node.stable_id, action, value);
    } catch (...) { return false; }
}
void extents(AtkComponent* object, gint* x, gint* y, gint* width, gint* height, AtkCoordType coordinates) {
    const NodeData& node = data(object);
    int left = 0, top = 0;
    if (node.owner && coordinates == ATK_XY_SCREEN) {
        ::Window child{};
        XTranslateCoordinates((*node.owner).display, (*node.owner).native, DefaultRootWindow((*node.owner).display), 0, 0, &left, &top, &child);
    } else if (node.parent && coordinates == ATK_XY_PARENT) {
        left = -static_cast<int>(data(node.parent).node.bounds.x); top = -static_cast<int>(data(node.parent).node.bounds.y);
    }
    if (x) { *x = left + static_cast<int>(node.node.bounds.x); }
    if (y) { *y = top + static_cast<int>(node.node.bounds.y); }
    if (width) { *width = node.live ? static_cast<int>(node.node.bounds.width) : 0; }
    if (height) { *height = node.live ? static_cast<int>(node.node.bounds.height) : 0; }
}
}
LinuxAccessibility::LinuxAccessibility(Display* display, ::Window native, Window& model, const std::string& title)
    : impl_(std::make_unique<Impl>(Impl{display, native, &model})) {
    if (!application_root) {
        application_root = make_object(0); data(application_root).application = true;
        data(application_root).node.name = title;
        AtkUtilClass* util = ATK_UTIL_CLASS(g_type_class_ref(ATK_TYPE_UTIL));
        (*util).get_root = root_object; (*util).get_toolkit_name = toolkit_name; (*util).get_toolkit_version = toolkit_version;
        g_type_class_unref(util);
        static_cast<void>(atk_bridge_adaptor_init(nullptr, nullptr));
    }
    ++bridge_count;
    (*impl_).frame = make_object(0);
    NodeData& frame = data((*impl_).frame); frame.owner = impl_.get(); frame.parent = application_root;
    frame.index = static_cast<int>(data(application_root).children.size()); frame.node.name = title;
    data(application_root).children.push_back((*impl_).frame);
    g_signal_emit_by_name(application_root, "children-changed::add", frame.index, (*impl_).frame, nullptr);
    update();
}
LinuxAccessibility::~LinuxAccessibility() { detach(); }
void LinuxAccessibility::detach() {
    if (!(*impl_).model) { return; }
    (*impl_).model = nullptr;
    for (const std::pair<const std::uint64_t, AtkObject*>& entry : (*impl_).objects) {
        NodeData& node = data(entry.second); node.live = false; node.owner = nullptr; node.children.clear(); node.parent = nullptr;
        atk_object_notify_state_change(entry.second, ATK_STATE_DEFUNCT, true);
        g_object_unref(entry.second);
    }
    (*impl_).objects.clear();
    NodeData& frame = data((*impl_).frame); frame.live = false; frame.owner = nullptr; frame.children.clear();
    std::vector<AtkObject*>& frames = data(application_root).children;
    frames.erase(std::remove(frames.begin(), frames.end(), (*impl_).frame), frames.end());
    g_signal_emit_by_name(application_root, "children-changed::remove", frame.index, (*impl_).frame, nullptr);
    for (std::size_t i = 0; i < frames.size(); ++i) { data(frames[i]).index = static_cast<int>(i); }
    frame.parent = nullptr; g_object_unref((*impl_).frame); (*impl_).frame = nullptr;
    if (--bridge_count == 0) {
        atk_bridge_adaptor_cleanup(); g_object_unref(application_root); application_root = nullptr;
    }
}
void LinuxAccessibility::update() {
    if (!(*impl_).model) { return; }
    if ((*(*impl_).model).semantic_generation() == (*impl_).generation && !(*impl_).objects.empty()) { return; }
    const SemanticSnapshot snapshot = (*(*impl_).model).semantic_snapshot();
    if ((*impl_).generation == snapshot.generation && !(*impl_).objects.empty()) { return; }
    (*impl_).generation = snapshot.generation;
    for (const std::pair<const std::uint64_t, AtkObject*>& entry : (*impl_).objects) { data(entry.second).live = false; }
    data((*impl_).frame).children.clear();
    XWindowAttributes attributes{}; XGetWindowAttributes((*impl_).display, (*impl_).native, &attributes);
    data((*impl_).frame).node.bounds = {0, 0, static_cast<double>(attributes.width), static_cast<double>(attributes.height)};
    data((*impl_).frame).node.states = SemanticState::enabled | SemanticState::visible;
    for (std::size_t i = 0; i < snapshot.roots.size(); ++i) { (*impl_).sync(snapshot.roots[i], (*impl_).frame, static_cast<int>(i)); }
    for (std::unordered_map<std::uint64_t, AtkObject*>::iterator i = (*impl_).objects.begin(); i != (*impl_).objects.end();) {
        AtkObject* object = (*i).second; NodeData& node = data(object);
        if (node.live) { ++i; continue; }
        node.owner = nullptr; node.children.clear(); node.parent = nullptr;
        atk_object_notify_state_change(object, ATK_STATE_DEFUNCT, true);
        g_object_unref(object); i = (*impl_).objects.erase(i);
    }
}
void accessibility_dispatch() {
    for (int i = 0; i < 16 && g_main_context_pending(nullptr); ++i) { g_main_context_iteration(nullptr, false); }
}
void accessibility_wait(int display_fd, int wake_fd, int milliseconds) {
    GMainContext* context = g_main_context_default();
    if (!g_main_context_acquire(context)) { return; }
    gint priority = 0, timeout = -1;
    const gboolean ready = g_main_context_prepare(context, &priority);
    int count = g_main_context_query(context, priority, &timeout, nullptr, 0);
    std::vector<GPollFD> native(static_cast<std::size_t>(std::max(0, count)));
    count = g_main_context_query(context, priority, &timeout, native.data(), static_cast<int>(native.size()));
    if (count > static_cast<int>(native.size())) { g_main_context_release(context); return; }
    std::vector<pollfd> descriptors{{display_fd, POLLIN, 0}, {wake_fd, POLLIN, 0}};
    for (const GPollFD& descriptor : native) { descriptors.push_back({descriptor.fd, static_cast<short>(descriptor.events), 0}); }
    int wait = milliseconds < 0 ? timeout : timeout < 0 ? milliseconds : std::min(milliseconds, timeout);
    if (ready) { wait = 0; }
    static_cast<void>(poll(descriptors.data(), descriptors.size(), wait));
    for (std::size_t i = 0; i < native.size(); ++i) { native[i].revents = descriptors[i + 2].revents; }
    if (g_main_context_check(context, priority, native.data(), static_cast<int>(native.size()))) { g_main_context_dispatch(context); }
    g_main_context_release(context);
}
}
