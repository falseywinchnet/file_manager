#include "application.hpp"
#include "file_manager/platform_paths.hpp"
#include "gui_forms/gui_forms.hpp"
#include "gui_forms/application.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace file_manager {
class ApplicationLatencyProbe final {
public:
    static std::uint64_t navigate(Application& application, const std::filesystem::path& path) {
        application.request_navigation(path, true);
        return application.requested_generation_.load();
    }
    static std::uint64_t applied(const Application& application) {
        return application.applied_generation_;
    }
    static void search(Application& application, const std::string& query) {
        (*application.search_box_).set_text(query);
        application.apply_filter();
    }
    static bool searching(const Application& application) { return application.search_loading_; }
    static bool found(const Application& application, const std::string& filename) {
        if (!application.search_showing_) return false;
        for (const std::string& id : application.search_order_) {
            const std::unordered_map<std::string, DirectoryEntry>::const_iterator entry = application.entries_.find(id);
            if (entry != application.entries_.end() && (*entry).second.name == filename &&
                (*entry).second.identity.available()) return true;
        }
        return false;
    }
};
}

namespace {
using Clock = std::chrono::steady_clock;
double milliseconds(const Clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}
void noop() {}
void print_process_memory() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX memory{};
    memory.cb = sizeof(memory);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory), sizeof(memory))) {
        std::cout << ",private_bytes=" << memory.PrivateUsage
                  << ",working_set_bytes=" << memory.WorkingSetSize
                  << ",peak_working_set_bytes=" << memory.PeakWorkingSetSize;
    }
#endif
}
class StopGuard final {
public:
    explicit StopGuard(file_manager::Application& application) : application_(application) {}
    ~StopGuard() { application_.stop(); }
private:
    file_manager::Application& application_;
};

void await_generation(file_manager::Application& application, gui_forms::Window& window,
                      const std::uint64_t generation, const Clock::time_point start,
                      const std::string& label) {
    double drain_ms{};
    double max_drain_ms{};
    std::size_t drains{};
    while (file_manager::ApplicationLatencyProbe::applied(application) < generation) {
        const Clock::time_point before = Clock::now();
        application.drain_ui();
        const double elapsed = milliseconds(Clock::now() - before);
        drain_ms += elapsed;
        max_drain_ms = std::max(max_drain_ms, elapsed);
        ++drains;
        if (Clock::now() - start > std::chrono::seconds(30)) throw std::runtime_error("navigation timeout");
        if (file_manager::ApplicationLatencyProbe::applied(application) < generation)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const double total_ms = milliseconds(Clock::now() - start);
    const Clock::time_point layout_start = Clock::now();
    window.perform_layout();
    const double layout_ms = milliseconds(Clock::now() - layout_start);
    const gui_forms::MetricsSnapshot metrics = window.metrics_snapshot();
    std::cout << label << ",total_ms=" << total_ms << ",drain_ms=" << drain_ms
              << ",max_drain_ms=" << max_drain_ms << ",drains=" << drains
              << ",final_layout_ms=" << layout_ms << ",flushes=" << metrics.flush_count
              << ",measure_passes=" << metrics.measure_passes << ",arrange_passes=" << metrics.arrange_passes << '\n';
}

void run_case(const std::filesystem::path& root, const std::filesystem::path& destination,
              const std::string& label) {
    const Clock::time_point constructor_start = Clock::now();
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(root, std::nullopt, false, std::string{});
    StopGuard guard(*application);
    const double constructor_ms = milliseconds(Clock::now() - constructor_start);
    const Clock::time_point window_start = Clock::now();
    const std::unique_ptr<gui_forms::Window> window = application->make_window();
    const double window_ms = milliseconds(Clock::now() - window_start);
    std::cout << label << ",constructor_ms=" << constructor_ms << ",make_window_ms=" << window_ms
              << ",make_window_flushes=" << window->metrics_snapshot().flush_count << '\n';
    window->reset_activity_metrics();
    const Clock::time_point initial_start = Clock::now();
    application->bind_host(noop, noop);
    await_generation(*application, *window, 1, initial_start, label + ".initial");
    for (int iteration = 0; iteration < 5; ++iteration) {
        const std::filesystem::path target = iteration % 2 == 0 ? destination : root;
        window->reset_activity_metrics();
        const Clock::time_point start = Clock::now();
        const std::uint64_t generation = file_manager::ApplicationLatencyProbe::navigate(*application, target);
        await_generation(*application, *window, generation, start, label + ".navigate" + std::to_string(iteration));
    }
}

class NativeBenchmark final {
public:
    explicit NativeBenchmark(const std::filesystem::path& root, const bool partial)
        : started_(Clock::now()), application_(std::make_shared<file_manager::Application>(
              root, std::nullopt, false, std::string{})), partial_(partial) {}
    ~NativeBenchmark() { application_->stop(); }
    void wake_ready(std::function<void()> wake) { wake_ = std::move(wake); }
    void close() { static_cast<void>(handle_.request_close()); }
    void closed() { application_->stop(); }
    void drain() { application_->drain_ui(); }
    void ready(gui_forms::Window& window, gui_forms::ApplicationWindowHandle handle) {
        window_ = &window;
        handle_ = handle;
        repaint_control_ = partial_
            ? window.find("file-manager-app.shell.location.navigation.back") : window.root();
        if (!repaint_control_) throw std::runtime_error("native benchmark repaint control missing");
        std::cout << "native,ready_ms=" << milliseconds(Clock::now() - started_) << std::endl;
        application_->bind_host(wake_, std::bind_front(&NativeBenchmark::close, this));
        timer_ = window.schedule_ui_timer(*window.root(), std::chrono::milliseconds(10),
            Clock::now() + std::chrono::milliseconds(10), std::bind_front(&NativeBenchmark::tick, this));
    }
    void tick(gui_forms::FrameTime now) {
        if (now - started_ > std::chrono::seconds(180)) {
            timed_out_ = true;
            std::cout << "native,timeout=true" << std::endl;
            timer_.disconnect();
            close();
            return;
        }
        const gui_forms::MetricsSnapshot metrics = window_->metrics_snapshot();
        if (file_manager::ApplicationLatencyProbe::applied(*application_) == 0 ||
            metrics.frames_presented <= prior_frames_) return;
        std::cout << "native,sample=" << samples_ << ",elapsed_ms=" << milliseconds(now - started_)
                  << ",frames=" << metrics.frames_presented
                  << ",present_total_ms=" << static_cast<double>(metrics.present_duration_nanoseconds) / 1000000.0
                  << ",present_worst_ms=" << static_cast<double>(metrics.worst_present_duration_nanoseconds) / 1000000.0
                  << ",flushes=" << metrics.flush_count << ",measure_passes=" << metrics.measure_passes
                  << ",paint_passes=" << metrics.paint_passes
                  << ",full_window_paints=" << metrics.full_window_paints
                  << ",partial_paints=" << metrics.partial_paints
                  << ",painted_damage_area=" << metrics.painted_damage_area;
        print_process_memory();
        std::cout << std::endl;
        ++samples_;
        prior_frames_ = metrics.frames_presented;
        if (samples_ >= 5) {
            timer_.disconnect();
            close();
        } else {
            repaint_control_->invalidate(gui_forms::Dirty::paint);
        }
    }
    int run() {
        gui_forms::ApplicationWindowOptions options;
        options.title = "File Manager — repaint benchmark (self-closing)";
        options.initial_size = {1340, 850};
        options.print_metrics_on_close = true;
        options.wake_ready = std::bind_front(&NativeBenchmark::wake_ready, this);
        options.ready = std::bind_front(&NativeBenchmark::ready, this);
        options.dispatch_pending = std::bind_front(&NativeBenchmark::drain, this);
        options.closed = std::bind_front(&NativeBenchmark::closed, this);
        const gui_forms::ApplicationResult result = gui_forms::Application::run(
            application_->make_window(), std::move(options));
        if (result.callback_exception) std::rethrow_exception(result.callback_exception);
        return result.accepted() && !timed_out_ && samples_ == 5 ? 0 : 1;
    }
private:
    Clock::time_point started_;
    std::shared_ptr<file_manager::Application> application_;
    gui_forms::Window* window_{};
    gui_forms::ApplicationWindowHandle handle_;
    gui_forms::FrameRequestToken timer_;
    std::function<void()> wake_;
    std::uint64_t prior_frames_{};
    unsigned int samples_{};
    bool timed_out_{};
    bool partial_{};
    gui_forms::Control::Ptr repaint_control_;
};

void run_live_search(const std::filesystem::path& root, const std::string& root_id,
                     const std::string& query, const std::string& expected_filename) {
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(root, std::nullopt, false, root_id);
    StopGuard stop(*application);
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(noop, noop);
    await_generation(*application, *window, 1, Clock::now(), "live_search.initial");
    const Clock::time_point started = Clock::now();
    file_manager::ApplicationLatencyProbe::search(*application, query);
    do {
        (*application).drain_ui();
        if (Clock::now() - started > std::chrono::seconds(30)) {
            throw std::runtime_error("live frontend search timed out");
        }
        if (file_manager::ApplicationLatencyProbe::searching(*application)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    } while (file_manager::ApplicationLatencyProbe::searching(*application));
    if (!file_manager::ApplicationLatencyProbe::found(*application, expected_filename)) {
        throw std::runtime_error("live frontend search did not return the expected identity-checked file");
    }
    (*window).perform_layout();
    std::cout << "live_search,expected_file_present=true,elapsed_ms="
              << milliseconds(Clock::now() - started) << '\n';
}
}

int main(int argc, char** argv) {
    try {
        if (argc == 6 && std::string_view(argv[2]) == "--live-search") {
            run_live_search(file_manager::path_from_utf8(argv[1]), argv[3], argv[4], argv[5]);
            return 0;
        }
        if (argc != 2 && argc != 3) throw std::runtime_error(
            "usage: application_latency_benchmark REPOSITORY [--native|--native-partial]\n"
            "or: application_latency_benchmark ROOT --live-search ROOT_ID QUERY EXPECTED_FILENAME");
        std::cout << std::fixed << std::setprecision(3);
        if (argc == 3) {
            const bool partial = std::string_view(argv[2]) == "--native-partial";
            if (!partial && std::string_view(argv[2]) != "--native") throw std::runtime_error("unknown benchmark mode");
            NativeBenchmark native(file_manager::path_from_utf8(argv[1]), partial);
            return native.run();
        }
        run_case(file_manager::user_home_directory(), file_manager::path_from_utf8(argv[1]), "real_home_repo");
        // Only this uniquely created disposable directory is written or removed.
        const std::filesystem::path fixture = std::filesystem::temp_directory_path() /
            ("fm-latency-" + std::to_string(Clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(fixture / "large");
        for (int index = 0; index < 2000; ++index) {
            std::ofstream(fixture / "large" / ("item-" + std::to_string(index) + ".txt"), std::ios::binary) << "benchmark\n";
        }
        try { run_case(fixture, fixture / "large", "synthetic_2000"); }
        catch (...) { std::filesystem::remove_all(fixture); throw; }
        std::filesystem::remove_all(fixture);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
