#pragma once

#include "gui_forms/c_api.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace gui_forms::abi::detail {

extern thread_local gf_result last_error_code;
extern thread_local std::string last_error_message;

inline gf_result fail(gf_result result, std::string message) noexcept {
    last_error_code = result;
    try {
        last_error_message = std::move(message);
    } catch (...) {
        last_error_message.clear();
    }
    return result;
}

template <typename Operation, typename... Arguments>
gf_result translate(Operation operation, Arguments&&... arguments) noexcept {
    try {
        return std::invoke(operation,
                           std::forward<Arguments>(arguments)...);
    } catch (const std::invalid_argument& error) {
        return fail(GF_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::logic_error& error) {
        return fail(GF_ERROR_INVALID_ARGUMENT, error.what());
    } catch (const std::exception& error) {
        return fail(GF_ERROR_INTERNAL, error.what());
    } catch (...) {
        return fail(GF_ERROR_INTERNAL,
                    "GUI.Forms ABI caught a non-standard exception");
    }
}

} // namespace gui_forms::abi::detail
