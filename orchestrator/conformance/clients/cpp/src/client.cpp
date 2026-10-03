#include "fileman_orchestrator/client.hpp"
#include "search_projection.hpp"

#include <utility>

#if defined(__unix__) || defined(__APPLE__) || defined(_WIN32)

#if defined(_WIN32)
#include "client_windows.hpp"
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include <cerrno>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>
#include <thread>
#include <type_traits>
#include <variant>

namespace fileman::orchestrator {
namespace {

constexpr std::string_view wire_family = "orchestrator.local";
constexpr std::string_view wire_magic = "ORC1";
constexpr std::size_t discovery_limit = 16'384;
constexpr std::size_t credential_limit = 256;
constexpr std::size_t json_depth_limit = 64;
constexpr std::size_t json_value_limit = 16'384;

struct JsonNumber {
    std::string text{};
};

struct JsonValue {
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue, std::less<>>;
    std::variant<std::nullptr_t, bool, JsonNumber, std::string, Array, Object> value{};
};

[[noreturn]] void fail(const std::string& message) {
    throw ClientError(message);
}

void validate_utf8(const std::string_view input) {
    std::size_t position = 0;
    while (position < input.size()) {
        const unsigned char first = static_cast<unsigned char>(input[position]);
        ++position;
        if (first <= 0x7F) continue;

        std::size_t continuation_count{};
        std::uint32_t codepoint{};
        if (first >= 0xC2 && first <= 0xDF) {
            continuation_count = 1;
            codepoint = first & 0x1F;
        } else if (first >= 0xE0 && first <= 0xEF) {
            continuation_count = 2;
            codepoint = first & 0x0F;
        } else if (first >= 0xF0 && first <= 0xF4) {
            continuation_count = 3;
            codepoint = first & 0x07;
        } else {
            fail("JSON is not valid UTF-8");
        }
        if (position + continuation_count > input.size()) fail("JSON is not valid UTF-8");
        for (std::size_t index = 0; index < continuation_count; ++index) {
            const unsigned char continuation = static_cast<unsigned char>(input[position]);
            ++position;
            if ((continuation & 0xC0) != 0x80) fail("JSON is not valid UTF-8");
            codepoint = (codepoint << 6) | (continuation & 0x3F);
        }
        const bool overlong = (continuation_count == 1 && codepoint < 0x80) ||
                              (continuation_count == 2 && codepoint < 0x800) ||
                              (continuation_count == 3 && codepoint < 0x10000);
        if (overlong || (codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF) {
            fail("JSON is not valid UTF-8");
        }
    }
}

#if !defined(_WIN32)
std::string system_error(const std::string_view operation) {
    const std::string message = std::string(operation) + ": " + std::strerror(errno);
    return message;
}
#endif

class JsonParser final {
public:
    explicit JsonParser(const std::string_view input) : input_(input) { validate_utf8(input); }

    JsonValue parse() {
        JsonValue result = parse_value(0);
        whitespace();
        if (position_ != input_.size()) {
            fail("JSON contains trailing data");
        }
        return result;
    }

private:
    JsonValue parse_value(const std::size_t depth) {
        if (depth > json_depth_limit) {
            fail("JSON nesting exceeds client limit");
        }
        ++value_count_;
        if (value_count_ > json_value_limit) {
            fail("JSON value count exceeds client limit");
        }
        whitespace();
        if (position_ == input_.size()) {
            fail("JSON ended before a value");
        }
        JsonValue result{};
        switch (input_[position_]) {
            case '{': result.value = parse_object(depth + 1); break;
            case '[': result.value = parse_array(depth + 1); break;
            case '"': result.value = parse_string(); break;
            case 't': literal("true"); result.value = true; break;
            case 'f': literal("false"); result.value = false; break;
            case 'n': literal("null"); result.value = nullptr; break;
            default: result.value = parse_number(); break;
        }
        return result;
    }

    JsonValue::Object parse_object(const std::size_t depth) {
        ++position_;
        JsonValue::Object object{};
        whitespace();
        if (consume('}')) {
            return object;
        }
        for (;;) {
            whitespace();
            if (position_ == input_.size() || input_[position_] != '"') {
                fail("JSON object key is not a string");
            }
            std::string key = parse_string();
            whitespace();
            require(':');
            JsonValue value = parse_value(depth);
            const std::pair<JsonValue::Object::iterator, bool> insertion = object.emplace(std::move(key), std::move(value));
            if (!insertion.second) {
                fail("JSON object contains a duplicate key");
            }
            whitespace();
            if (consume('}')) {
                return object;
            }
            require(',');
        }
    }

    JsonValue::Array parse_array(const std::size_t depth) {
        ++position_;
        JsonValue::Array array{};
        whitespace();
        if (consume(']')) {
            return array;
        }
        for (;;) {
            array.push_back(parse_value(depth));
            whitespace();
            if (consume(']')) {
                return array;
            }
            require(',');
        }
    }

    std::string parse_string() {
        require('"');
        std::string output{};
        while (position_ < input_.size()) {
            const unsigned char byte = static_cast<unsigned char>(input_[position_]);
            ++position_;
            if (byte == '"') {
                return output;
            }
            if (byte < 0x20) {
                fail("JSON string contains an unescaped control byte");
            }
            if (byte != '\\') {
                output.push_back(static_cast<char>(byte));
                continue;
            }
            if (position_ == input_.size()) {
                fail("JSON string ends inside an escape");
            }
            switch (input_[position_++]) {
                case '"': output.push_back('"'); break;
                case '\\': output.push_back('\\'); break;
                case '/': output.push_back('/'); break;
                case 'b': output.push_back('\b'); break;
                case 'f': output.push_back('\f'); break;
                case 'n': output.push_back('\n'); break;
                case 'r': output.push_back('\r'); break;
                case 't': output.push_back('\t'); break;
                case 'u': append_unicode_escape(output); break;
                default: fail("JSON string contains an invalid escape");
            }
        }
        fail("JSON string is unterminated");
    }

    void append_unicode_escape(std::string& output) {
        std::uint32_t codepoint = parse_hex_quad();
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
            if (position_ + 2 > input_.size() || input_.substr(position_, 2) != "\\u") {
                fail("JSON high surrogate has no low surrogate");
            }
            position_ += 2;
            const std::uint32_t low = parse_hex_quad();
            if (low < 0xDC00 || low > 0xDFFF) {
                fail("JSON surrogate pair is invalid");
            }
            codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
        } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
            fail("JSON low surrogate has no high surrogate");
        }
        append_utf8(output, codepoint);
    }

    std::uint32_t parse_hex_quad() {
        if (position_ + 4 > input_.size()) {
            fail("JSON Unicode escape is truncated");
        }
        std::uint32_t value = 0;
        for (int index = 0; index < 4; ++index) {
            const char digit = input_[position_++];
            value <<= 4;
            if (digit >= '0' && digit <= '9') value |= static_cast<std::uint32_t>(digit - '0');
            else if (digit >= 'a' && digit <= 'f') value |= static_cast<std::uint32_t>(digit - 'a' + 10);
            else if (digit >= 'A' && digit <= 'F') value |= static_cast<std::uint32_t>(digit - 'A' + 10);
            else fail("JSON Unicode escape contains a non-hex digit");
        }
        return value;
    }

    static void append_utf8(std::string& output, const std::uint32_t codepoint) {
        if (codepoint <= 0x7F) {
            output.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0xFFFF) {
            output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else {
            output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    JsonNumber parse_number() {
        const std::size_t start = position_;
        consume('-');
        if (consume('0')) {
            if (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') {
                fail("JSON number has a leading zero");
            }
        } else {
            digits();
        }
        if (consume('.')) digits();
        if (position_ < input_.size() && (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;
            if (!consume('+')) consume('-');
            digits();
        }
        if (position_ == start) {
            fail("JSON value is invalid");
        }
        const std::size_t length = position_ - start;
        const std::string_view digits = input_.substr(start, length);
        JsonNumber number{std::string(digits)};
        return number;
    }

    void digits() {
        const std::size_t start = position_;
        while (position_ < input_.size() && input_[position_] >= '0' && input_[position_] <= '9') {
            ++position_;
        }
        if (position_ == start) fail("JSON number requires digits");
    }

    void literal(const std::string_view text) {
        if (input_.substr(position_, text.size()) != text) fail("JSON literal is invalid");
        position_ += text.size();
    }

    void whitespace() {
        while (position_ < input_.size()) {
            const char value = input_[position_];
            if (value != ' ' && value != '\n' && value != '\r' && value != '\t') break;
            ++position_;
        }
    }

    bool consume(const char expected) {
        if (position_ < input_.size() && input_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void require(const char expected) {
        if (!consume(expected)) fail(std::string("JSON expected '") + expected + "'");
    }

    std::string_view input_{};
    std::size_t position_{};
    std::size_t value_count_{};
};

const JsonValue::Object& object(const JsonValue& value, const std::string_view context) {
    const JsonValue::Object* result = std::get_if<JsonValue::Object>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be an object");
    return *result;
}

const JsonValue::Array& array(const JsonValue& value, const std::string_view context) {
    const JsonValue::Array* result = std::get_if<JsonValue::Array>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be an array");
    return *result;
}

const JsonValue& field(const JsonValue::Object& value, const std::string_view name) {
    const JsonValue::Object::const_iterator found = value.find(name);
    if (found == value.end()) fail("JSON object is missing field: " + std::string(name));
    return (*found).second;
}

const std::string& string(const JsonValue& value, const std::string_view context) {
    const std::string* result = std::get_if<std::string>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be a string");
    return *result;
}

bool boolean(const JsonValue& value, const std::string_view context) {
    const bool* result = std::get_if<bool>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be a Boolean");
    return *result;
}

std::uint64_t unsigned_integer(const JsonValue& value, const std::string_view context) {
    const JsonNumber* number = std::get_if<JsonNumber>(&value.value);
    if (number == nullptr || (*number).text.empty() || (*number).text.front() == '-') {
        fail(std::string(context) + " must be an unsigned integer");
    }
    std::uint64_t result{};
    const std::from_chars_result parsed = std::from_chars((*number).text.data(), (*number).text.data() + (*number).text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != (*number).text.data() + (*number).text.size()) {
        fail(std::string(context) + " is outside the unsigned integer range");
    }
    return result;
}

std::int64_t signed_integer(const JsonValue& value, const std::string_view context) {
    const JsonNumber* number = std::get_if<JsonNumber>(&value.value);
    if (number == nullptr) fail(std::string(context) + " must be a signed integer");
    std::int64_t result{};
    const std::string& text = (*number).text;
    const std::from_chars_result parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        fail(std::string(context) + " is outside the signed integer range");
    }
    return result;
}

std::string json_escape(const std::string_view input) {
    std::ostringstream output{};
    output << '"';
    constexpr char hex[] = "0123456789abcdef";
    for (const char raw : input) {
        const unsigned char byte = static_cast<unsigned char>(raw);
        switch (byte) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (byte < 0x20) {
                    output << "\\u00" << hex[byte >> 4] << hex[byte & 0x0F];
                } else {
                    output << static_cast<char>(byte);
                }
        }
    }
    output << '"';
    const std::string encoded = output.str();
    return encoded;
}

std::string read_bounded_file(const std::filesystem::path& path, const std::size_t limit) {
#if defined(_WIN32)
    const std::string bytes = windows_local::read_private(path, limit);
    return bytes;
#else
    std::ifstream input(path, std::ios::binary);
    if (!input) fail("cannot open " + path.string());
    std::string bytes{};
    bytes.resize(limit + 1);
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    bytes.resize(static_cast<std::size_t>(input.gcount()));
    if (bytes.size() > limit) fail("file exceeds byte ceiling: " + path.string());
    if (!input.eof() && input.fail()) fail("cannot read " + path.string());
    return bytes;
#endif
}

void validate_runtime_path(const std::filesystem::path& path) {
    if (!path.is_absolute() || !path.has_filename()) fail("runtime directory must be an absolute leaf");
    for (const std::filesystem::path& component : path) {
        if (component == "." || component == "..") fail("runtime directory contains a dot component");
    }
}

#if !defined(_WIN32)
struct stat checked_stat(const std::filesystem::path& path) {
    struct stat status {};
    if (::lstat(path.c_str(), &status) != 0) fail(system_error("lstat " + path.string()));
    return status;
}

void validate_private(const std::filesystem::path& path, const uid_t owner, const mode_t kind) {
    const struct stat status = checked_stat(path);
    if ((status.st_mode & S_IFMT) != kind || status.st_uid != owner) {
        fail("endpoint object has unexpected type or owner: " + path.string());
    }
    if ((status.st_mode & 0077) != 0) fail("endpoint object is not private: " + path.string());
}

void send_all(const std::intptr_t socket, const void* data, const std::size_t size) {
    const char* bytes = static_cast<const char*>(data);
    std::size_t sent = 0;
    while (sent < size) {
#if defined(MSG_NOSIGNAL)
        const ssize_t count = ::send(socket, bytes + sent, size - sent, MSG_NOSIGNAL);
#else
        const ssize_t count = ::send(socket, bytes + sent, size - sent, 0);
#endif
        if (count > 0) sent += static_cast<std::size_t>(count);
        else if (count < 0 && errno == EINTR) continue;
        else fail(system_error("send local frame"));
    }
}

void receive_all(const std::intptr_t socket, void* data, const std::size_t size) {
    char* bytes = static_cast<char*>(data);
    std::size_t received = 0;
    while (received < size) {
        const ssize_t count = ::recv(socket, bytes + received, size - received, 0);
        if (count > 0) received += static_cast<std::size_t>(count);
        else if (count == 0) fail(received == 0 ? "local session disconnected" : "local frame ended abruptly");
        else if (errno == EINTR) continue;
        else fail(system_error("receive local frame"));
    }
}

void local_close(std::intptr_t socket) noexcept { ::close(static_cast<int>(socket)); }
#else
void send_all(std::intptr_t socket, const void* data, std::size_t size) {
    windows_local::transfer(socket, const_cast<void*>(data), size, true);
}
void receive_all(std::intptr_t socket, void* data, std::size_t size) {
    windows_local::transfer(socket, data, size, false);
}
void local_close(std::intptr_t socket) noexcept { windows_local::close(socket); }
#endif

void write_frame(const std::intptr_t socket, const std::string_view payload) {
    if (payload.empty() || payload.size() > local_wire_max_frame_bytes) fail("outgoing frame violates byte ceiling");
    const std::uint32_t length = static_cast<std::uint32_t>(payload.size());
    char header[8]{};
    std::memcpy(header, wire_magic.data(), wire_magic.size());
    for (unsigned index = 0; index < 4; ++index) header[4 + index] = static_cast<char>(length >> (24 - index * 8));
    send_all(socket, header, sizeof(header));
    send_all(socket, payload.data(), payload.size());
}

std::string read_frame(const std::intptr_t socket) {
    char header[8]{};
    receive_all(socket, header, sizeof(header));
    if (std::string_view(header, 4) != wire_magic) fail("local frame has invalid magic");
    std::uint32_t length{};
    for (unsigned index = 0; index < 4; ++index) length = (length << 8) | static_cast<unsigned char>(header[4 + index]);
    if (length == 0 || length > local_wire_max_frame_bytes) fail("incoming frame violates byte ceiling");
    std::string payload(length, '\0');
    receive_all(socket, payload.data(), payload.size());
    return payload;
}

#if !defined(_WIN32)
void set_timeouts(const int socket) {
    timeval timeout{};
    timeout.tv_sec = 5;
    if (::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) {
        fail(system_error("configure local socket timeout"));
    }
}
#endif

bool is_ascii_whitespace(const unsigned char byte) {
    const bool whitespace = byte == ' ' || byte == '\n' || byte == '\r' || byte == '\t';
    return whitespace;
}

std::string trim_ascii(std::string value) {
    while (!value.empty() && is_ascii_whitespace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
    while (!value.empty() && is_ascii_whitespace(static_cast<unsigned char>(value.back()))) value.pop_back();
    return value;
}

void validate_credential(const std::string_view credential) {
    if (credential.size() != 64) fail("session credential has invalid width");
    for (const char digit : credential) {
        if (!((digit >= '0' && digit <= '9') || (digit >= 'a' && digit <= 'f') ||
              (digit >= 'A' && digit <= 'F'))) {
            fail("session credential is not hexadecimal");
        }
    }
}

struct ParsedResponse {
    JsonValue root{};

    [[nodiscard]] const JsonValue& result() const {
        const JsonValue::Object& response = object(root, "response");
        const JsonValue& value = field(response, "result");
        return value;
    }
};

ParsedResponse parse_successful_response(const std::string_view response,
                                         const std::string_view expected_id) {
    ParsedResponse parsed{JsonParser(response).parse()};
    const JsonValue::Object& root = object(parsed.root, "response");
    if (string(field(root, "id"), "response.id") != expected_id) fail("response id does not match request");
    const std::string& status = string(field(root, "status"), "response.status");
    if (status != "success") {
        const JsonValue::Object& error = object(field(root, "error"), "response.error");
        fail("Orchestrator returned " + status + ": " + string(field(error, "message"), "error.message"));
    }
    return parsed;
}

ParsedResponse parse_search_response(const std::string_view response,
                                     const std::string_view expected_id) {
    ParsedResponse parsed{JsonParser(response).parse()};
    const JsonValue::Object& root = object(parsed.root, "response");
    if (string(field(root, "id"), "response.id") != expected_id) fail("response id does not match request");
    const std::string& status = string(field(root, "status"), "response.status");
    if (status != "success" && status != "partial") {
        const JsonValue::Object& error = object(field(root, "error"), "response.error");
        fail("Orchestrator returned " + status + ": " + string(field(error, "message"), "error.message"));
    }
    return parsed;
}

std::uint16_t narrow_u16(const std::uint64_t value, const std::string_view context) {
    if (value > std::numeric_limits<std::uint16_t>::max()) fail(std::string(context) + " exceeds uint16");
    const std::uint16_t narrowed = static_cast<std::uint16_t>(value);
    return narrowed;
}

std::uint32_t narrow_u32(const std::uint64_t value, const std::string_view context) {
    if (value > std::numeric_limits<std::uint32_t>::max()) fail(std::string(context) + " exceeds uint32");
    const std::uint32_t narrowed = static_cast<std::uint32_t>(value);
    return narrowed;
}

std::optional<std::string> optional_string(const JsonValue& value,
                                           const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    const std::optional<std::string> result = string(value, context);
    return result;
}

std::optional<bool> optional_boolean(const JsonValue& value,
                                     const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    const std::optional<bool> result = boolean(value, context);
    return result;
}

std::optional<std::uint64_t> optional_unsigned_integer(
    const JsonValue& value, const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    const std::uint64_t result = unsigned_integer(value, context);
    return result;
}

std::optional<std::string> optional_object_string(
    const JsonValue::Object& record, const std::string_view name) {
    const JsonValue::Object::const_iterator found = record.find(name);
    if (found == record.end()) return std::nullopt;
    const std::optional<std::string> result = optional_string((*found).second, name);
    return result;
}

std::optional<std::string> identity_alias(const JsonValue::Object& record,
    const std::string_view runtime_name, const std::string_view semantic_name) {
    std::optional<std::string> runtime = optional_object_string(record, runtime_name);
    std::optional<std::string> semantic = optional_object_string(record, semantic_name);
    if (runtime && semantic && *runtime != *semantic) {
        fail("conflicting search object identity spellings");
    }
    if (runtime) return runtime;
    return semantic;
}

SearchObjectIdentityInfo project_search_identity(const JsonValue::Object& record) {
    SearchObjectIdentityInfo identity{};
    identity.root_id = identity_alias(record, "root", "root_id");
    identity.file_object_id = identity_alias(record, "id", "file_object_id");
    identity.incarnation = optional_object_string(record, "incarnation");
    const JsonValue::Object::const_iterator found = record.find("platform_key");
    if (found != record.end() && !std::holds_alternative<std::nullptr_t>((*found).second.value)) {
        const JsonValue::Object& fields = object((*found).second, "search object platform_key");
        identity.platform_key.emplace();
        for (const JsonValue::Object::value_type& member : fields) {
            const std::string& value = string(member.second, "search object platform_key value");
            (*identity.platform_key).emplace(member.first, value);
        }
    }
    return identity;
}

void project_search_revision(const JsonValue::Object& record,
    const JsonValue::Object& metadata, SearchResultInfo& projected) {
    const JsonValue::Object::const_iterator generation = record.find("generation");
    if (generation != record.end()) {
        projected.generation = optional_unsigned_integer((*generation).second, "search row generation");
    }
    const JsonValue::Object::const_iterator mode = metadata.find("mode");
    if (mode != metadata.end() && !std::holds_alternative<std::nullptr_t>((*mode).second.value)) {
        const std::uint64_t value = unsigned_integer((*mode).second, "search stored mode");
        projected.mode = narrow_u32(value, "search stored mode");
    }
    const JsonValue::Object::const_iterator modified = metadata.find("modified_unix_nano");
    if (modified != metadata.end() && !std::holds_alternative<std::nullptr_t>((*modified).second.value)) {
        projected.modified_unix_nanoseconds = signed_integer((*modified).second, "search stored mtime");
    }
}

SettingValue parse_setting_value(const JsonValue& value,
                                 const std::string_view context) {
    if (const bool* boolean_value = std::get_if<bool>(&value.value)) {
        return *boolean_value;
    }
    if (std::holds_alternative<JsonNumber>(value.value)) {
        const std::uint64_t result = unsigned_integer(value, context);
    return result;
    }
    if (const std::string* string_value = std::get_if<std::string>(&value.value)) {
        return *string_value;
    }
    fail(std::string(context) + " is not an admitted settings scalar");
}

struct SettingValueEncoder final {
    std::string operator()(const bool item) const {
        const std::string encoded = item ? "true" : "false";
        return encoded;
    }
    std::string operator()(const std::uint64_t item) const {
        const std::string encoded = std::to_string(item);
        return encoded;
    }
    std::string operator()(const std::string& item) const {
        const std::string encoded = json_escape(item);
        return encoded;
    }
};

std::string encode_setting_value(const SettingValue& value) {
    const SettingValueEncoder encoder{};
    const std::string encoded = std::visit(encoder, value);
    return encoded;
}

SettingsSnapshotInfo parse_settings_snapshot(const JsonValue& value) {
    const JsonValue::Object& snapshot = object(value, "settings snapshot");
    SettingsSnapshotInfo output{
        string(field(snapshot, "schema_revision"),
               "settings.snapshot.schema_revision"),
        unsigned_integer(field(snapshot, "revision"),
                         "settings.snapshot.revision"),
        string(field(snapshot, "recovery_provenance"),
               "settings.snapshot.recovery_provenance"),
        {},
    };
    const JsonValue::Object& values = object(field(snapshot, "values"),
                                "settings.snapshot.values");
    output.values.reserve(values.size());
    for (const JsonValue::Object::value_type& entry : values) {
        const std::string& id = entry.first;
        const JsonValue& setting_value = entry.second;
        output.values.push_back({id, parse_setting_value(
            setting_value, "settings.snapshot.value")});
    }
    return output;
}

SettingsCommitInfo parse_settings_commit(const JsonValue& value) {
    const JsonValue::Object& commit = object(value, "settings commit");
    if (!boolean(field(commit, "committed"), "settings.commit.committed") ||
        string(field(commit, "terminal"), "settings.commit.terminal") !=
            "success") {
        fail("settings commit did not report committed success");
    }
    SettingsCommitInfo output{
        {},
        boolean(field(commit, "restart_required"),
                "settings.commit.restart_required"),
        string(field(commit, "audit_id"), "settings.commit.audit_id"),
        parse_settings_snapshot(field(commit, "snapshot")),
    };
    for (const JsonValue& id : array(field(commit, "changed_fields"),
                                "settings.commit.changed_fields")) {
        output.changed_fields.push_back(string(id, "settings changed field"));
    }
    return output;
}

ServicesSnapshotInfo parse_services_snapshot(const JsonValue& value) {
    const JsonValue::Object& snapshot = object(value, "services snapshot");
    const JsonValue::Object& schema = object(field(snapshot, "schema"),
                                "services snapshot schema");
    ServicesSnapshotInfo output{
        narrow_u16(unsigned_integer(field(schema, "major"),
                                    "services.schema.major"),
                   "services.schema.major"),
        narrow_u16(unsigned_integer(field(schema, "minor"),
                                    "services.schema.minor"),
                   "services.schema.minor"),
        string(field(snapshot, "snapshot_kind"),
               "services.snapshot_kind"),
        {},
    };
    if (string(field(schema, "family"), "services.schema.family") !=
            "ORC-UI-001" || output.schema_major != 1) {
        fail("services snapshot uses an unsupported contract");
    }
    for (const JsonValue& item : array(field(snapshot, "services"),
                                  "services list")) {
        const JsonValue::Object& record = object(item, "service record");
        ServiceInfo service{
            string(field(record, "id"), "service.id"),
            string(field(record, "title"), "service.title"),
            string(field(record, "state"), "service.state"),
            boolean(field(record, "ready"), "service.ready"),
            optional_string(field(record, "instance_id"),
                            "service.instance_id"),
            optional_unsigned_integer(field(record, "generation"),
                                      "service.generation"),
            string(field(record, "transport"), "service.transport"),
            optional_string(field(record, "currentness"),
                            "service.currentness"),
            {},
            {},
            {},
        };
        if (const JsonValue::Object::const_iterator reason = record.find("reason"); reason != record.end()) {
            service.reason = optional_string((*reason).second, "service.reason");
        }
        for (const JsonValue& root : array(field(record, "roots"), "service.roots")) {
            service.roots.push_back(string(root, "service root"));
        }
        for (const JsonValue& item_command : array(field(record, "commands"),
                                              "service.commands")) {
            const JsonValue::Object& command = object(item_command, "service command");
            service.commands.push_back({
                string(field(command, "id"), "service.command.id"),
                string(field(command, "title"), "service.command.title"),
                boolean(field(command, "available"),
                        "service.command.available"),
                string(field(command, "effect"), "service.command.effect"),
            });
        }
        output.services.push_back(std::move(service));
    }
    return output;
}

VersionInfo parse_version_info(const JsonValue& value) {
    const JsonValue::Object& result = object(value, "version result");
    const JsonValue::Object& protocol = object(field(result, "protocol"), "version.protocol");
    const JsonValue::Object& local_wire = object(field(result, "local_wire"), "version.local_wire");
    VersionInfo output{
        string(field(result, "component"), "version.component"),
        string(field(result, "build_version"), "version.build_version"),
        string(field(protocol, "family"), "version.protocol.family"),
        narrow_u16(unsigned_integer(field(protocol, "major"), "version.protocol.major"),
                   "version.protocol.major"),
        narrow_u16(unsigned_integer(field(protocol, "minor"), "version.protocol.minor"),
                   "version.protocol.minor"),
        string(field(local_wire, "family"), "version.local_wire.family"),
        narrow_u16(unsigned_integer(field(local_wire, "major"), "version.local_wire.major"),
                   "version.local_wire.major"),
        narrow_u16(unsigned_integer(field(local_wire, "minor"), "version.local_wire.minor"),
                   "version.local_wire.minor"),
    };
    return output;
}

ReleaseInfo parse_release_info(const JsonValue& value) {
    const JsonValue::Object& result = object(value, "release result");
    ReleaseInfo release{
        string(field(result, "profile"), "release.profile"),
        string(field(result, "target_version"), "release.target_version"),
        string(field(result, "build_version"), "release.build_version"),
        string(field(result, "first_platform"), "release.first_platform"),
        string(field(result, "state"), "release.state"),
        boolean(field(result, "ready"), "release.ready"),
        {},
        {},
        {},
    };
    for (const JsonValue& contract : array(field(result, "required_contracts"),
                                      "release.required_contracts")) {
        release.required_contracts.push_back(string(contract, "required contract"));
    }
    for (const JsonValue& item : array(field(result, "requirements"), "release.requirements")) {
        const JsonValue::Object& requirement = object(item, "release requirement");
        release.requirements.push_back({
            string(field(requirement, "id"), "requirement.id"),
            string(field(requirement, "state"), "requirement.state"),
            string(field(requirement, "evidence"), "requirement.evidence"),
        });
    }
    const JsonValue::Object& provenance = object(field(result, "provenance"), "release.provenance");
    release.provenance = {
        string(field(provenance, "algorithm"), "provenance.algorithm"),
        string(field(provenance, "scope"), "provenance.scope"),
        string(field(provenance, "digest"), "provenance.digest"),
        boolean(field(provenance, "signed"), "provenance.signed"),
        unsigned_integer(field(provenance, "embedded_inputs"), "provenance.embedded_inputs"),
    };
    if (release.provenance.algorithm != "sha256" || release.provenance.digest.size() != 64) {
        fail("release provenance uses an unsupported digest shape");
    }
    return release;
}

StatusInfo parse_status_info(const JsonValue& value) {
    const JsonValue::Object& result = object(value, "status result");
    const JsonValue::Object& lifecycle = object(field(result, "lifecycle"), "status.lifecycle");
    const JsonValue::Object& core = object(field(result, "core_release"), "status.core_release");
    const JsonValue::Object& summary = object(field(result, "availability_summary"),
                                 "status.availability_summary");
    StatusInfo output{
        string(field(result, "component"), "status.component"),
        string(field(result, "scope"), "status.scope"),
        string(field(lifecycle, "state"), "lifecycle.state"),
        unsigned_integer(field(lifecycle, "generation"), "lifecycle.generation"),
        string(field(core, "state"), "core_release.state"),
        string(field(core, "target_version"), "core_release.target_version"),
        boolean(field(core, "ready"), "core_release.ready"),
        boolean(field(result, "lazy"), "status.lazy"),
        boolean(field(result, "has_gui"), "status.has_gui"),
        string(field(result, "engine_scope"), "status.engine_scope"),
        string(field(result, "normal_integration_route"), "status.normal_integration_route"),
        boolean(field(result, "degraded_engine_fallback"), "status.degraded_engine_fallback"),
        {
            unsigned_integer(field(summary, "available"), "availability.available"),
            unsigned_integer(field(summary, "degraded"), "availability.degraded"),
            unsigned_integer(field(summary, "unavailable"), "availability.unavailable"),
            unsigned_integer(field(summary, "negotiating"), "availability.negotiating"),
            unsigned_integer(field(summary, "deferred"), "availability.deferred"),
            unsigned_integer(field(summary, "stubbed"), "availability.stubbed"),
        },
    };
    return output;
}

std::vector<ContractInfo> parse_contracts(const JsonValue& value) {
    std::vector<ContractInfo> output{};
    for (const JsonValue& item : array(value, "contracts result")) {
        const JsonValue::Object& record = object(item, "contract record");
        output.push_back({
            string(field(record, "id"), "contract.id"),
            string(field(record, "name"), "contract.name"),
            string(field(record, "provider"), "contract.provider"),
            string(field(record, "stage"), "contract.stage"),
            boolean(field(record, "executable"), "contract.executable"),
        });
    }
    return output;
}

std::vector<AvailabilityInfo> parse_availability(const JsonValue& value) {
    std::vector<AvailabilityInfo> output{};
    for (const JsonValue& item : array(value, "availability result")) {
        const JsonValue::Object& record = object(item, "availability record");
        output.push_back({
            string(field(record, "id"), "availability.id"),
            string(field(record, "provider"), "availability.provider"),
            string(field(record, "state"), "availability.state"),
            string(field(record, "reason"), "availability.reason"),
            boolean(field(record, "required"), "availability.required"),
        });
    }
    return output;
}

std::optional<std::vector<std::string>> optional_string_list(
    const JsonValue::Object& record, const std::string_view name) {
    const JsonValue::Object::const_iterator found = record.find(name);
    if (found == record.end() || std::holds_alternative<std::nullptr_t>((*found).second.value)) {
        return std::nullopt;
    }
    const JsonValue::Array& values = array((*found).second, name);
    std::optional<std::vector<std::string>> result{std::in_place};
    (*result).reserve(values.size());
    for (const JsonValue& value : values) {
        (*result).push_back(string(value, name));
    }
    return result;
}

}  // namespace

SearchPageInfo detail::project_search_response(
    const std::string_view response, const std::string_view expected_id) {
    ParsedResponse parsed = parse_search_response(response, expected_id);
    const JsonValue::Object& result = object(parsed.result(), "search result");
    const JsonValue::Object& response_root = object(parsed.root, "search response");
    SearchPageInfo page{
        string(field(response_root, "status"), "search.status"),
        string(field(result, "source"), "search.source"),
        boolean(field(result, "complete"), "search.complete"),
        std::nullopt,
        std::nullopt,
        {},
        {},
        {},
    };
    if (const JsonValue::Object::const_iterator generation = result.find("generation"); generation != result.end()) {
        page.generation = optional_unsigned_integer((*generation).second,
                                                    "search.generation");
    }
    if (const JsonValue::Object::const_iterator cursor = result.find("cursor"); cursor != result.end() &&
        !std::holds_alternative<std::nullptr_t>((*cursor).second.value)) {
        const JsonValue::Object& cursor_object = object((*cursor).second, "search.cursor");
        page.cursor = SearchCursorInfo{
            string(field(cursor_object, "source"), "search.cursor.source"),
            string(field(cursor_object, "value"), "search.cursor.value")};
    }
    page.coverage.stale_roots = optional_string_list(result, "stale_roots");
    page.coverage.unavailable_roots = optional_string_list(result, "unavailable_roots");
    page.coverage.unavailable_paths = optional_string_list(result, "unavailable_paths");
    page.coverage.warnings = optional_string_list(result, "warnings");
    const JsonValue::Object::const_iterator scan = result.find("scan_id");
    if (scan != result.end()) {
        page.coverage.scan_id = optional_string((*scan).second, "search.scan_id");
    }
    const JsonValue::Array& records = array(field(result, "results"), "search.results");
    page.results.reserve(records.size());
    page.names.reserve(records.size());
    for (const JsonValue& item : records) {
        const JsonValue::Object& record = object(item, "search result record");
        const JsonValue::Object& metadata = object(field(record, "metadata"), "search result metadata");
        const JsonValue::Object& object_record = object(field(record, "object"), "search result object");
        SearchResultInfo projected{
            string(field(metadata, "name"), "search result name"),
            string(field(object_record, "path"), "search result path"),
            string(field(metadata, "kind"), "search result kind"),
            signed_integer(field(metadata, "size"), "search result size"),
            boolean(field(record, "unavailable"), "search result unavailable"),
        };
        projected.object = project_search_identity(object_record);
        project_search_revision(record, metadata, projected);
        page.names.push_back(projected.name);
        page.results.push_back(std::move(projected));
    }
    return page;
}

Client::Client(const std::intptr_t socket, SessionInfo session) noexcept
    : socket_(socket), session_(std::move(session)) {}

Client::Client(Client&& other) noexcept
    : socket_(std::exchange(other.socket_, -1)),
      next_request_id_(other.next_request_id_),
      session_(std::move(other.session_)) {}

Client& Client::operator=(Client&& other) noexcept {
    if (this != &other) {
        if (socket_ >= 0) local_close(socket_);
        socket_ = std::exchange(other.socket_, -1);
        next_request_id_ = other.next_request_id_;
        session_ = std::move(other.session_);
    }
    return *this;
}

Client::~Client() {
    if (socket_ >= 0) local_close(socket_);
}

std::filesystem::path default_runtime_directory() {
#if defined(_WIN32)
    const wchar_t* configured = _wgetenv(L"FILEMAN_ORCHESTRATOR_RUNTIME_DIR");
    if (configured != nullptr && *configured != L'\0') {
        const std::filesystem::path runtime(configured);
        validate_runtime_path(runtime);
        return runtime;
    }
#else
    const char* configured = std::getenv("FILEMAN_ORCHESTRATOR_RUNTIME_DIR");
    if (configured != nullptr && *configured != '\0') {
        const std::filesystem::path runtime(configured);
        validate_runtime_path(runtime);
        return runtime;
    }
#endif
#if defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home == nullptr || *home == '\0') fail("HOME is unavailable for macOS service discovery");
    std::filesystem::path runtime = std::filesystem::path(home);
    if (!runtime.is_absolute()) fail("HOME is not absolute for macOS service discovery");
    runtime /= "Library";
    runtime /= "Application Support";
    runtime /= "fo-orchestrator";
    return runtime;
#else
    std::error_code error{};
    std::filesystem::path runtime = std::filesystem::temp_directory_path(error);
    if (error) fail("cannot resolve the user-session temporary directory: " + error.message());
    runtime /= "fo-orchestrator";
    return runtime;
#endif
}

Client Client::connect_default(std::string client_name) {
    const std::filesystem::path runtime = default_runtime_directory();
    Client client = connect(runtime, std::move(client_name));
    return client;
}

Client Client::connect(const std::filesystem::path& runtime_directory, std::string client_name) {
#if defined(_WIN32)
    Client client = connect_once(runtime_directory, std::move(client_name));
        return client;
#else
    try {
        Client client = connect_once(runtime_directory, client_name);
        return client;
    } catch (const ClientError& initial_error) {
        const std::string initial_message = std::string(initial_error.what());
        const std::filesystem::path socket_path = runtime_directory / "orchestrator.sock";
        const int trigger = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (trigger < 0) throw;
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        const std::string encoded_path = socket_path.string();
        if (encoded_path.size() >= sizeof(address.sun_path)) {
            local_close(trigger);
            throw;
        }
        std::memcpy(address.sun_path, encoded_path.c_str(), encoded_path.size() + 1);
        if (::connect(trigger, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
            local_close(trigger);
            throw;
        }
        local_close(trigger);

        const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            try {
                Client client = connect_once(runtime_directory, client_name);
        return client;
            } catch (const ClientError&) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        fail("local daemon activation did not publish a usable endpoint; initial error: " +
             initial_message);
    }
#endif
}

Client Client::connect_once(const std::filesystem::path& runtime_directory,
                            std::string client_name) {
    validate_runtime_path(runtime_directory);
#if defined(_WIN32)
    windows_local::validate_directory(runtime_directory);
#else
    const struct stat directory = checked_stat(runtime_directory);
    if ((directory.st_mode & S_IFMT) != S_IFDIR || directory.st_uid != ::geteuid()) {
        fail("runtime directory has unexpected type or owner");
    }
    if ((directory.st_mode & 0077) != 0 || (directory.st_mode & 0700) != 0700) {
        fail("runtime directory is not private");
    }
#endif

    const std::filesystem::path discovery_path = runtime_directory / "discovery.json";
    const std::filesystem::path credential_path = runtime_directory / "session.token";
#if !defined(_WIN32)
    const std::filesystem::path socket_path = runtime_directory / "orchestrator.sock";
    validate_private(discovery_path, directory.st_uid, S_IFREG);
    validate_private(credential_path, directory.st_uid, S_IFREG);
    validate_private(socket_path, directory.st_uid, S_IFSOCK);
#endif

    std::string discovery_json = read_bounded_file(discovery_path, discovery_limit);
    const JsonValue discovery_value = JsonParser(discovery_json).parse();
    const JsonValue::Object& discovery = object(discovery_value, "discovery");
    const std::string instance_id = string(field(discovery, "instance_id"), "discovery.instance_id");
    if (string(field(discovery, "family"), "discovery.family") != wire_family ||
        unsigned_integer(field(discovery, "major"), "discovery.major") != local_wire_major ||
        unsigned_integer(field(discovery, "minor"), "discovery.minor") > local_wire_minor ||
        instance_id.empty() ||
#if !defined(_WIN32)
        string(field(discovery, "endpoint"), "discovery.endpoint") != socket_path.string() ||
#endif
        string(field(discovery, "credential_file"), "discovery.credential_file") != "session.token") {
        fail("discovery record does not match the local endpoint");
    }

    std::string credential = trim_ascii(read_bounded_file(credential_path, credential_limit));
    validate_credential(credential);
    if (client_name.empty() || client_name.size() > local_wire_max_client_name_bytes) {
        fail("client name is empty or too long");
    }

#if defined(_WIN32)
    if (string(field(discovery, "transport"), "discovery.transport") != "windows_named_pipe") fail("invalid Windows transport");
    const std::intptr_t socket = windows_local::connect(
        string(field(discovery, "endpoint"), "discovery.endpoint"),
        narrow_u32(unsigned_integer(field(discovery, "server_pid"), "discovery.server_pid"), "server_pid"),
        string(field(discovery, "user_sid"), "discovery.user_sid"));
#else
    const int socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket < 0) fail(system_error("create Unix socket"));
#endif
    try {
#if !defined(_WIN32)
        set_timeouts(socket);
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        const std::string encoded_path = socket_path.string();
        if (encoded_path.size() >= sizeof(address.sun_path)) fail("Unix socket path exceeds platform limit");
        std::memcpy(address.sun_path, encoded_path.c_str(), encoded_path.size() + 1);
        if (::connect(socket, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
            fail(system_error("connect Unix socket"));
        }
#endif

        const std::string hello = "{\"family\":\"orchestrator.local\",\"major\":" +
                                  std::to_string(local_wire_major) + ",\"minor\":" +
                                  std::to_string(local_wire_minor) + ",\"client\":" +
                                  json_escape(client_name) + ",\"credential\":" +
                                  json_escape(credential) + "}";
        write_frame(socket, hello);
        const JsonValue response = JsonParser(read_frame(socket)).parse();
        const JsonValue::Object& server = object(response, "server hello");
        if (string(field(server, "family"), "server.family") != wire_family ||
            unsigned_integer(field(server, "major"), "server.major") != local_wire_major ||
            unsigned_integer(field(server, "minor"), "server.minor") > local_wire_minor ||
            string(field(server, "instance_id"), "server.instance_id") != instance_id) {
            fail("server hello does not match discovery");
        }
        SessionInfo session{
            instance_id,
            unsigned_integer(field(server, "lifecycle_generation"), "server.lifecycle_generation"),
            narrow_u32(
                unsigned_integer(field(server, "max_frame_bytes"), "server.max_frame_bytes"),
                "server.max_frame_bytes"),
        };
        if (session.max_frame_bytes == 0 || session.max_frame_bytes > local_wire_max_frame_bytes) {
            fail("server selected an invalid frame ceiling");
        }
        Client client(socket, std::move(session));
        return client;
    } catch (...) {
        local_close(socket);
        throw;
    }
}

const SessionInfo& Client::session() const noexcept { return session_; }

std::string Client::call(const std::string_view method,
                         const std::string_view contract_id,
                         const std::uint16_t contract_major,
                         const std::uint16_t contract_minor,
                         const std::string_view params_json,
                         const bool allow_partial) {
    if (socket_ < 0) fail("local session is closed");
    const std::string request_id = "cpp-" + std::to_string(next_request_id_);
    ++next_request_id_;
    const std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    if (now.count() < 0) fail("system clock is before the Unix epoch");
    const std::uint64_t deadline = static_cast<std::uint64_t>(now.count()) + 5'000;
    const std::string request = "{\"id\":" + json_escape(request_id) + ",\"method\":" +
                         json_escape(method) + ",\"contract\":{\"id\":" +
                         json_escape(contract_id) + ",\"major\":" +
                         std::to_string(contract_major) + ",\"minor\":" +
                         std::to_string(contract_minor) + "},\"deadline_unix_ms\":" +
                         std::to_string(deadline) + ",\"params\":" + std::string(params_json) + "}";
    write_frame(socket_, request);
    std::string response = read_frame(socket_);
    ParsedResponse parsed = allow_partial ? parse_search_response(response, request_id)
                                : parse_successful_response(response, request_id);
    (void)parsed;
    return response;
}

SearchPageInfo Client::search(std::string root_id, std::string text) {
    SearchPageInfo page = search_subtree(std::move(root_id), std::nullopt, std::move(text));
    return page;
}

SearchPageInfo Client::search_subtree(
    std::string root_id,
    std::optional<std::string> relative_path,
    std::string text,
    const std::uint32_t maximum_results,
    std::optional<SearchCursorInfo> cursor,
    std::vector<SearchExactFilter> filters) {
    if (maximum_results == 0 || maximum_results > 1'000) {
        fail("search result limit is outside the contract bound");
    }
    if (cursor && ((*cursor).value.empty() ||
                   ((*cursor).source != "catalogue" &&
                    (*cursor).source != "live_filesystem"))) {
        fail("search cursor is outside the closed source/value shape");
    }
    if (text.empty() && filters.empty()) {
        fail("search requires text or at least one exact metadata filter");
    }
    if (filters.size() > 7) {
        fail("search exact-filter count exceeds the contract bound");
    }
    std::map<std::string, std::string, std::less<>> exact_filters{};
    for (SearchExactFilter& filter : filters) {
        const bool known = filter.field == "name" || filter.field == "path" ||
                           filter.field == "kind" || filter.field == "size_min" ||
                           filter.field == "size_max" ||
                           filter.field == "modified_after" ||
                           filter.field == "modified_before";
        if (!known || filter.value.empty() || filter.value.size() > 16'384) {
            fail("search exact filter is outside the frozen field/value shape");
        }
        const std::pair<std::map<std::string, std::string, std::less<>>::iterator, bool> inserted = exact_filters.emplace(
            std::move(filter.field), std::move(filter.value));
        if (!inserted.second) {
            fail("search exact filters contain a duplicate field");
        }
    }
    if (!text.empty() && exact_filters.find("name") != exact_filters.end()) {
        fail("search text and exact filters both define name");
    }
    if (!exact_filters.empty() && cursor && (*cursor).source != "catalogue") {
        fail("filtered search cursor must remain on the catalogue lane");
    }
    std::string filters_json{};
    if (!exact_filters.empty()) {
        filters_json = ",\"filters\":{";
        bool first = true;
        for (const std::map<std::string, std::string, std::less<>>::value_type& entry : exact_filters) {
            const std::string& field = entry.first;
            const std::string& value = entry.second;
            if (!first) filters_json += ',';
            first = false;
            filters_json += json_escape(field) + ':' + json_escape(value);
        }
        filters_json += '}';
    }
    const std::string params = "{\"query_id\":\"cpp-live\",\"root_id\":" + json_escape(root_id) +
                        (relative_path ? ",\"relative_path\":" + json_escape(*relative_path) : "") +
                        ",\"descendants\":true,\"text\":" + json_escape(text) +
                        filters_json +
                        (cursor ? ",\"cursor\":{\"source\":" +
                                      json_escape((*cursor).source) +
                                      ",\"value\":" + json_escape((*cursor).value) + "}"
                                : "") +
                        ",\"budget\":{\"max_results\":" + std::to_string(maximum_results) +
                        ",\"max_visited_entries\":100000,"
                        "\"max_stat_calls\":4096,\"max_wall_time_ms\":1000,"
                        "\"max_open_directories\":8,\"max_response_bytes\":131072}}";
    const std::string response = call("orchestrator.search", "ORC-FE-001", 1, 0, params, true);
    SearchPageInfo page = detail::project_search_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    return page;
}

SettingsSchemaInfo Client::settings_schema() {
    const std::string response = call("orchestrator.settings.schema", "ORC-SET-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    const JsonValue::Object& result = object(parsed.result(), "settings schema result");
    SettingsSchemaInfo schema{
        string(field(result, "schema_revision"),
               "settings.schema_revision"),
        {},
    };
    for (const JsonValue& item : array(field(result, "fields"),
                                  "settings schema fields")) {
        const JsonValue::Object& record = object(item, "settings schema field");
        SettingSchemaFieldInfo field_info{
            string(field(record, "id"), "settings.field.id"),
            string(field(record, "namespace"), "settings.field.namespace"),
            string(field(record, "presentation_tab"),
                   "settings.field.presentation_tab"),
            string(field(record, "label_key"), "settings.field.label_key"),
            string(field(record, "value_type"), "settings.field.value_type"),
            parse_setting_value(field(record, "default"),
                                "settings.field.default"),
            optional_unsigned_integer(field(record, "minimum"),
                                      "settings.field.minimum"),
            optional_unsigned_integer(field(record, "maximum"),
                                      "settings.field.maximum"),
            {},
            string(field(record, "restart_effect"),
                   "settings.field.restart_effect"),
            string(field(record, "availability"),
                   "settings.field.availability"),
            string(field(record, "availability_reason"),
                   "settings.field.availability_reason"),
        };
        for (const JsonValue& choice : array(field(record, "choices"),
                                        "settings field choices")) {
            field_info.choices.push_back(string(choice, "settings field choice"));
        }
        schema.fields.push_back(std::move(field_info));
    }
    return schema;
}

SettingsSnapshotInfo Client::settings_snapshot() {
    const std::string response = call("orchestrator.settings.snapshot", "ORC-SET-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    SettingsSnapshotInfo result = parse_settings_snapshot(parsed.result());
    return result;
}

SettingsCommitInfo Client::apply_setting(const std::uint64_t expected_revision,
                                         std::string id,
                                         SettingValue value) {
    std::vector<SettingChange> changes{};
    changes.push_back({std::move(id), std::move(value)});
    SettingsCommitInfo result = apply_settings(expected_revision, std::move(changes));
    return result;
}

SettingsCommitInfo Client::apply_settings(
    const std::uint64_t expected_revision,
    std::vector<SettingChange> changes) {
    if (changes.empty() || changes.size() > 64) {
        fail("settings transaction requires 1..=64 changes");
    }
    std::string mutations = "[";
    for (std::size_t index = 0; index < changes.size(); ++index) {
        if (index != 0) mutations += ',';
        mutations += "{\"id\":" + json_escape(changes[index].id) +
                     ",\"set\":" + encode_setting_value(changes[index].value) + "}";
    }
    mutations += ']';
    const std::string params = "{\"expected_revision\":" +
                        std::to_string(expected_revision) +
                        ",\"mutations\":" + mutations + "}";
    const std::string response = call("orchestrator.settings.apply", "ORC-SET-001", 1, 0,
                               params);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    SettingsCommitInfo result = parse_settings_commit(parsed.result());
    return result;
}

SettingsCommitInfo Client::reset_setting(const std::uint64_t expected_revision,
                                         std::string id) {
    const std::string params = "{\"expected_revision\":" +
                        std::to_string(expected_revision) +
                        ",\"mutations\":[{\"id\":" + json_escape(id) +
                        ",\"reset_to_default\":true}]}";
    const std::string response = call("orchestrator.settings.apply", "ORC-SET-001", 1, 0,
                               params);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    SettingsCommitInfo result = parse_settings_commit(parsed.result());
    return result;
}

ServicesSnapshotInfo Client::services_snapshot() {
    const std::string response = call("orchestrator.services.snapshot", "ORC-UI-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    ServicesSnapshotInfo result = parse_services_snapshot(parsed.result());
    return result;
}

ServiceCommandResultInfo Client::service_command(
    std::string service_id,
    std::string command_id,
    std::optional<std::string> expected_instance_id,
    std::optional<std::uint64_t> expected_generation,
    std::optional<std::string> root_id) {
    if (service_id.empty() || command_id.empty()) {
        fail("service and command identifiers must be nonempty");
    }
    std::string params = "{\"service_id\":" + json_escape(service_id) +
                         ",\"command_id\":" + json_escape(command_id);
    if (expected_instance_id) {
        params += ",\"expected_instance_id\":" +
                  json_escape(*expected_instance_id);
    }
    if (expected_generation) {
        params += ",\"expected_generation\":" +
                  std::to_string(*expected_generation);
    }
    if (root_id) params += ",\"root_id\":" + json_escape(*root_id);
    params += '}';
    const std::string response = call("orchestrator.services.command", "ORC-UI-001",
                               1, 0, params);
    ParsedResponse parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    const JsonValue::Object& result = object(parsed.result(), "service command result");
    ServiceCommandResultInfo output{
        string(field(result, "service_id"), "service command service_id"),
        string(field(result, "command_id"), "service command command_id"),
        string(field(result, "terminal"), "service command terminal"),
        string(field(result, "effect"), "service command effect"),
    };
    return output;
}

VersionInfo Client::version() {
    const std::string response = call("orchestrator.version", "ORC-LIF-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    VersionInfo result = parse_version_info(parsed.result());
    return result;
}

ReleaseInfo Client::release() {
    const std::string response = call("orchestrator.release", "ORC-LIF-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    ReleaseInfo result = parse_release_info(parsed.result());
    return result;
}

StatusInfo Client::status() {
    const std::string response = call("orchestrator.status", "ORC-LIF-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    StatusInfo result = parse_status_info(parsed.result());
    return result;
}

std::vector<ContractInfo> Client::contracts() {
    const std::string response = call("orchestrator.contracts.list", "ORC-COM-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    std::vector<ContractInfo> result = parse_contracts(parsed.result());
    return result;
}

std::vector<AvailabilityInfo> Client::availability() {
    const std::string response = call("orchestrator.availability.list", "ORC-COM-001", 1, 0);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    std::vector<AvailabilityInfo> result = parse_availability(parsed.result());
    return result;
}

BootstrapSnapshot Client::bootstrap() {
    const std::string response = call("orchestrator.frontend.bootstrap", "ORC-FE-001",
                               frontend_contract_major, frontend_contract_minor);
    ParsedResponse parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    const JsonValue::Object& result = object(parsed.result(), "frontend bootstrap result");
    const JsonValue::Object& schema = object(field(result, "schema"), "bootstrap.schema");
    const JsonValue::Object& identity = object(field(result, "snapshot"), "bootstrap.snapshot");
    const JsonValue::Object& routing = object(field(result, "routing"), "bootstrap.routing");
    const JsonValue::Object& fallback = object(field(routing, "direct_engine_fallback"),
                                  "bootstrap.routing.direct_engine_fallback");
    const JsonValue::Object& controls = object(field(result, "service_controls"),
                                  "bootstrap.service_controls");
    const JsonValue::Object& opening = object(field(result, "frontend_opening"),
                                 "bootstrap.frontend_opening");
    const JsonValue::Object& orchestrator_gate = object(field(opening, "orchestrator_gate"),
                                           "bootstrap.opening.orchestrator_gate");
    const JsonValue::Object& external_gates = object(field(opening, "external_gates"),
                                        "bootstrap.opening.external_gates");
    const JsonValue::Object& gui_forms_gate = object(field(external_gates, "gui_forms"),
                                        "bootstrap.opening.gui_forms");
    const JsonValue::Object& architect_gate = object(field(external_gates, "architect_direction"),
                                        "bootstrap.opening.architect_direction");
    const JsonValue::Object& opening_policy = object(field(opening, "policy"),
                                        "bootstrap.opening.policy");
    std::vector<FrontendGateBlockerInfo> opening_blockers{};
    for (const JsonValue& item : array(field(orchestrator_gate, "blockers"),
                                  "bootstrap.opening.blockers")) {
        const JsonValue::Object& blocker = object(item, "bootstrap opening blocker");
        opening_blockers.push_back({
            string(field(blocker, "kind"), "bootstrap.blocker.kind"),
            string(field(blocker, "id"), "bootstrap.blocker.id"),
            string(field(blocker, "state"), "bootstrap.blocker.state"),
            string(field(blocker, "reason"), "bootstrap.blocker.reason"),
        });
    }

    BootstrapSnapshot snapshot{
        session_,
        string(field(schema, "family"), "bootstrap.schema.family"),
        narrow_u16(unsigned_integer(field(schema, "major"), "bootstrap.schema.major"),
                   "bootstrap.schema.major"),
        narrow_u16(unsigned_integer(field(schema, "minor"), "bootstrap.schema.minor"),
                   "bootstrap.schema.minor"),
        {
            string(field(identity, "kind"), "bootstrap.snapshot.kind"),
            unsigned_integer(field(identity, "lifecycle_generation"),
                             "bootstrap.snapshot.lifecycle_generation"),
            unsigned_integer(field(identity, "configuration_generation"),
                             "bootstrap.snapshot.configuration_generation"),
        },
        parse_version_info(field(result, "version")),
        parse_release_info(field(result, "release")),
        parse_status_info(field(result, "status")),
        parse_contracts(field(result, "contracts")),
        parse_availability(field(result, "availability")),
        {
            string(field(routing, "normal_integration_route"),
                   "bootstrap.routing.normal_integration_route"),
            string(field(routing, "engine_scope"), "bootstrap.routing.engine_scope"),
            {
                boolean(field(fallback, "registered"), "bootstrap.fallback.registered"),
                boolean(field(fallback, "eligible"), "bootstrap.fallback.eligible"),
                string(field(fallback, "state"), "bootstrap.fallback.state"),
                string(field(fallback, "reason"), "bootstrap.fallback.reason"),
            },
        },
        {
            boolean(field(controls, "shutdown_eligible"), "bootstrap.controls.shutdown"),
            boolean(field(controls, "restart_eligible"), "bootstrap.controls.restart"),
            string(field(controls, "restart_strategy"),
                   "bootstrap.controls.restart_strategy"),
            string(field(controls, "restart_effect"), "bootstrap.controls.restart_effect"),
            string(field(controls, "diagnostics_state"), "bootstrap.controls.diagnostics_state"),
            optional_string(field(controls, "diagnostics_locator"),
                            "bootstrap.controls.diagnostics_locator"),
        },
        {
            {
                string(field(orchestrator_gate, "authority"),
                       "bootstrap.opening.orchestrator.authority"),
                string(field(orchestrator_gate, "state"),
                       "bootstrap.opening.orchestrator.state"),
                boolean(field(orchestrator_gate, "satisfied"),
                        "bootstrap.opening.orchestrator.satisfied"),
                std::move(opening_blockers),
            },
            {
                string(field(gui_forms_gate, "authority"),
                       "bootstrap.opening.gui_forms.authority"),
                optional_string(field(gui_forms_gate, "evidence_capability_id"),
                                "bootstrap.opening.gui_forms.evidence"),
                string(field(gui_forms_gate, "state"),
                       "bootstrap.opening.gui_forms.state"),
                optional_boolean(field(gui_forms_gate, "satisfied"),
                                 "bootstrap.opening.gui_forms.satisfied"),
            },
            {
                string(field(architect_gate, "authority"),
                       "bootstrap.opening.architect.authority"),
                std::nullopt,
                string(field(architect_gate, "state"),
                       "bootstrap.opening.architect.state"),
                optional_boolean(field(architect_gate, "satisfied"),
                                 "bootstrap.opening.architect.satisfied"),
            },
            {
                boolean(field(opening_policy, "live_snapshot_required"),
                        "bootstrap.opening.policy.live_snapshot_required"),
                boolean(field(opening_policy,
                              "separately_gated_provider_absence_blocks_opening"),
                        "bootstrap.opening.policy.provider_absence"),
                string(field(opening_policy, "stale_snapshot_authority"),
                       "bootstrap.opening.policy.stale_snapshot_authority"),
            },
        },
    };
    if (snapshot.schema_family != "ORC-FE-001" ||
        snapshot.schema_major != frontend_contract_major ||
        snapshot.schema_minor > frontend_contract_minor ||
        snapshot.snapshot.kind != "immutable") {
        fail("frontend bootstrap schema is incompatible");
    }
    if (snapshot.version.local_wire_family != wire_family ||
        snapshot.version.local_wire_major != local_wire_major ||
        snapshot.version.local_wire_minor > local_wire_minor) {
        fail("frontend bootstrap reports an incompatible local wire");
    }
    if (snapshot.snapshot.lifecycle_generation != snapshot.session.lifecycle_generation ||
        snapshot.status.lifecycle_generation != snapshot.session.lifecycle_generation) {
        fail("lifecycle generation changed during bootstrap");
    }
    if (snapshot.routing.normal_integration_route != snapshot.status.normal_integration_route ||
        snapshot.routing.engine_scope != snapshot.status.engine_scope) {
        fail("frontend bootstrap routing projections disagree");
    }
    std::vector<AvailabilityInfo>::const_iterator gui_forms = snapshot.availability.begin();
    for (; gui_forms != snapshot.availability.end(); ++gui_forms) {
        if ((*gui_forms).id == "gui_forms.consumption_manifest") break;
    }
    const bool supervisor_restart_available =
        snapshot.service_controls.restart_eligible &&
        snapshot.service_controls.restart_strategy == "shutdown_then_supervisor_reactivate" &&
        snapshot.service_controls.restart_effect == "new_instance_and_lifecycle_generation";
    const bool supervisor_restart_unavailable =
        !snapshot.service_controls.restart_eligible &&
        snapshot.service_controls.restart_strategy == "unavailable" &&
        snapshot.service_controls.restart_effect == "none";
    if (snapshot.frontend_opening.orchestrator_gate.authority != "orchestrator" ||
        (snapshot.frontend_opening.orchestrator_gate.satisfied &&
         (!snapshot.frontend_opening.orchestrator_gate.blockers.empty() ||
          snapshot.frontend_opening.orchestrator_gate.state != "available")) ||
        (!snapshot.frontend_opening.orchestrator_gate.satisfied &&
         snapshot.frontend_opening.orchestrator_gate.state != "blocked") ||
        snapshot.frontend_opening.gui_forms_gate.authority != "gui_forms" ||
        snapshot.frontend_opening.gui_forms_gate.evidence_capability_id !=
            std::optional<std::string>{"gui_forms.consumption_manifest"} ||
        gui_forms == snapshot.availability.end() ||
        snapshot.frontend_opening.gui_forms_gate.state != (*gui_forms).state ||
        snapshot.frontend_opening.gui_forms_gate.satisfied !=
            std::optional<bool>{(*gui_forms).state == "available"} ||
        snapshot.frontend_opening.architect_direction_gate.authority != "grand_architect" ||
        snapshot.frontend_opening.architect_direction_gate.state != "recorded" ||
        snapshot.frontend_opening.architect_direction_gate.satisfied !=
            std::optional<bool>{true} ||
        (!supervisor_restart_available && !supervisor_restart_unavailable) ||
        snapshot.frontend_opening.orchestrator_gate.satisfied !=
            supervisor_restart_available ||
        !snapshot.frontend_opening.policy.live_snapshot_required ||
        snapshot.frontend_opening.policy.separately_gated_provider_absence_blocks_opening ||
        snapshot.frontend_opening.policy.stale_snapshot_authority != "display_only") {
        fail("frontend opening projection is inconsistent");
    }
    return snapshot;
}

bool BootstrapSnapshot::orchestrator_gate_ready() const noexcept {
    if (!frontend_opening.orchestrator_gate.satisfied ||
        !frontend_opening.orchestrator_gate.blockers.empty() ||
        frontend_opening.orchestrator_gate.state != "available" || !release.ready ||
        !status.core_ready || status.lifecycle_state != "ready" ||
        release.state != "ready" || release.target_version != "1.0.0" ||
        status.core_release_state != release.state ||
        status.core_target_version != release.target_version ||
        schema_family != "ORC-FE-001" || schema_major != frontend_contract_major ||
        schema_minor > frontend_contract_minor ||
        snapshot.kind != "immutable" ||
        snapshot.lifecycle_generation != session.lifecycle_generation ||
        routing.normal_integration_route != "orchestrator" ||
        !routing.direct_engine_fallback.registered_route ||
        !service_controls.restart_eligible ||
        service_controls.restart_strategy != "shutdown_then_supervisor_reactivate" ||
        service_controls.restart_effect != "new_instance_and_lifecycle_generation") {
        return false;
    }
    for (const std::string& required : release.required_contracts) {
        bool found = false;
        for (const ContractInfo& item : contracts) {
            if (item.id == required && item.stage == "stable" && item.executable) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    for (const AvailabilityInfo& item : availability) {
        if (item.id == "frontend.bootstrap" && item.state == "available") return true;
    }
    return false;
}

void Client::shutdown() {
    (void)call("orchestrator.shutdown", "ORC-LIF-001", 1, 0);
    local_close(std::exchange(socket_, -1));
}

}  // namespace fileman::orchestrator

#else

namespace fileman::orchestrator {

std::filesystem::path default_runtime_directory() {
    std::filesystem::path runtime = std::filesystem::temp_directory_path();
    runtime /= "fo-orchestrator";
    return runtime;
}

Client Client::connect(const std::filesystem::path&, std::string) {
    throw ClientError("the C++ conformance client currently requires a Unix-domain socket platform");
}
Client Client::connect_default(std::string) {
    throw ClientError("the C++ conformance client currently requires a Unix-domain socket platform");
}
Client::Client(const std::intptr_t socket, SessionInfo session) noexcept : socket_(socket), session_(std::move(session)) {}
Client::Client(Client&&) noexcept = default;
Client& Client::operator=(Client&&) noexcept = default;
Client::~Client() = default;
const SessionInfo& Client::session() const noexcept { return session_; }
VersionInfo Client::version() { throw ClientError("unsupported platform"); }
ReleaseInfo Client::release() { throw ClientError("unsupported platform"); }
StatusInfo Client::status() { throw ClientError("unsupported platform"); }
std::vector<ContractInfo> Client::contracts() { throw ClientError("unsupported platform"); }
std::vector<AvailabilityInfo> Client::availability() { throw ClientError("unsupported platform"); }
BootstrapSnapshot Client::bootstrap() { throw ClientError("unsupported platform"); }
SearchPageInfo Client::search(std::string, std::string) { throw ClientError("unsupported platform"); }
SearchPageInfo Client::search_subtree(std::string, std::optional<std::string>, std::string,
                                      std::uint32_t, std::optional<SearchCursorInfo>,
                                      std::vector<SearchExactFilter>) {
    throw ClientError("unsupported platform");
}
SettingsSchemaInfo Client::settings_schema() { throw ClientError("unsupported platform"); }
SettingsSnapshotInfo Client::settings_snapshot() { throw ClientError("unsupported platform"); }
SettingsCommitInfo Client::apply_setting(std::uint64_t, std::string, SettingValue) { throw ClientError("unsupported platform"); }
SettingsCommitInfo Client::apply_settings(std::uint64_t, std::vector<SettingChange>) { throw ClientError("unsupported platform"); }
SettingsCommitInfo Client::reset_setting(std::uint64_t, std::string) { throw ClientError("unsupported platform"); }
ServicesSnapshotInfo Client::services_snapshot() { throw ClientError("unsupported platform"); }
ServiceCommandResultInfo Client::service_command(std::string, std::string,
                                                  std::optional<std::string>,
                                                  std::optional<std::uint64_t>,
                                                  std::optional<std::string>) {
    throw ClientError("unsupported platform");
}
bool BootstrapSnapshot::orchestrator_gate_ready() const noexcept { return false; }
void Client::shutdown() { throw ClientError("unsupported platform"); }
std::string Client::call(std::string_view, std::string_view, std::uint16_t, std::uint16_t, std::string_view, bool) {
    throw ClientError("unsupported platform");
}

}  // namespace fileman::orchestrator

#endif

namespace fileman::orchestrator {

const SettingValue* SettingsSnapshotInfo::find(const std::string_view id) const noexcept {
    for (const SettingValueInfo& entry : values) {
        if (entry.id == id) return &entry.value;
    }
    return nullptr;
}

const ServiceInfo* ServicesSnapshotInfo::find(const std::string_view id) const noexcept {
    for (const ServiceInfo& service : services) {
        if (service.id == id) return &service;
    }
    return nullptr;
}

}  // namespace fileman::orchestrator
