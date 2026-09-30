#include "application.hpp"
#include "about_view.hpp"
#include "file_manager/document_picker_view.hpp"
#include "file_manager/platform_paths.hpp"
#include "gui_forms/application.hpp"

#if defined(__APPLE__)
#include "gui_forms/platform/macos_host.hpp"
#endif

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

namespace {

// Main owns these models until the native loop returns. Callbacks borrow this
// record; they neither prolong its lifetime nor form model ownership cycles.
struct Runtime final {
    Runtime() = default;
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    std::shared_ptr<file_manager::Application> application{};
    std::shared_ptr<file_manager::DocumentPickerView> picker{};
    std::shared_ptr<file_manager::AboutView> about{};
    std::function<void()> show_picker{};
    std::function<void()> hide_picker{};
    std::function<void()> show_about{};
    std::function<void()> hide_about{};
    std::function<void()> wake{};
    gui_forms::ApplicationWindowHandle primary_handle{};
    gui_forms::ApplicationWindowHandle picker_handle{};
    gui_forms::Window* primary_window{};
    gui_forms::Control::Ptr picker_return_focus{};
    bool picker_open{};
    bool primary_was_enabled{};

    ~Runtime() { if (application) (*application).stop(); }

    void present_picker(const std::filesystem::path& location) {
        if (picker_open || !show_picker || primary_window == nullptr) return;
        picker_return_focus = (*primary_window).focused_control();
        primary_was_enabled = (*(*primary_window).root()).enabled();
        (*picker).set_authority_valid(true);
        (*picker).present(location);
        picker_open = true;
        (*(*primary_window).root()).set_enabled(false);
        show_picker();
    }
    void picker_closing(gui_forms::HostCloseRequest&) { (*picker).cancel(); }
    void present_about() { if (show_about) show_about(); }
    void dismiss_about() { if (hide_about) hide_about(); }
    void picker_completed(const file_manager::DocumentPickerResult& result) {
        if (result.terminal == file_manager::DocumentPickerTerminal::accepted ||
            result.terminal == file_manager::DocumentPickerTerminal::cancelled) {
            if (hide_picker) hide_picker();
            if (picker_open && primary_window != nullptr) {
                (*(*primary_window).root()).set_enabled(primary_was_enabled);
                if (picker_return_focus) (*primary_window).request_focus(picker_return_focus);
            }
            picker_return_focus.reset();
            picker_open = false;
        }
        (*application).document_picker_completed(result);
    }
    void drain() { (*application).drain_ui(); }
    void stop() {
        // Native teardown can close the owner before its secondary window.
        // Cancel selection without trying to restore focus into that owner.
        primary_window = nullptr;
        if (picker_open) (*picker).cancel();
        (*picker).set_authority_valid(false);
        (*application).stop();
    }
    void close_primary() { static_cast<void>(primary_handle.request_close()); }
    void accept_wake(std::function<void()> value) { wake = std::move(value); }
    void primary_ready(gui_forms::Window&, gui_forms::ApplicationWindowHandle handle) {
        primary_handle = handle;
        (*application).bind_host(wake, std::bind_front(&Runtime::close_primary, this));
    }
    void picker_visibility(std::function<void()> show, std::function<void()> hide) {
        show_picker = std::move(show);
        hide_picker = std::move(hide);
    }
    void about_visibility(std::function<void()> show, std::function<void()> hide) {
        show_about = std::move(show);
        hide_about = std::move(hide);
    }
    void picker_ready(gui_forms::Window&, gui_forms::ApplicationWindowHandle handle) {
        picker_handle = handle;
        show_picker = std::bind_front(&Runtime::show_portable_picker, this);
        hide_picker = std::bind_front(&gui_forms::ApplicationWindowHandle::hide, handle);
    }
    void show_portable_picker() {
        if (!picker_handle.show().accepted()) (*picker).cancel();
    }
    void about_ready(gui_forms::Window&, gui_forms::ApplicationWindowHandle handle) {
        show_about = std::bind_front(&gui_forms::ApplicationWindowHandle::show, handle);
        hide_about = std::bind_front(&gui_forms::ApplicationWindowHandle::hide, handle);
    }
#if defined(__APPLE__)
    void mac_ready(std::function<void()> host_wake, std::function<void()> close,
        std::function<gui_forms::HostDialogResult(const gui_forms::HostDialogRequest&)>,
        std::function<gui_forms::HostServiceStatus(const gui_forms::HostTooltipRequest&)>,
        std::function<void()>, std::function<gui_forms::HostClipboardTextResult()>,
        std::function<gui_forms::HostServiceStatus(std::string_view)>) {
        (*application).bind_host(std::move(host_wake), std::move(close));
    }
#endif
};

std::filesystem::path default_root() {
#if defined(_WIN32)
    const wchar_t* const configured = _wgetenv(L"FILE_MANAGER_ROOT");
#else
    const char* const configured = std::getenv("FILE_MANAGER_ROOT");
#endif
    if (configured != nullptr && *configured != 0) {
        const std::filesystem::path result(configured);
        return result;
    }
    const std::filesystem::path result = file_manager::user_home_directory();
    return result;
}

#if defined(_WIN32)
// Owns the single LocalAlloc block, including all count argument strings.
struct WindowsArgumentOwner final {
    explicit WindowsArgumentOwner(LPWSTR* const arguments) : values(arguments) {}
    ~WindowsArgumentOwner() { LocalFree(values); }
    WindowsArgumentOwner(const WindowsArgumentOwner&) = delete;
    WindowsArgumentOwner& operator=(const WindowsArgumentOwner&) = delete;
    LPWSTR* values{};
};
#endif

std::vector<std::string> command_arguments(const int argc, char** const argv) {
    std::vector<std::string> result{};
#if defined(_WIN32)
    static_cast<void>(argc);
    static_cast<void>(argv);
    int count{};
    LPWSTR* const values = CommandLineToArgvW(GetCommandLineW(), &count);
    if (values == nullptr) throw std::runtime_error("cannot read Windows command line");
    const WindowsArgumentOwner owner(values);
    result.reserve(static_cast<std::size_t>(count));
    for (int index = 1; index < count; ++index) {
        const std::filesystem::path argument_path(owner.values[index]);
        std::string argument = file_manager::path_utf8(argument_path);
        result.push_back(std::move(argument));
    }
#else
    for (int index = 1; index < argc; ++index) result.emplace_back(argv[index]);
#endif
    return result;
}

} // namespace

int main(const int argc, char** argv) {
    try {
        std::filesystem::path root = default_root();
        std::optional<std::filesystem::path> quarantine{};
        std::string engine_root_id{};
        bool allow_mutations = false;
        const std::vector<std::string> arguments = command_arguments(argc, argv);
        for (std::size_t index = 0; index < arguments.size(); ++index) {
            const std::string_view argument(arguments[index]);
            if (argument == "--root" && index + 1 < arguments.size()) {
                ++index;
                root = file_manager::path_from_utf8(arguments[index]);
            } else if (argument == "--quarantine" && index + 1 < arguments.size()) {
                ++index;
                quarantine = file_manager::path_from_utf8(arguments[index]);
            } else if (argument == "--allow-mutations") {
                allow_mutations = true;
            } else if (argument == "--engine-root-id" && index + 1 < arguments.size()) {
                ++index;
                engine_root_id = arguments[index];
            } else {
                std::cerr << "usage: File Manager [--root DIRECTORY] "
                             "[--engine-root-id ID] "
                             "[--allow-mutations --quarantine DIRECTORY]\n";
                return 2;
            }
        }
        if (quarantine && !allow_mutations) {
            std::cerr << "File Manager: --quarantine requires --allow-mutations\n";
            return 2;
        }

        Runtime runtime{};
        runtime.application = std::make_shared<file_manager::Application>(
            root, quarantine, allow_mutations, std::move(engine_root_id));
        file_manager::DocumentPickerRequest picker_request{};
        picker_request.profile = file_manager::DocumentPickerProfile::open_file;
        picker_request.protected_root = root;
        picker_request.initial_location = root;
        picker_request.owner_application_id = "file-manager";
        picker_request.authority = file_manager::DocumentPickerAuthority::trusted_local_host;
        picker_request.home_location = file_manager::user_home_directory();
        picker_request.maximum_selection = 1;
        runtime.picker = std::make_shared<file_manager::DocumentPickerView>(std::move(picker_request));
        runtime.about = std::make_shared<file_manager::AboutView>();
        (*runtime.application).bind_secondary_surfaces(
            std::bind_front(&Runtime::present_picker, &runtime),
            std::bind_front(&Runtime::present_about, &runtime));
        (*runtime.about).bind_hide(std::bind_front(&Runtime::dismiss_about, &runtime));
        const gui_forms::SubscriptionToken picker_completed =
            (*runtime.picker).completed().subscribe(
                std::bind_front(&Runtime::picker_completed, &runtime));

#if defined(__APPLE__)
        using NativeWindow = gui_forms::host::MacApplicationWindow;
#else
        using NativeWindow = gui_forms::ApplicationWindow;
#endif
        NativeWindow primary{};
        primary.stable_id = "file-manager.window";
        primary.model = (*runtime.application).make_window();
        runtime.primary_window = primary.model.get();
        primary.options.title = "File Manager";
        primary.options.initial_size = {1340, 850};
        primary.options.minimum_size = {150, 150};
        primary.options.print_metrics_on_close = true;
        primary.options.dispatch_pending = std::bind_front(&Runtime::drain, &runtime);
        primary.options.closed = std::bind_front(&Runtime::stop, &runtime);
#if defined(__APPLE__)
        primary.options.titlebar_presentation = gui_forms::host::MacTitlebarPresentation::transparent_full_size_content;
        for (const std::string_view id : web_forms_generated_file_manager_sapphire::NativeForm::window_drag_region_ids) {
            primary.options.window_drag_region_ids.emplace_back(id);
        }
        primary.options.host_ready = std::bind_front(&Runtime::mac_ready, &runtime);
#else
        primary.options.wake_ready = std::bind_front(&Runtime::accept_wake, &runtime);
        primary.options.ready = std::bind_front(&Runtime::primary_ready, &runtime);
#endif
        NativeWindow picker_window{};
        picker_window.stable_id = "file-manager.open-picker";
        picker_window.owner_id = primary.stable_id;
        picker_window.model = std::make_unique<gui_forms::Window>(
            (*runtime.picker).root_control(), gui_forms::Size{760, 560});
        (*runtime.picker).attach_dialog(*picker_window.model);
        picker_window.options.title = "Open — File Manager";
        picker_window.options.initial_size = {760, 560};
        picker_window.options.minimum_size = {620, 460};
        picker_window.options.initially_visible = false;
        picker_window.options.hide_on_close = true;
        picker_window.options.minimizable = false;
        picker_window.options.print_metrics_on_close = false;
#if defined(__APPLE__)
        picker_window.options.close_request = std::bind_front(&Runtime::picker_closing, &runtime);
        picker_window.options.titlebar_presentation = gui_forms::host::MacTitlebarPresentation::transparent_full_size_content;
        picker_window.options.window_drag_region_id = "file-manager.picker.title";
        picker_window.options.visibility_ready = std::bind_front(&Runtime::picker_visibility, &runtime);
#else
        picker_window.options.closing = std::bind_front(&Runtime::picker_closing, &runtime);
        picker_window.options.ready = std::bind_front(&Runtime::picker_ready, &runtime);
#endif
        NativeWindow about_window{};
        about_window.stable_id = "file-manager.about-window";
        about_window.owner_id = primary.stable_id;
        about_window.model = std::make_unique<gui_forms::Window>(
            (*runtime.about).root_control(), gui_forms::Size{540, 330});
        about_window.options.title = "About File Manager";
        about_window.options.initial_size = {540, 330};
        about_window.options.minimum_size = {500, 300};
        about_window.options.initially_visible = false;
        about_window.options.hide_on_close = true;
        about_window.options.minimizable = false;
        about_window.options.print_metrics_on_close = false;
#if defined(__APPLE__)
        about_window.options.titlebar_presentation = gui_forms::host::MacTitlebarPresentation::transparent_full_size_content;
        about_window.options.window_drag_region_id = "file-manager.about.title";
        about_window.options.visibility_ready = std::bind_front(&Runtime::about_visibility, &runtime);
#else
        about_window.options.ready = std::bind_front(&Runtime::about_ready, &runtime);
#endif
        std::vector<NativeWindow> windows{};
        windows.reserve(3);
        windows.push_back(std::move(primary));
        windows.push_back(std::move(picker_window));
        windows.push_back(std::move(about_window));
        static_cast<void>(picker_completed);
#if defined(__APPLE__)
        const int result = gui_forms::host::run_macos_application(std::move(windows));
        return result;
#else
        const gui_forms::ApplicationResult result = gui_forms::Application::run(std::move(windows));
        (*runtime.application).stop();
        if (result.callback_exception) std::rethrow_exception(result.callback_exception);
        if (!result.accepted()) {
            std::cerr << "File Manager: native host failed (" << static_cast<int>(result.error) << ")\n";
            return 1;
        }
        return result.native_exit_code;
#endif
    } catch (const std::exception& error) {
        std::cerr << "File Manager: " << error.what() << '\n';
        return 1;
    }
}
