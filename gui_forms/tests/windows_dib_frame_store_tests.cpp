#include "dib_frame_store.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

namespace {
using gui_forms::host::detail::DibFrameFault;
using gui_forms::host::detail::DibFrameKey;
using gui_forms::host::detail::DibFrameStatus;
using gui_forms::host::detail::DibFrameStore;
using gui_forms::host::detail::DibFrontView;
using gui_forms::host::detail::DibSurfaceView;
using gui_forms::host::detail::same_frame_key;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void require_pixels(std::span<const std::uint32_t> pixels, std::uint32_t expected) {
    require(!pixels.empty(), "expected nonempty pixels");
    for (std::size_t index = 0; index < pixels.size(); ++index) {
        require(pixels[index] == expected, "unexpected pixel");
    }
}

void fill_candidate(DibFrameStore& store, std::uint32_t value) {
    const DibSurfaceView view = store.candidate();
    require(!view.pixels.empty(), "candidate must be available");
    std::fill(view.pixels.begin(), view.pixels.end(), value);
}

void require_front(const DibFrameStore& store, DibFrameKey key, int width, int height,
                   std::uint32_t value) {
    require(same_frame_key(store.front_key(), key), "committed identity changed");
    const DibFrontView front = store.front();
    require(front.width == width && front.height == height, "committed dimensions changed");
    require_pixels(front.pixels, value);
}

void test_failures_and_exposure() {
    DibFrameStore destination{};
    const DibFrameStatus destination_status = destination.begin(8, 8);
    require(destination_status == DibFrameStatus::success, "destination allocation");
    const DibSurfaceView output = destination.candidate();
    DibFrameStore store{};
    const DibFrameKey first{.revision = 1, .epoch = 7, .scale = 1.0};
    const DibFrameKey next{.revision = 2, .epoch = 7, .scale = 1.0};
    const RECT damage{0, 0, 4, 3};
    const DibFrameStatus initial_begin = store.begin(4, 3);
    require(initial_begin == DibFrameStatus::success, "initial begin");
    fill_candidate(store, 0x00112233U);
    const DibFrameStatus initial_commit = store.commit(output.dc, damage, first);
    require(initial_commit == DibFrameStatus::success, "initial commit");
    require_front(store, first, 4, 3, 0x00112233U);

    store.inject_failure(DibFrameFault::allocation);
    const DibFrameStatus allocation_refusal = store.begin(5, 4);
    require(allocation_refusal == DibFrameStatus::allocation_failed, "allocation refusal");
    require_front(store, first, 4, 3, 0x00112233U);
    store.inject_failure(DibFrameFault::begin_flush);
    const DibFrameStatus begin_flush_refusal = store.begin(4, 3);
    require(begin_flush_refusal == DibFrameStatus::flush_failed, "begin flush refusal");
    require_front(store, first, 4, 3, 0x00112233U);

    const std::array<DibFrameFault, 2> faults{DibFrameFault::commit_flush, DibFrameFault::present};
    const std::array<DibFrameStatus, 2> statuses{DibFrameStatus::flush_failed, DibFrameStatus::present_failed};
    for (std::size_t index = 0; index < faults.size(); ++index) {
        const DibFrameStatus failure_begin = store.begin(4, 3);
        require(failure_begin == DibFrameStatus::success, "failure candidate begin");
        fill_candidate(store, 0x00abcdefU);
        store.inject_failure(faults[index]);
        const DibFrameStatus failed_commit = store.commit(output.dc, damage, next);
        require(failed_commit == statuses[index], "injected commit refusal");
        require(store.candidate().pixels.empty(), "failed candidate borrow revoked");
        require_front(store, first, 4, 3, 0x00112233U);
        std::fill(output.pixels.begin(), output.pixels.end(), 0U);
        const DibFrameStatus exposure_status = store.expose(output.dc, damage);
        require(exposure_status == DibFrameStatus::success, "old front exposure");
        for (std::size_t row = 0; row < 8; ++row) {
            for (std::size_t column = 0; column < 8; ++column) {
                const std::uint32_t expected = row < 3 && column < 4 ? 0x00112233U : 0U;
                require(output.pixels[row * 8 + column] == expected, "exposure uses old front only");
            }
        }
    }
    const DibFrameStatus invalid_target_begin = store.begin(4, 3);
    require(invalid_target_begin == DibFrameStatus::success, "real invalid target begin");
    fill_candidate(store, 0x00ffffffU);
    const DibFrameStatus invalid_target_commit = store.commit(nullptr, damage, next);
    require(invalid_target_commit == DibFrameStatus::present_failed, "null target refusal");
    require_front(store, first, 4, 3, 0x00112233U);

    const DibFrameStatus recovery_begin = store.begin(4, 3);
    require(recovery_begin == DibFrameStatus::success, "recovery begin");
    const DibSurfaceView replacement = store.candidate();
    require_pixels(replacement.pixels, 0x00112233U);
    replacement.pixels[5] = 0x000000ffU;
    const RECT patch{1, 1, 2, 2};
    const DibFrameStatus patch_commit = store.commit(output.dc, patch, first);
    require(patch_commit == DibFrameStatus::success, "same identity live patch");
    const DibFrontView patched = store.front();
    for (std::size_t index = 0; index < patched.pixels.size(); ++index) {
        const std::uint32_t expected = index == 5 ? 0x000000ffU : 0x00112233U;
        require(patched.pixels[index] == expected, "partial candidate preserves undamaged pixels");
    }
    require(same_frame_key(store.front_key(), first), "live patch retains identity");
}

void test_resize_and_validation() {
    DibFrameStore destination{};
    const DibFrameStatus destination_status = destination.begin(8, 8);
    require(destination_status == DibFrameStatus::success, "resize destination");
    const DibSurfaceView output = destination.candidate();
    DibFrameStore store{};
    const DibFrameKey first{.revision = 3, .epoch = 9, .scale = 1.0};
    const DibFrameKey resized{.revision = 4, .epoch = 10, .scale = 1.5};
    const RECT damage{0, 0, 8, 8};
    const DibFrameStatus initial_begin = store.begin(2, 2);
    require(initial_begin == DibFrameStatus::success, "resize initial begin");
    fill_candidate(store, 0x00665544U);
    const DibFrameStatus initial_commit = store.commit(output.dc, damage, first);
    require(initial_commit == DibFrameStatus::success, "resize initial commit");
    const DibFrameStatus resize_begin = store.begin(4, 3);
    require(resize_begin == DibFrameStatus::success, "resize candidate");
    require_front(store, first, 2, 2, 0x00665544U);
    const DibSurfaceView candidate = store.candidate();
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            const std::uint32_t expected = row < 2 && column < 2 ? 0x00665544U : 0U;
            require(candidate.pixels[row * 4 + column] == expected, "resize copied overlap and cleared extension");
        }
    }
    const DibFrameStatus busy_begin = store.begin(1, 1);
    require(busy_begin == DibFrameStatus::busy, "active candidate excludes replacement");
    store.abort();
    require_front(store, first, 2, 2, 0x00665544U);
    const DibFrameStatus empty_begin = store.begin(0, 0);
    require(empty_begin == DibFrameStatus::empty, "minimized surface");
    const DibFrameStatus negative_begin = store.begin(-1, 0);
    require(negative_begin == DibFrameStatus::invalid, "negative extent rejected before empty");
    const DibFrameStatus oversized_begin = store.begin(std::numeric_limits<int>::max(), 2);
    require(oversized_begin == DibFrameStatus::invalid, "oversized width");
    const DibFrameStatus over_budget_begin = store.begin(4097, 4096);
    require(over_budget_begin == DibFrameStatus::invalid, "pixel cap checked");
    require_front(store, first, 2, 2, 0x00665544U);
    const DibFrameStatus retry_begin = store.begin(4, 3);
    require(retry_begin == DibFrameStatus::success, "resize retry");
    const DibFrameKey invalid{.revision = 4, .epoch = 10, .scale = std::numeric_limits<double>::infinity()};
    const DibFrameStatus invalid_scale_commit = store.commit(output.dc, damage, invalid);
    require(invalid_scale_commit == DibFrameStatus::invalid, "invalid scale");
    const DibFrameStatus invalid_receipt_commit = store.commit(output.dc, damage, {});
    require(invalid_receipt_commit == DibFrameStatus::invalid, "invalid receipt");
    require_front(store, first, 2, 2, 0x00665544U);
    fill_candidate(store, 0x00aabbccU);
    const DibFrameStatus resized_commit = store.commit(output.dc, damage, resized);
    require(resized_commit == DibFrameStatus::success, "successful resize");
    require_front(store, resized, 4, 3, 0x00aabbccU);
    store.close();
    store.close();
    require(!store.has_front() && store.front().pixels.empty(), "closed front empty");
    const DibFrameStatus closed_begin = store.begin(1, 1);
    require(closed_begin == DibFrameStatus::invalid, "closed admission rejected");
}

struct ForeignAttempt final {
    DibFrameStore& store;
    DibFrameStatus status{DibFrameStatus::success};
    void run() { status = store.begin(1, 1); }
};

void test_executor_and_cleanup() {
    DibFrameStore store{};
    ForeignAttempt attempt{.store = store};
    std::thread foreign(&ForeignAttempt::run, std::ref(attempt));
    foreign.join();
    require(attempt.status == DibFrameStatus::wrong_executor, "foreign executor rejected");
    const DWORD before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    for (int iteration = 0; iteration < 32; ++iteration) {
        DibFrameStore temporary{};
        const DibFrameStatus cleanup_begin = temporary.begin(8, 8);
        require(cleanup_begin == DibFrameStatus::success, "cleanup allocation");
        temporary.abort();
        temporary.close();
    }
    const DWORD after = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    require(after == before, "owned GDI objects released after repeated close");
}
} // namespace

int main() {
    try {
        test_failures_and_exposure();
        test_resize_and_validation();
        test_executor_and_cleanup();
        std::cout << "DIB store offscreen transaction fixtures passed; native Window lifecycle not exercised\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
