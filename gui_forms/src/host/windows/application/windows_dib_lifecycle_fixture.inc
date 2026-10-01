// Included only in the dedicated test translation unit. No installed header or
// production host exports these controls or private inspection operations.
class DibFixtureControl final : public Control {
public:
    DibFixtureControl() : Control(StableId("dib-fixture")) {}
    Color color{0x22, 0x44, 0x66, 0xff};
    bool fail{};
    bool revoke_epoch{};
    std::shared_ptr<LiveSurface> surface{};
protected:
    void on_paint(Painter& painter, Rect damage) override {
        painter.fill_rect(damage, color);
        if (surface) painter.draw_live_surface(surface, client_rectangle(), 1.0);
        if (fail) throw std::runtime_error("fixture callback failure");
        if (revoke_epoch) {
            revoke_epoch = false;
            Window& model = *attached_window();
            const double next_scale = model.scale() + 0.25;
            model.set_scale(next_scale);
        }
    }
};

struct DibHostFixture final {
    static void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }
    static void request_paint(WindowsHostState& state) {
        InvalidateRect(state.hwnd_, nullptr, FALSE);
        SendMessageW(state.hwnd_, WM_PAINT, 0, 0);
    }
    static void require_front(WindowsHostState& state, detail::DibFrameKey key,
                              const std::vector<std::uint32_t>& pixels) {
        require(detail::same_frame_key(state.raster_.frames_.front_key(), key), "host front identity changed");
        const detail::DibFrontView front = state.raster_.frames_.front();
        require(front.pixels.size() == pixels.size(), "host front extent changed");
        require(std::equal(pixels.begin(), pixels.end(), front.pixels.begin()), "host front pixels changed");
    }
    static void exercise(WindowsHostState& state, DibFixtureControl& control) {
        require(state.raster_.memory_dc_ == state.raster_.metrics_dc_ &&
                state.raster_.metrics_dc_ != nullptr, "metrics context unavailable before first paint");
        request_paint(state);
        const PaintLeaseSnapshot initial = (*state.model_).paint_lease_snapshot();
        require(initial.presented_revision != 0, "initial Window receipt not accepted");
        require(state.raster_.matches_front(initial), "initial front/model mismatch");
        const detail::DibFrameKey first = state.raster_.frames_.front_key();
        const detail::DibFrontView view = state.raster_.frames_.front();
        const std::vector<std::uint32_t> old_pixels(view.pixels.begin(), view.pixels.end());

        control.color = {0xaa, 0xbb, 0xcc, 0xff};
        control.invalidate(Dirty::paint);
        state.collect_damage();
        state.raster_.frames_.inject_failure(detail::DibFrameFault::commit_flush);
        request_paint(state);
        require_front(state, first, old_pixels);
        const PaintLeaseSnapshot failed = (*state.model_).paint_lease_snapshot();
        require(failed.presented_revision == initial.presented_revision, "failed frame acknowledged");
        require(!state.pending_damage_.empty(), "failed damage lost");
        request_paint(state);
        require_front(state, first, old_pixels);
        require((*state.model_).paint_lease_snapshot().leases_started == failed.leases_started,
                "recovery exposure unexpectedly repainted");
        request_paint(state);
        require(state.raster_.matches_front((*state.model_).paint_lease_snapshot()), "recovery commit mismatch");

        control.fail = true;
        control.invalidate(Dirty::paint);
        state.collect_damage();
        const detail::DibFrameKey before_throw = state.raster_.frames_.front_key();
        request_paint(state);
        require(detail::same_frame_key(state.raster_.frames_.front_key(), before_throw), "callback failure committed");
        require(state.native_callback_faults_ == 1, "callback failure not contained");
        control.fail = false;
        request_paint(state);
        request_paint(state);

        control.revoke_epoch = true;
        control.invalidate(Dirty::paint);
        state.collect_damage();
        const detail::DibFrameKey before_revoke = state.raster_.frames_.front_key();
        request_paint(state);
        require(detail::same_frame_key(state.raster_.frames_.front_key(), before_revoke), "revoked epoch committed");
        require((*state.model_).paint_lease_snapshot().surface_epoch != before_revoke.epoch,
                "fixture did not revoke the model epoch");
        (*state.model_).set_scale(state.scale_);
        request_paint(state);
        request_paint(state);

        std::shared_ptr<LiveSurface> surface = LiveSurface::create({.width = 4U, .height = 4U,
            .buffer_count = 2U});
        require(static_cast<bool>(surface), "live surface allocation");
        LiveSurfaceWriteLease write = (*surface).try_acquire_write();
        require(static_cast<bool>(write), "live write lease");
        const std::span<std::byte> bytes = write.pixels();
        std::fill(bytes.begin(), bytes.end(), std::byte{0x55});
        const std::uint64_t published_generation = write.publish();
        require(published_generation != 0, "live publish");
        control.surface = surface;
        control.invalidate(Dirty::paint);
        state.collect_damage();
        request_paint(state);
        const Control::Ptr root = (*state.model_).root();
        const bool live_registered = (*state.model_).queue_live_surface_presentation(root, surface);
        require(live_registered, "live registration");
        const detail::DibFrameKey live_key = state.raster_.frames_.front_key();
        state.present_live_surface_updates();
        require(state.live_updates_presented_ != 0, "live host presentation missing");
        require(detail::same_frame_key(state.raster_.frames_.front_key(), live_key), "live update changed receipt");
        const detail::DibFrontView live_front = state.raster_.frames_.front();
        require((live_front.pixels[0] & 0x00ffffffU) == 0x00555555U, "live glyph-free raster pixels missing");
        const std::vector<std::uint32_t> live_pixels(live_front.pixels.begin(), live_front.pixels.end());
        // With two buffers, retaining the old native read lease prevents the
        // second producer rotation from reacquiring that old buffer.
        for (int rotation = 0; rotation < 2; ++rotation) {
            LiveSurfaceWriteLease recycled = (*surface).try_acquire_write();
            require(static_cast<bool>(recycled), "live native read lease retained");
            const std::span<std::byte> recycled_bytes = recycled.pixels();
            std::fill(recycled_bytes.begin(), recycled_bytes.end(), std::byte{0x77});
            const std::uint64_t recycled_generation = recycled.publish();
            require(recycled_generation != 0, "recycled live publish");
        }
        state.raster_.frames_.inject_failure(detail::DibFrameFault::present);
        state.present_live_surface_updates();
        require_front(state, live_key, live_pixels);
        require(state.live_updates_failed_ != 0, "failed live presentation not counted");
        request_paint(state);
        require_front(state, live_key, live_pixels);
        request_paint(state);
        require(state.pending_damage_.empty(), "same-revision recovery left permanent damage");
        const detail::DibFrontView recovered_live = state.raster_.frames_.front();
        require((recovered_live.pixels[0] & 0x00ffffffU) == 0x00777777U,
                "normal recovery lost the newest live generation");

        control.invalidate(Dirty::paint);
        state.collect_damage();
        const std::uint64_t sampled = state.live_updates_sampled_;
        state.present_live_surface_updates();
        require(state.live_updates_sampled_ == sampled, "stale-front live placement sampled");
        request_paint(state);

        const detail::DibFrameKey before_resize = state.raster_.frames_.front_key();
        const detail::DibFrontView before_extent = state.raster_.frames_.front();
        const int old_width = before_extent.width;
        const int old_height = before_extent.height;
        SetWindowPos(state.hwnd_, nullptr, 0, 0, 120, 100,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        state.resize();
        state.raster_.frames_.inject_failure(detail::DibFrameFault::allocation);
        request_paint(state);
        require(detail::same_frame_key(state.raster_.frames_.front_key(), before_resize), "failed resize relabelled front");
        const detail::DibFrontView failed_resize = state.raster_.frames_.front();
        require(failed_resize.width == old_width && failed_resize.height == old_height, "failed resize changed old extent");
        request_paint(state);
        request_paint(state);
        require(state.raster_.matches_front((*state.model_).paint_lease_snapshot()), "resized receipt mismatch");
        const detail::DibFrameKey before_empty = state.raster_.frames_.front_key();
        const bool empty_size_accepted = state.raster_.resize({}, state.scale_);
        require(empty_size_accepted, "zero size accepted");
        const bool empty_frame_begun = state.raster_.begin_frame();
        require(!empty_frame_begun, "zero size allocated candidate");
        require(detail::same_frame_key(state.raster_.frames_.front_key(), before_empty), "zero size relabelled front");
        state.resize();
        SendMessageW(state.hwnd_, WM_CLOSE, 0, 0);
        require(state.hwnd_ == nullptr, "owned native window did not close");
    }
    static void run() {
        const HRESULT com_status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        require(SUCCEEDED(com_status), "fixture COM setup");
        try {
            WNDCLASSEXW native_class{};
            native_class.cbSize = sizeof(native_class);
            native_class.lpfnWndProc = window_procedure;
            native_class.hInstance = GetModuleHandleW(nullptr);
            native_class.lpszClassName = L"GUIForms.DibFixture.v1";
            const ATOM registered_class = RegisterClassExW(&native_class);
            require(registered_class != 0, "fixture class registration");
            {
                std::shared_ptr<DibFixtureControl> control = std::make_shared<DibFixtureControl>();
                std::unique_ptr<Window> model = std::make_unique<Window>(control, Size{80, 60});
                WindowsHostOptions options{};
                options.quit_thread_on_close = false;
                WindowsHostState state(std::move(model), options);
                HWND window = CreateWindowExW(WS_EX_NOACTIVATE, native_class.lpszClassName,
                    L"DIB fixture", WS_POPUP, 0, 0, 80, 60, nullptr, nullptr,
                    native_class.hInstance, &state);
                require(window != nullptr, "fixture native creation");
                const bool initialized = state.initialize(window);
                require(initialized, "fixture host initialization");
                exercise(state, *control);
            }
            const BOOL class_released = UnregisterClassW(native_class.lpszClassName, native_class.hInstance);
            require(class_released != FALSE,
                    "fixture class release");
        } catch (...) {
            CoUninitialize();
            throw;
        }
        CoUninitialize();
    }
};
