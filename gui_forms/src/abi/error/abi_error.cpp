#include "abi_error.hpp"

namespace gui_forms::abi::detail {

thread_local gf_result last_error_code = GF_OK;
thread_local std::string last_error_message;

} // namespace gui_forms::abi::detail
