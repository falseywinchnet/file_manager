#pragma once

#include "gui_forms/c_api.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace gui_forms::abi0 {

class Error final : public std::runtime_error {
public:
    Error(gf_result result, std::string message)
        : std::runtime_error(std::move(message)), result_(result) {}

    [[nodiscard]] gf_result result() const noexcept { return result_; }

private:
    gf_result result_;
};

class Api final {
public:
    Api() {
        table_.struct_size = sizeof(table_);
        const gf_result result = gf_get_api_v0(GF_ABI_VERSION_0_1, &table_);
        if (result != GF_OK) {
            throw Error(result, "GUI.Forms ABI 0.x negotiation failed");
        }
    }

    [[nodiscard]] const gf_api_v0& table() const noexcept { return table_; }

    [[noreturn]] void throw_last(gf_result result) const {
        gf_error_view error{};
        if (table_.last_error(&error) == GF_OK && error.message.data != nullptr) {
            throw Error(result, std::string(error.message.data, error.message.size));
        }
        throw Error(result, "GUI.Forms ABI 0.x operation failed");
    }

private:
    gf_api_v0 table_{};
};

class Control final {
public:
    Control() = default;

    Control(const Api& api, std::string_view stable_id) : api_(&api) {
        const gf_string_view value{stable_id.data(), stable_id.size()};
        const gf_result result = api.table().control_create(value, &handle_);
        if (result != GF_OK) {
            api.throw_last(result);
        }
    }

    Control(const Control& other) : api_(other.api_), handle_(other.handle_) {
        if (api_ != nullptr && handle_.slot != 0U) {
            const gf_result result = api_->table().retain(handle_);
            if (result != GF_OK) {
                api_->throw_last(result);
            }
        }
    }

    Control& operator=(const Control& other) {
        if (this == &other) {
            return *this;
        }
        Control copy(other);
        swap(copy);
        return *this;
    }

    Control(Control&& other) noexcept
        : api_(std::exchange(other.api_, nullptr)),
          handle_(std::exchange(other.handle_, {})) {}

    Control& operator=(Control&& other) noexcept {
        if (this != &other) {
            reset();
            api_ = std::exchange(other.api_, nullptr);
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    ~Control() { reset(); }

    void swap(Control& other) noexcept {
        std::swap(api_, other.api_);
        std::swap(handle_, other.handle_);
    }

    void reset() noexcept {
        if (api_ != nullptr && handle_.slot != 0U) {
            static_cast<void>(api_->table().release(handle_));
        }
        api_ = nullptr;
        handle_ = {};
    }

    [[nodiscard]] gf_handle get() const noexcept { return handle_; }

    void set_visible(bool visible) const {
        check(api_->table().set_visible(handle_, visible ? 1U : 0U));
    }

    [[nodiscard]] bool visible() const {
        uint32_t value{};
        check(api_->table().get_visible(handle_, &value));
        return value != 0U;
    }

    [[nodiscard]] std::string stable_id() const {
        uint64_t required{};
        gf_result result = api_->table().stable_id(handle_, nullptr, 0U, &required);
        if (result != GF_ERROR_BUFFER_TOO_SMALL && result != GF_OK) {
            api_->throw_last(result);
        }
        std::string value(required, '\0');
        result = api_->table().stable_id(handle_, value.data(), value.size(), &required);
        check(result);
        return value;
    }

    void dispose() {
        check(api_->table().dispose(handle_));
        handle_ = {};
    }

private:
    void check(gf_result result) const {
        if (result != GF_OK) {
            api_->throw_last(result);
        }
    }

    const Api* api_{};
    gf_handle handle_{};
};

} // namespace gui_forms::abi0
