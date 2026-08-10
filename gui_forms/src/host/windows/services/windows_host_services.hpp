#pragma once

// Included inside the Win32 adapter's private namespace after its conversion,
// dialog, cursor, and monitor helpers have been declared. Keeping this state
// owner separate makes service policy review independent from message routing.
class WindowsHostServices final : public HostServices {
public:
    WindowsHostServices() : HostServices(windows_capabilities()) {}

    void bind_owner(HWND owner) noexcept { owner_ = owner; }

protected:
    HostMonitorResult query_monitors_impl() override {
        HostMonitorResult result;
        const BOOL enumerated = EnumDisplayMonitors(
            nullptr, nullptr, &WindowsHostServices::collect_monitor,
            reinterpret_cast<LPARAM>(&result.monitors));
        if (!enumerated || result.monitors.empty()) {
            result.status.error = HostServiceError::backend_failure;
        }
        return result;
    }

    HostServiceStatus set_cursor_impl(CursorKind cursor) override {
        return apply_system_cursor(cursor)
            ? HostServiceStatus{}
            : HostServiceStatus{HostServiceError::backend_failure};
    }

    HostServiceStatus set_pointer_capture_impl(bool captured,
                                               std::uint64_t) override {
        if (owner_ == nullptr) return {HostServiceError::backend_failure};
        if (captured) {
            SetCapture(owner_);
            return GetCapture() == owner_ ? HostServiceStatus{}
                                          : HostServiceStatus{HostServiceError::backend_failure};
        }
        if (GetCapture() == owner_ && !ReleaseCapture()) {
            return {HostServiceError::backend_failure};
        }
        return {};
    }

    HostClipboardTextResult read_clipboard_text_impl() override {
        HostClipboardTextResult result;
        result.generation = static_cast<std::uint64_t>(GetClipboardSequenceNumber());
        if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return result;
        if (!OpenClipboard(owner_)) {
            result.status.error = HostServiceError::backend_failure;
            return result;
        }
        HANDLE data = GetClipboardData(CF_UNICODETEXT);
        const wchar_t* locked = data == nullptr
            ? nullptr : static_cast<const wchar_t*>(GlobalLock(data));
        if (locked == nullptr) {
            CloseClipboard();
            result.status.error = HostServiceError::backend_failure;
            return result;
        }
        result.text_utf8 = utf8_from_wide(locked);
        result.has_text = true;
        GlobalUnlock(data);
        CloseClipboard();
        return result;
    }

    HostServiceStatus write_clipboard_text_impl(std::string_view text) override {
        const std::wstring wide = wide_from_utf8(text);
        if (!text.empty() && wide.empty()) return {HostServiceError::invalid_utf8};
        if (!OpenClipboard(owner_)) return {HostServiceError::backend_failure};
        if (!EmptyClipboard()) {
            CloseClipboard();
            return {HostServiceError::backend_failure};
        }
        const std::size_t bytes = (wide.size() + 1U) * sizeof(wchar_t);
        HGLOBAL storage = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (storage == nullptr) {
            CloseClipboard();
            return {HostServiceError::backend_failure};
        }
        void* destination = GlobalLock(storage);
        if (destination == nullptr) {
            GlobalFree(storage);
            CloseClipboard();
            return {HostServiceError::backend_failure};
        }
        std::memcpy(destination, wide.c_str(), bytes);
        GlobalUnlock(storage);
        if (SetClipboardData(CF_UNICODETEXT, storage) == nullptr) {
            GlobalFree(storage);
            CloseClipboard();
            return {HostServiceError::backend_failure};
        }
        CloseClipboard();
        return {};
    }

    HostDialogResult show_dialog_impl(const HostDialogRequest& request) override {
        if (std::holds_alternative<HostMessageDialogRequest>(request.payload)) {
            return native_message_dialog(
                owner_, request.request_id,
                std::get<HostMessageDialogRequest>(request.payload));
        }
        if (std::holds_alternative<HostOpenFileDialogRequest>(request.payload)) {
            return native_open_dialog(
                owner_, request.request_id,
                std::get<HostOpenFileDialogRequest>(request.payload));
        }
        if (std::holds_alternative<HostSaveFileDialogRequest>(request.payload)) {
            return native_save_dialog(
                owner_, request.request_id,
                std::get<HostSaveFileDialogRequest>(request.payload));
        }
        if (std::holds_alternative<HostFolderDialogRequest>(request.payload)) {
            return native_folder_dialog(
                owner_, request.request_id,
                std::get<HostFolderDialogRequest>(request.payload));
        }
        return native_color_dialog(
            owner_, request.request_id,
            std::get<HostColorDialogRequest>(request.payload), custom_colors_);
    }

    HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) override {
        UINT type = MB_OK;
        switch (request.cue) {
        case HostSoundCue::notification: type = MB_ICONASTERISK; break;
        case HostSoundCue::success: type = MB_OK; break;
        case HostSoundCue::warning: type = MB_ICONEXCLAMATION; break;
        case HostSoundCue::error: type = MB_ICONHAND; break;
        case HostSoundCue::operation_complete: type = MB_OK; break;
        }
        return MessageBeep(type) ? HostServiceStatus{}
                                 : HostServiceStatus{HostServiceError::backend_failure};
    }

    void shutdown_impl() noexcept override { owner_ = nullptr; }

private:
    static BOOL CALLBACK collect_monitor(HMONITOR monitor, HDC, LPRECT,
                                         LPARAM context) {
        std::vector<HostMonitor>& monitors =
            *reinterpret_cast<std::vector<HostMonitor>*>(context);
        MONITORINFOEXW info{};
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoW(monitor, &info)) return TRUE;
        HostMonitor value;
        value.id = utf8_from_wide(info.szDevice);
        if (value.id.empty()) {
            value.id = "win32.monitor." + std::to_string(
                reinterpret_cast<std::uintptr_t>(monitor));
        }
        value.frame = {
            static_cast<double>(info.rcMonitor.left),
            static_cast<double>(info.rcMonitor.top),
            static_cast<double>(info.rcMonitor.right - info.rcMonitor.left),
            static_cast<double>(info.rcMonitor.bottom - info.rcMonitor.top)};
        value.work_area = {
            static_cast<double>(info.rcWork.left),
            static_cast<double>(info.rcWork.top),
            static_cast<double>(info.rcWork.right - info.rcWork.left),
            static_cast<double>(info.rcWork.bottom - info.rcWork.top)};
        value.scale = native_monitor_scale(monitor);
        value.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0U;
        monitors.push_back(std::move(value));
        return TRUE;
    }

    HWND owner_{};
    std::array<COLORREF, 16> custom_colors_{};
};
