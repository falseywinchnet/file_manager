#include "windows_accessibility.hpp"
#include <oleacc.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <new>
namespace gui_forms::host {
namespace {
const SemanticNode* find_node(const SemanticNode& node, std::uint64_t id) {
    if (node.runtime_id == id) { return &node; }
    for (const SemanticNode& child : node.children) {
        const SemanticNode* found = find_node(child, id);
        if (found) { return found; }
    }
    return nullptr;
}
const SemanticNode* parent_node(const SemanticNode& node, std::uint64_t id) {
    for (const SemanticNode& child : node.children) {
        if (child.runtime_id == id) { return &node; }
        const SemanticNode* found = parent_node(child, id);
        if (found) { return found; }
    }
    return nullptr;
}
const SemanticNode* state_node(const SemanticNode& node, SemanticState state) {
    if (has_semantic_state(node.states, state)) { return &node; }
    for (const SemanticNode& child : node.children) {
        const SemanticNode* found = state_node(child, state);
        if (found) { return found; }
    }
    return nullptr;
}
const SemanticNode* hit_node(const SemanticNode& node, double x, double y) {
    for (std::size_t i = node.children.size(); i > 0; --i) {
        const SemanticNode* hit = hit_node(node.children[i - 1], x, y);
        if (hit) { return hit; }
    }
    if (has_semantic_state(node.states, SemanticState::visible) && x >= node.bounds.x &&
        y >= node.bounds.y && x < node.bounds.x + node.bounds.width && y < node.bounds.y + node.bounds.height) { return &node; }
    return nullptr;
}
HRESULT bstr(std::string_view value, BSTR* result) noexcept {
    if (!result) { return E_POINTER; }
    *result = nullptr;
    if (value.empty()) { return S_FALSE; }
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) { return E_FAIL; }
    *result = SysAllocStringLen(nullptr, count);
    if (!*result) { return E_OUTOFMEMORY; }
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), *result, count);
    return S_OK;
}
LONG role(SemanticRole value) noexcept {
    switch (value) {
    case SemanticRole::button: return ROLE_SYSTEM_PUSHBUTTON;
    case SemanticRole::check_box: case SemanticRole::check_list_item: return ROLE_SYSTEM_CHECKBUTTON;
    case SemanticRole::radio_button: return ROLE_SYSTEM_RADIOBUTTON;
    case SemanticRole::text_box: return ROLE_SYSTEM_TEXT;
    case SemanticRole::numeric_field: return ROLE_SYSTEM_SPINBUTTON;
    case SemanticRole::combo_box: return ROLE_SYSTEM_COMBOBOX;
    case SemanticRole::slider: return ROLE_SYSTEM_SLIDER;
    case SemanticRole::list: return ROLE_SYSTEM_LIST;
    case SemanticRole::list_item: return ROLE_SYSTEM_LISTITEM;
    case SemanticRole::tab_group: return ROLE_SYSTEM_PAGETABLIST;
    case SemanticRole::tab: return ROLE_SYSTEM_PAGETAB;
    case SemanticRole::menu_bar: return ROLE_SYSTEM_MENUBAR;
    case SemanticRole::menu: return ROLE_SYSTEM_MENUPOPUP;
    case SemanticRole::menu_bar_item: case SemanticRole::menu_item: return ROLE_SYSTEM_MENUITEM;
    case SemanticRole::static_text: return ROLE_SYSTEM_STATICTEXT;
    case SemanticRole::image: return ROLE_SYSTEM_GRAPHIC;
    case SemanticRole::scroll_bar: return ROLE_SYSTEM_SCROLLBAR;
    case SemanticRole::progress_bar: return ROLE_SYSTEM_PROGRESSBAR;
    case SemanticRole::link: return ROLE_SYSTEM_LINK;
    case SemanticRole::tool_tip: return ROLE_SYSTEM_TOOLTIP;
    case SemanticRole::separator: return ROLE_SYSTEM_SEPARATOR;
    default: return ROLE_SYSTEM_GROUPING;
    }
}
SemanticAction default_action(const SemanticNode& node) noexcept {
    for (SemanticAction action : {SemanticAction::press, SemanticAction::show_menu, SemanticAction::select,
                                  SemanticAction::expand, SemanticAction::focus}) {
        if (std::find(node.actions.begin(), node.actions.end(), action) != node.actions.end()) { return action; }
    }
    return SemanticAction::focus;
}
}
struct WindowsAccessibility::State {
    HWND hwnd = nullptr;
    Window* model = nullptr;
    double scale = 1;
    std::uint64_t generation = 0, focus = 0;
    bool observed = false;
    bool snapshot(SemanticNode& root) noexcept {
        if (!model || !hwnd) { return false; }
        try {
            SemanticSnapshot tree = (*model).semantic_snapshot();
            root.runtime_id = 0;
            root.role = SemanticRole::group;
            root.children = std::move(tree.roots);
            root.states = SemanticState::enabled | SemanticState::visible;
            wchar_t title[1024]{}; GetWindowTextW(hwnd, title, 1024);
            char utf8[4096]{}; WideCharToMultiByte(CP_UTF8, 0, title, -1, utf8, 4096, nullptr, nullptr);
            root.name = utf8;
            RECT bounds{}; GetClientRect(hwnd, &bounds);
            root.bounds = {0, 0, bounds.right / scale, bounds.bottom / scale};
            return true;
        } catch (...) { return false; }
    }
    bool perform(const SemanticNode& node, SemanticAction action, BSTR value = nullptr) noexcept {
        if (!model || node.runtime_id == 0) { return false; }
        try {
            std::string text;
            if (value) {
                const UINT length = SysStringLen(value);
                if (length > 1024 * 1024) { return false; }
                const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, length, nullptr, 0, nullptr, nullptr);
                if (size == 0 && length != 0) { return false; }
                text.resize(size);
                WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, length, text.data(), size, nullptr, nullptr);
            }
            if (action == SemanticAction::focus) { SetFocus(hwnd); }
            const bool handled = (*model).perform_semantic_action(node.stable_id, action, text);
            if (handled) { NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, hwnd, OBJID_CLIENT, CHILDID_SELF); }
            return handled;
        } catch (...) { return false; }
    }
};
class WindowsAccessibility::Accessible final : public IAccessible {
    std::atomic<ULONG> references_{1};
    std::shared_ptr<State> state_;
    std::uint64_t id_;
    const SemanticNode* node(SemanticNode& root, VARIANT child) noexcept {
        if (child.vt != VT_I4 || child.lVal < 0 || !(*state_).snapshot(root)) { return nullptr; }
        const SemanticNode* self = find_node(root, id_);
        if (!self || child.lVal == CHILDID_SELF) { return self; }
        const std::size_t index = static_cast<std::size_t>(child.lVal - 1);
        return index < (*self).children.size() ? &(*self).children[index] : nullptr;
    }
    HRESULT dispatch(std::uint64_t id, IDispatch** result) noexcept {
        if (!result) { return E_POINTER; }
        *result = new(std::nothrow) Accessible(state_, id);
        return *result ? S_OK : E_OUTOFMEMORY;
    }
    HRESULT variant(const SemanticNode* found, VARIANT* result) noexcept {
        if (!result) { return E_POINTER; }
        VariantInit(result);
        if (!found) { return S_FALSE; }
        // MSAA clients descend through VT_DISPATCH. Returning a new wrapper for
        // this same object makes focus and hit-test traversal recurse forever.
        if ((*found).runtime_id == id_) {
            (*result).vt = VT_I4;
            (*result).lVal = CHILDID_SELF;
            return S_OK;
        }
        (*result).vt = VT_DISPATCH;
        return dispatch((*found).runtime_id, &(*result).pdispVal);
    }
    HRESULT property(VARIANT child, BSTR* value, int kind) noexcept {
        if (!value) { return E_POINTER; } *value = nullptr;
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!found) { return CO_E_OBJNOTCONNECTED; }
        return bstr(kind == 0 ? (*found).name : kind == 1 ? (*found).value : (*found).description, value);
    }
  public:
    Accessible(std::shared_ptr<State> state, std::uint64_t id) : state_(std::move(state)), id_(id) {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** out) override {
        if (!out) { return E_POINTER; } *out = nullptr;
        if (iid == IID_IUnknown || iid == IID_IDispatch || iid == IID_IAccessible) {
            *out = static_cast<IAccessible*>(this); AddRef(); return S_OK;
        }
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() override { const ULONG n = --references_; if (!n) { delete this; } return n; }
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* out) override { if (!out) { return E_POINTER; } *out = 0; return S_OK; }
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo** out) override { if (out) { *out = nullptr; } return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** out) override {
        if (!out) { return E_POINTER; } *out = nullptr;
        SemanticNode root; if (!(*state_).snapshot(root)) { return CO_E_OBJNOTCONNECTED; }
        if (id_ == 0) { return CreateStdAccessibleObject((*state_).hwnd, OBJID_WINDOW, IID_IDispatch, reinterpret_cast<void**>(out)); }
        const SemanticNode* parent = parent_node(root, id_);
        return parent ? dispatch((*parent).runtime_id, out) : CO_E_OBJNOTCONNECTED;
    }
    HRESULT STDMETHODCALLTYPE get_accChildCount(LONG* out) override {
        if (!out) { return E_POINTER; } *out = 0;
        SemanticNode root; VARIANT self{}; self.vt = VT_I4;
        const SemanticNode* found = node(root, self);
        if (!found) { return CO_E_OBJNOTCONNECTED; } *out = static_cast<LONG>((*found).children.size()); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_accChild(VARIANT child, IDispatch** out) override {
        if (!out) { return E_POINTER; } *out = nullptr;
        SemanticNode root; const SemanticNode* found = node(root, child);
        return found ? dispatch((*found).runtime_id, out) : E_INVALIDARG;
    }
    HRESULT STDMETHODCALLTYPE get_accName(VARIANT child, BSTR* out) override { return property(child, out, 0); }
    HRESULT STDMETHODCALLTYPE get_accValue(VARIANT child, BSTR* out) override { return property(child, out, 1); }
    HRESULT STDMETHODCALLTYPE get_accDescription(VARIANT child, BSTR* out) override { return property(child, out, 2); }
    HRESULT STDMETHODCALLTYPE get_accRole(VARIANT child, VARIANT* out) override {
        if (!out) { return E_POINTER; } VariantInit(out);
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!found) { return CO_E_OBJNOTCONNECTED; }
        (*out).vt = VT_I4; (*out).lVal = (*found).runtime_id == 0 ? ROLE_SYSTEM_CLIENT : role((*found).role); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_accState(VARIANT child, VARIANT* out) override {
        if (!out) { return E_POINTER; } VariantInit(out);
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!found) { return CO_E_OBJNOTCONNECTED; }
        const SemanticState state = (*found).states;
        LONG bits = 0;
        if (!has_semantic_state(state, SemanticState::enabled)) { bits |= STATE_SYSTEM_UNAVAILABLE; }
        if (!has_semantic_state(state, SemanticState::visible)) { bits |= STATE_SYSTEM_INVISIBLE; }
        if (has_semantic_state(state, SemanticState::focused)) { bits |= STATE_SYSTEM_FOCUSED; }
        if (has_semantic_state(state, SemanticState::focusable)) { bits |= STATE_SYSTEM_FOCUSABLE; }
        if (has_semantic_state(state, SemanticState::checked)) { bits |= STATE_SYSTEM_CHECKED; }
        if (has_semantic_state(state, SemanticState::mixed)) { bits |= STATE_SYSTEM_MIXED; }
        if (has_semantic_state(state, SemanticState::selected)) { bits |= STATE_SYSTEM_SELECTED; }
        if (has_semantic_state(state, SemanticState::read_only)) { bits |= STATE_SYSTEM_READONLY; }
        if (has_semantic_state(state, SemanticState::expanded)) { bits |= STATE_SYSTEM_EXPANDED; }
        if (has_semantic_state(state, SemanticState::protected_content)) { bits |= STATE_SYSTEM_PROTECTED; }
        if (has_semantic_state(state, SemanticState::busy)) { bits |= STATE_SYSTEM_BUSY; }
        (*out).vt = VT_I4; (*out).lVal = bits; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE get_accHelp(VARIANT child, BSTR* out) override { return property(child, out, 2); }
    HRESULT STDMETHODCALLTYPE get_accHelpTopic(BSTR* file, VARIANT, LONG* topic) override { if (file) { *file = nullptr; } if (topic) { *topic = 0; } return S_FALSE; }
    HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(VARIANT, BSTR* out) override { if (!out) { return E_POINTER; } *out = nullptr; return S_FALSE; }
    HRESULT STDMETHODCALLTYPE get_accFocus(VARIANT* out) override {
        SemanticNode root; VARIANT self{}; self.vt = VT_I4; const SemanticNode* found = node(root, self);
        return variant(found ? state_node(*found, SemanticState::focused) : nullptr, out);
    }
    HRESULT STDMETHODCALLTYPE get_accSelection(VARIANT* out) override {
        SemanticNode root; VARIANT self{}; self.vt = VT_I4; const SemanticNode* found = node(root, self);
        return variant(found ? state_node(*found, SemanticState::selected) : nullptr, out);
    }
    HRESULT STDMETHODCALLTYPE get_accDefaultAction(VARIANT child, BSTR* out) override {
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!out) { return E_POINTER; } *out = nullptr;
        return found && !(*found).actions.empty() ? bstr(semantic_action_name(default_action(*found)), out) : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE accSelect(LONG flags, VARIANT child) override {
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!found) { return CO_E_OBJNOTCONNECTED; }
        if ((flags & ~(SELFLAG_TAKEFOCUS | SELFLAG_TAKESELECTION)) != 0 || flags == 0) { return E_INVALIDARG; }
        bool done = true;
        if (flags & SELFLAG_TAKEFOCUS) { done = (*state_).perform(*found, SemanticAction::focus); }
        if (flags & SELFLAG_TAKESELECTION) { done = (*state_).perform(*found, SemanticAction::select) && done; }
        return done ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE accLocation(LONG* x, LONG* y, LONG* width, LONG* height, VARIANT child) override {
        if (!x || !y || !width || !height) { return E_POINTER; }
        *x = *y = *width = *height = 0;
        SemanticNode root; const SemanticNode* found = node(root, child);
        if (!found) { return CO_E_OBJNOTCONNECTED; }
        POINT origin{}; ClientToScreen((*state_).hwnd, &origin);
        *x = origin.x + static_cast<LONG>(std::lround((*found).bounds.x * (*state_).scale));
        *y = origin.y + static_cast<LONG>(std::lround((*found).bounds.y * (*state_).scale));
        *width = static_cast<LONG>(std::lround((*found).bounds.width * (*state_).scale));
        *height = static_cast<LONG>(std::lround((*found).bounds.height * (*state_).scale)); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE accNavigate(LONG direction, VARIANT start, VARIANT* out) override {
        SemanticNode root; const SemanticNode* found = node(root, start); const SemanticNode* target = nullptr;
        if (found && direction == NAVDIR_FIRSTCHILD && !(*found).children.empty()) { target = &(*found).children.front(); }
        if (found && direction == NAVDIR_LASTCHILD && !(*found).children.empty()) { target = &(*found).children.back(); }
        if (found && (direction == NAVDIR_NEXT || direction == NAVDIR_PREVIOUS)) {
            const SemanticNode* parent = parent_node(root, (*found).runtime_id);
            if (parent) {
                for (std::size_t i = 0; i < (*parent).children.size(); ++i) {
                    if ((*parent).children[i].runtime_id != (*found).runtime_id) { continue; }
                    if (direction == NAVDIR_NEXT && i + 1 < (*parent).children.size()) { target = &(*parent).children[i + 1]; }
                    if (direction == NAVDIR_PREVIOUS && i > 0) { target = &(*parent).children[i - 1]; }
                }
            }
        }
        return variant(target, out);
    }
    HRESULT STDMETHODCALLTYPE accHitTest(LONG x, LONG y, VARIANT* out) override {
        SemanticNode root; VARIANT self{}; self.vt = VT_I4; const SemanticNode* found = node(root, self);
        POINT origin{}; ClientToScreen((*state_).hwnd, &origin);
        return variant(found ? hit_node(*found, (x - origin.x) / (*state_).scale, (y - origin.y) / (*state_).scale) : nullptr, out);
    }
    HRESULT STDMETHODCALLTYPE accDoDefaultAction(VARIANT child) override {
        SemanticNode root; const SemanticNode* found = node(root, child);
        return found && (*state_).perform(*found, default_action(*found)) ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE put_accName(VARIANT, BSTR) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE put_accValue(VARIANT child, BSTR value) override {
        SemanticNode root; const SemanticNode* found = node(root, child);
        return found && (*state_).perform(*found, SemanticAction::set_value, value) ? S_OK : S_FALSE;
    }
};
WindowsAccessibility::WindowsAccessibility() : state_(std::make_shared<State>()) {}
WindowsAccessibility::~WindowsAccessibility() { detach(); }
void WindowsAccessibility::attach(HWND hwnd, Window& model, double scale) {
    (*state_).hwnd = hwnd; (*state_).model = &model; (*state_).scale = scale;
}
void WindowsAccessibility::detach() noexcept { (*state_).model = nullptr; (*state_).hwnd = nullptr; }
LRESULT WindowsAccessibility::object(WPARAM parameter) noexcept {
    if (!(*state_).model) { return 0; }
    (*state_).observed = true;
    Accessible* root = new(std::nothrow) Accessible(state_, 0);
    if (!root) { return 0; }
    const LRESULT result = LresultFromObject(IID_IAccessible, parameter, root);
    (*root).Release(); return result;
}
void WindowsAccessibility::notify(double scale) noexcept {
    (*state_).scale = scale;
    if (!(*state_).model || !(*state_).observed ||
        (*(*state_).model).semantic_generation() == (*state_).generation) { return; }
    try {
        const SemanticSnapshot snapshot = (*(*state_).model).semantic_snapshot();
        if (snapshot.generation == (*state_).generation) { return; }
        (*state_).generation = snapshot.generation;
        NotifyWinEvent(EVENT_OBJECT_REORDER, (*state_).hwnd, OBJID_CLIENT, CHILDID_SELF);
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, (*state_).hwnd, OBJID_CLIENT, CHILDID_SELF);
        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, (*state_).hwnd, OBJID_CLIENT, CHILDID_SELF);
        std::uint64_t focused = 0;
        for (const SemanticNode& root : snapshot.roots) {
            const SemanticNode* found = state_node(root, SemanticState::focused);
            if (found) { focused = (*found).runtime_id; break; }
        }
        if (focused != (*state_).focus) {
            (*state_).focus = focused;
            NotifyWinEvent(EVENT_OBJECT_FOCUS, (*state_).hwnd, OBJID_CLIENT, CHILDID_SELF);
        }
    } catch (...) {}
}
}
