#include "gui_forms/text_mask.hpp"
#include "../src/core/text/text_mask/text_mask_state.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace gui_forms;
using Clock = std::chrono::steady_clock;
void require(const bool value, const char* const message) {
    if (!value) throw std::runtime_error(message);
}
class Wake final : public PreparedTextWakeTarget {
public:
    std::mutex mutex{};
    std::condition_variable condition{};
    std::size_t count{0};
    void post_prepared_text_wake() noexcept override {
        std::lock_guard<std::mutex> lock(mutex);
        ++count;
        condition.notify_all();
    }
    void wait_for(const std::size_t wanted) {
        std::unique_lock<std::mutex> lock(mutex);
        const Clock::time_point deadline = Clock::now() + std::chrono::seconds(10);
        while (count < wanted) {
            const std::cv_status status = condition.wait_until(lock, deadline);
            require(status != std::cv_status::timeout, "probe worker timeout");
        }
    }
};
std::vector<std::byte> read_font(const std::filesystem::path& directory) {
    const std::filesystem::path path = directory / "Cousine-Regular.ttf";
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    require(static_cast<bool>(input), "probe font opens");
    const std::streamsize length = input.tellg();
    require(length > 0 && length < 4*1024*1024, "probe bounded font length");
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    require(static_cast<bool>(input), "probe font read");
    return bytes;
}
double elapsed_us(const Clock::time_point begin, const Clock::time_point end) {
    const std::chrono::duration<double, std::micro> elapsed = end - begin;
    const double result = elapsed.count();
    return result;
}
struct DirectProbe final {
    std::shared_ptr<detail::MaskLedger> ledger{};
    std::shared_ptr<const detail::MaskKey> key{};
    std::array<double, 64> samples{};
    bool success{false};
    void run() noexcept {
        try {
            for (std::size_t index = 0; index < samples.size(); ++index) {
                std::shared_ptr<const detail::TextMaskStorage> output{};
                const Clock::time_point begin = Clock::now();
                const TextMaskResult result = detail::native_mask_backend().execute(ledger, key, output);
                const Clock::time_point end = Clock::now();
                if (result.status != TextMaskStatus::success || !output) return;
                samples[index] = elapsed_us(begin, end);
            }
            std::sort(samples.begin(), samples.end());
            success = true;
        } catch (...) { success = false; }
    }
};
}

int main(const int argc, char** const argv) {
    try {
        require(argc == 2, "pass approved font directory");
        const std::vector<std::byte> bytes = read_font(std::filesystem::path(argv[1]));
        Wake wake{};
        TextMaskService service{};
        EncodedFontLease bank{};
        const std::array<PreparedFontSource, 1> sources{{{.encoded = bytes}}};
        TextMaskResult result = service.create_font_bank(sources, bank);
        require(result.status == TextMaskStatus::success, "probe bank");
        std::unique_ptr<TextMaskSession> session{};
        result = service.open_session(&wake, session);
        require(result.status == TextMaskStatus::success, "probe session");
        std::array<double, 64> samples{};
        double first_us = 0.0;
        TextMaskLease mask{};
        for (std::size_t index = 0; index <= samples.size(); ++index) {
            const std::string text = "tiny label " + std::to_string(index);
            TextMaskRequestId id{};
            const Clock::time_point begin = Clock::now();
            result = (*session).submit(bank, {.utf8 = text}, id);
            require(result.status == TextMaskStatus::success, "probe submit");
            wake.wait_for(index + 1U);
            result = (*session).take(id, mask);
            const Clock::time_point end = Clock::now();
            require(result.status == TextMaskStatus::success, "probe completion");
            const TextMaskSessionSnapshot snapshot = (*session).snapshot();
            require(snapshot.occupied == 0, "probe completion consumed");
            const double duration = elapsed_us(begin, end);
            if (index == 0) first_us = duration;
            else samples[index - 1U] = duration;
        }
        std::sort(samples.begin(), samples.end());
        const Clock::time_point lookup_begin = Clock::now();
        for (unsigned index = 0; index < 10000; ++index) {
            result = (*session).lookup(bank, {.utf8 = "tiny label 64"}, mask);
            require(result.status == TextMaskStatus::success, "probe cached lookup");
        }
        const Clock::time_point lookup_end = Clock::now();
        const double lookup_mean = elapsed_us(lookup_begin, lookup_end) / 10000.0;
        DirectProbe direct{};
        direct.ledger = (*detail::TextMaskAccess::state(service)).ledger;
        detail::MaskCharge key_charge{};
        key_charge.reserve(direct.ledger, detail::MaskResource::keys, 1);
        const detail::MaskAllocator<detail::MaskKey> allocator(direct.ledger);
        std::shared_ptr<detail::MaskKey> key = std::allocate_shared<detail::MaskKey>(allocator, direct.ledger, std::move(key_charge));
        (*key).fonts = (*detail::TextMaskAccess::state(*session)).find_bank(bank);
        result = detail::normalize_mask_request({.utf8 = "tiny label 64"}, (*key).options);
        require(result.status == TextMaskStatus::success, "direct probe validated key");
        const std::string_view direct_text = "tiny label 64";
        (*key).text.resize(direct_text.size());
        std::copy(direct_text.begin(), direct_text.end(), (*key).text.begin());
        direct.key = std::move(key);
        std::thread direct_worker(&DirectProbe::run, &direct);
        direct_worker.join();
        require(direct.success, "direct private backend probe completes");
        const TextMaskBudgetSnapshot budget = service.budget_snapshot();
        std::cout << "workload=one-face-Cousine-16px-gray-tiny-label first_request_us=" << first_us
            << " unique_samples=64 p50_us=" << samples[31] << " p95_us=" << samples[60]
            << " p99_us=" << samples[63] << " max_us=" << samples[63]
            << " cached_lookup_mean_us=" << lookup_mean
            << " direct_p50_us=" << direct.samples[31] << " direct_p95_us=" << direct.samples[60]
            << " direct_max_us=" << direct.samples[63]
            << " peak_shaping_bytes=" << budget.shaping_payload.peak_admitted
            << " peak_workspace_bytes=" << budget.workspace.peak_admitted << '\n';
        (*session).join_and_release();
        return 0;
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        return 1;
    }
}
