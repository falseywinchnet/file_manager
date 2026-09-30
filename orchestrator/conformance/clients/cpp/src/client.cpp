#include "fileman_orchestrator/client.hpp"

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
    std::string text;
};

struct JsonValue {
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue, std::less<>>;
    std::variant<std::nullptr_t, bool, JsonNumber, std::string, Array, Object> value;
};

[[noreturn]] void fail(const std::string& message) {
    throw ClientError(message);
}

void validate_utf8(const std::string_view input) {
    std::size_t position = 0;
    while (position < input.size()) {
        const auto first = static_cast<unsigned char>(input[position++]);
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
            const auto continuation = static_cast<unsigned char>(input[position++]);
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
    return std::string(operation) + ": " + std::strerror(errno);
}
#endif

class JsonParser final {
public:
    explicit JsonParser(const std::string_view input) : input_(input) { validate_utf8(input); }

    JsonValue parse() {
        auto result = parse_value(0);
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
        if (++value_count_ > json_value_limit) {
            fail("JSON value count exceeds client limit");
        }
        whitespace();
        if (position_ == input_.size()) {
            fail("JSON ended before a value");
        }
        switch (input_[position_]) {
            case '{': return JsonValue{parse_object(depth + 1)};
            case '[': return JsonValue{parse_array(depth + 1)};
            case '"': return JsonValue{parse_string()};
            case 't': literal("true"); return JsonValue{true};
            case 'f': literal("false"); return JsonValue{false};
            case 'n': literal("null"); return JsonValue{nullptr};
            default: return JsonValue{parse_number()};
        }
    }

    JsonValue::Object parse_object(const std::size_t depth) {
        ++position_;
        JsonValue::Object object;
        whitespace();
        if (consume('}')) {
            return object;
        }
        for (;;) {
            whitespace();
            if (position_ == input_.size() || input_[position_] != '"') {
                fail("JSON object key is not a string");
            }
            auto key = parse_string();
            whitespace();
            require(':');
            auto [_, inserted] = object.emplace(std::move(key), parse_value(depth));
            if (!inserted) {
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
        JsonValue::Array array;
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
        std::string output;
        while (position_ < input_.size()) {
            const auto byte = static_cast<unsigned char>(input_[position_++]);
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
        auto codepoint = parse_hex_quad();
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
            if (position_ + 2 > input_.size() || input_.substr(position_, 2) != "\\u") {
                fail("JSON high surrogate has no low surrogate");
            }
            position_ += 2;
            const auto low = parse_hex_quad();
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
        const auto start = position_;
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
        return JsonNumber{std::string(input_.substr(start, position_ - start))};
    }

    void digits() {
        const auto start = position_;
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

    std::string_view input_;
    std::size_t position_{};
    std::size_t value_count_{};
};

const JsonValue::Object& object(const JsonValue& value, const std::string_view context) {
    const auto* result = std::get_if<JsonValue::Object>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be an object");
    return *result;
}

const JsonValue::Array& array(const JsonValue& value, const std::string_view context) {
    const auto* result = std::get_if<JsonValue::Array>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be an array");
    return *result;
}

const JsonValue& field(const JsonValue::Object& value, const std::string_view name) {
    const auto found = value.find(name);
    if (found == value.end()) fail("JSON object is missing field: " + std::string(name));
    return found->second;
}

const std::string& string(const JsonValue& value, const std::string_view context) {
    const auto* result = std::get_if<std::string>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be a string");
    return *result;
}

bool boolean(const JsonValue& value, const std::string_view context) {
    const auto* result = std::get_if<bool>(&value.value);
    if (result == nullptr) fail(std::string(context) + " must be a Boolean");
    return *result;
}

std::uint64_t unsigned_integer(const JsonValue& value, const std::string_view context) {
    const auto* number = std::get_if<JsonNumber>(&value.value);
    if (number == nullptr || number->text.empty() || number->text.front() == '-') {
        fail(std::string(context) + " must be an unsigned integer");
    }
    std::uint64_t result{};
    const auto parsed = std::from_chars(number->text.data(), number->text.data() + number->text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != number->text.data() + number->text.size()) {
        fail(std::string(context) + " is outside the unsigned integer range");
    }
    return result;
}

std::string json_escape(const std::string_view input) {
    std::ostringstream output;
    output << '"';
    constexpr char hex[] = "0123456789abcdef";
    for (const auto raw : input) {
        const auto byte = static_cast<unsigned char>(raw);
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
    return output.str();
}

std::string read_bounded_file(const std::filesystem::path& path, const std::size_t limit) {
#if defined(_WIN32)
    return windows_local::read_private(path, limit);
#else
    std::ifstream input(path, std::ios::binary);
    if (!input) fail("cannot open " + path.string());
    std::string bytes;
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
    for (const auto& component : path) {
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
    const auto status = checked_stat(path);
    if ((status.st_mode & S_IFMT) != kind || status.st_uid != owner) {
        fail("endpoint object has unexpected type or owner: " + path.string());
    }
    if ((status.st_mode & 0077) != 0) fail("endpoint object is not private: " + path.string());
}

void send_all(const std::intptr_t socket, const void* data, const std::size_t size) {
    const auto* bytes = static_cast<const char*>(data);
    std::size_t sent = 0;
    while (sent < size) {
#if defined(MSG_NOSIGNAL)
        const auto count = ::send(socket, bytes + sent, size - sent, MSG_NOSIGNAL);
#else
        const auto count = ::send(socket, bytes + sent, size - sent, 0);
#endif
        if (count > 0) sent += static_cast<std::size_t>(count);
        else if (count < 0 && errno == EINTR) continue;
        else fail(system_error("send local frame"));
    }
}

void receive_all(const std::intptr_t socket, void* data, const std::size_t size) {
    auto* bytes = static_cast<char*>(data);
    std::size_t received = 0;
    while (received < size) {
        const auto count = ::recv(socket, bytes + received, size - received, 0);
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
    char header[8];
    std::memcpy(header, wire_magic.data(), wire_magic.size());
    for (unsigned index = 0; index < 4; ++index) header[4 + index] = static_cast<char>(length >> (24 - index * 8));
    send_all(socket, header, sizeof(header));
    send_all(socket, payload.data(), payload.size());
}

std::string read_frame(const std::intptr_t socket) {
    char header[8];
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

std::string trim_ascii(std::string value) {
    const auto whitespace = [](const unsigned char byte) {
        return byte == ' ' || byte == '\n' || byte == '\r' || byte == '\t';
    };
    while (!value.empty() && whitespace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
    while (!value.empty() && whitespace(static_cast<unsigned char>(value.back()))) value.pop_back();
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
    JsonValue root;

    [[nodiscard]] const JsonValue& result() const {
        return field(object(root, "response"), "result");
    }
};

ParsedResponse parse_successful_response(const std::string_view response,
                                         const std::string_view expected_id) {
    ParsedResponse parsed{JsonParser(response).parse()};
    const auto& root = object(parsed.root, "response");
    if (string(field(root, "id"), "response.id") != expected_id) fail("response id does not match request");
    const auto& status = string(field(root, "status"), "response.status");
    if (status != "success") {
        const auto& error = object(field(root, "error"), "response.error");
        fail("Orchestrator returned " + status + ": " + string(field(error, "message"), "error.message"));
    }
    return parsed;
}

ParsedResponse parse_search_response(const std::string_view response,
                                     const std::string_view expected_id) {
    ParsedResponse parsed{JsonParser(response).parse()};
    const auto& root = object(parsed.root, "response");
    if (string(field(root, "id"), "response.id") != expected_id) fail("response id does not match request");
    const auto& status = string(field(root, "status"), "response.status");
    if (status != "success" && status != "partial") {
        const auto& error = object(field(root, "error"), "response.error");
        fail("Orchestrator returned " + status + ": " + string(field(error, "message"), "error.message"));
    }
    return parsed;
}

std::uint16_t narrow_u16(const std::uint64_t value, const std::string_view context) {
    if (value > std::numeric_limits<std::uint16_t>::max()) fail(std::string(context) + " exceeds uint16");
    return static_cast<std::uint16_t>(value);
}

std::uint32_t narrow_u32(const std::uint64_t value, const std::string_view context) {
    if (value > std::numeric_limits<std::uint32_t>::max()) fail(std::string(context) + " exceeds uint32");
    return static_cast<std::uint32_t>(value);
}

std::optional<std::string> optional_string(const JsonValue& value,
                                           const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    return string(value, context);
}

std::optional<bool> optional_boolean(const JsonValue& value,
                                     const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    return boolean(value, context);
}

std::optional<std::uint64_t> optional_unsigned_integer(
    const JsonValue& value, const std::string_view context) {
    if (std::holds_alternative<std::nullptr_t>(value.value)) return std::nullopt;
    return unsigned_integer(value, context);
}

SettingValue parse_setting_value(const JsonValue& value,
                                 const std::string_view context) {
    if (const auto* boolean_value = std::get_if<bool>(&value.value)) {
        return *boolean_value;
    }
    if (std::holds_alternative<JsonNumber>(value.value)) {
        return unsigned_integer(value, context);
    }
    if (const auto* string_value = std::get_if<std::string>(&value.value)) {
        return *string_value;
    }
    fail(std::string(context) + " is not an admitted settings scalar");
}

std::string encode_setting_value(const SettingValue& value) {
    return std::visit(
        [](const auto& item) -> std::string {
            using Item = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<Item, bool>) {
                return item ? "true" : "false";
            } else if constexpr (std::is_same_v<Item, std::uint64_t>) {
                return std::to_string(item);
            } else {
                return json_escape(item);
            }
        },
        value);
}

SettingsSnapshotInfo parse_settings_snapshot(const JsonValue& value) {
    const auto& snapshot = object(value, "settings snapshot");
    SettingsSnapshotInfo output{
        string(field(snapshot, "schema_revision"),
               "settings.snapshot.schema_revision"),
        unsigned_integer(field(snapshot, "revision"),
                         "settings.snapshot.revision"),
        string(field(snapshot, "recovery_provenance"),
               "settings.snapshot.recovery_provenance"),
        {},
    };
    const auto& values = object(field(snapshot, "values"),
                                "settings.snapshot.values");
    output.values.reserve(values.size());
    for (const auto& [id, setting_value] : values) {
        output.values.push_back({id, parse_setting_value(
            setting_value, "settings.snapshot.value")});
    }
    return output;
}

SettingsCommitInfo parse_settings_commit(const JsonValue& value) {
    const auto& commit = object(value, "settings commit");
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
    for (const auto& id : array(field(commit, "changed_fields"),
                                "settings.commit.changed_fields")) {
        output.changed_fields.push_back(string(id, "settings changed field"));
    }
    return output;
}

ServicesSnapshotInfo parse_services_snapshot(const JsonValue& value) {
    const auto& snapshot = object(value, "services snapshot");
    const auto& schema = object(field(snapshot, "schema"),
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
    for (const auto& item : array(field(snapshot, "services"),
                                  "services list")) {
        const auto& record = object(item, "service record");
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
        if (const auto reason = record.find("reason"); reason != record.end()) {
            service.reason = optional_string(reason->second, "service.reason");
        }
        for (const auto& root : array(field(record, "roots"), "service.roots")) {
            service.roots.push_back(string(root, "service root"));
        }
        for (const auto& item_command : array(field(record, "commands"),
                                              "service.commands")) {
            const auto& command = object(item_command, "service command");
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
    const auto& result = object(value, "version result");
    const auto& protocol = object(field(result, "protocol"), "version.protocol");
    const auto& local_wire = object(field(result, "local_wire"), "version.local_wire");
    return {
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
}

ReleaseInfo parse_release_info(const JsonValue& value) {
    const auto& result = object(value, "release result");
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
    for (const auto& contract : array(field(result, "required_contracts"),
                                      "release.required_contracts")) {
        release.required_contracts.push_back(string(contract, "required contract"));
    }
    for (const auto& item : array(field(result, "requirements"), "release.requirements")) {
        const auto& requirement = object(item, "release requirement");
        release.requirements.push_back({
            string(field(requirement, "id"), "requirement.id"),
            string(field(requirement, "state"), "requirement.state"),
            string(field(requirement, "evidence"), "requirement.evidence"),
        });
    }
    const auto& provenance = object(field(result, "provenance"), "release.provenance");
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
    const auto& result = object(value, "status result");
    const auto& lifecycle = object(field(result, "lifecycle"), "status.lifecycle");
    const auto& core = object(field(result, "core_release"), "status.core_release");
    const auto& summary = object(field(result, "availability_summary"),
                                 "status.availability_summary");
    return {
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
}

std::vector<ContractInfo> parse_contracts(const JsonValue& value) {
    std::vector<ContractInfo> output;
    for (const auto& item : array(value, "contracts result")) {
        const auto& record = object(item, "contract record");
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
    std::vector<AvailabilityInfo> output;
    for (const auto& item : array(value, "availability result")) {
        const auto& record = object(item, "availability record");
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

}  // namespace

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
    auto runtime = std::filesystem::path(home);
    if (!runtime.is_absolute()) fail("HOME is not absolute for macOS service discovery");
    return runtime / "Library" / "Application Support" / "fo-orchestrator";
#else
    std::error_code error;
    auto runtime = std::filesystem::temp_directory_path(error);
    if (error) fail("cannot resolve the user-session temporary directory: " + error.message());
    return runtime / "fo-orchestrator";
#endif
}

Client Client::connect_default(std::string client_name) {
    return connect(default_runtime_directory(), std::move(client_name));
}

Client Client::connect(const std::filesystem::path& runtime_directory, std::string client_name) {
#if defined(_WIN32)
    return connect_once(runtime_directory, std::move(client_name));
#else
    try {
        return connect_once(runtime_directory, client_name);
    } catch (const ClientError& initial_error) {
        const auto initial_message = std::string(initial_error.what());
        const auto socket_path = runtime_directory / "orchestrator.sock";
        const int trigger = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (trigger < 0) throw;
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        const auto encoded_path = socket_path.string();
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

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            try {
                return connect_once(runtime_directory, client_name);
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
    const auto directory = checked_stat(runtime_directory);
    if ((directory.st_mode & S_IFMT) != S_IFDIR || directory.st_uid != ::geteuid()) {
        fail("runtime directory has unexpected type or owner");
    }
    if ((directory.st_mode & 0077) != 0 || (directory.st_mode & 0700) != 0700) {
        fail("runtime directory is not private");
    }
#endif

    const auto discovery_path = runtime_directory / "discovery.json";
    const auto credential_path = runtime_directory / "session.token";
#if !defined(_WIN32)
    const auto socket_path = runtime_directory / "orchestrator.sock";
    validate_private(discovery_path, directory.st_uid, S_IFREG);
    validate_private(credential_path, directory.st_uid, S_IFREG);
    validate_private(socket_path, directory.st_uid, S_IFSOCK);
#endif

    auto discovery_json = read_bounded_file(discovery_path, discovery_limit);
    const auto discovery_value = JsonParser(discovery_json).parse();
    const auto& discovery = object(discovery_value, "discovery");
    const auto instance_id = string(field(discovery, "instance_id"), "discovery.instance_id");
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

    auto credential = trim_ascii(read_bounded_file(credential_path, credential_limit));
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
        const auto encoded_path = socket_path.string();
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
        const auto response = JsonParser(read_frame(socket)).parse();
        const auto& server = object(response, "server hello");
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
        return Client(socket, std::move(session));
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
    const auto request_id = "cpp-" + std::to_string(next_request_id_++);
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    if (now.count() < 0) fail("system clock is before the Unix epoch");
    const auto deadline = static_cast<std::uint64_t>(now.count()) + 5'000;
    const auto request = "{\"id\":" + json_escape(request_id) + ",\"method\":" +
                         json_escape(method) + ",\"contract\":{\"id\":" +
                         json_escape(contract_id) + ",\"major\":" +
                         std::to_string(contract_major) + ",\"minor\":" +
                         std::to_string(contract_minor) + "},\"deadline_unix_ms\":" +
                         std::to_string(deadline) + ",\"params\":" + std::string(params_json) + "}";
    write_frame(socket_, request);
    auto response = read_frame(socket_);
    auto parsed = allow_partial ? parse_search_response(response, request_id)
                                : parse_successful_response(response, request_id);
    (void)parsed;
    return response;
}

SearchPageInfo Client::search(std::string root_id, std::string text) {
    return search_subtree(std::move(root_id), std::nullopt, std::move(text));
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
    if (cursor && (cursor->value.empty() ||
                   (cursor->source != "catalogue" &&
                    cursor->source != "live_filesystem"))) {
        fail("search cursor is outside the closed source/value shape");
    }
    if (text.empty() && filters.empty()) {
        fail("search requires text or at least one exact metadata filter");
    }
    if (filters.size() > 7) {
        fail("search exact-filter count exceeds the contract bound");
    }
    std::map<std::string, std::string, std::less<>> exact_filters;
    for (auto& filter : filters) {
        const bool known = filter.field == "name" || filter.field == "path" ||
                           filter.field == "kind" || filter.field == "size_min" ||
                           filter.field == "size_max" ||
                           filter.field == "modified_after" ||
                           filter.field == "modified_before";
        if (!known || filter.value.empty() || filter.value.size() > 16'384) {
            fail("search exact filter is outside the frozen field/value shape");
        }
        const auto inserted = exact_filters.emplace(
            std::move(filter.field), std::move(filter.value));
        if (!inserted.second) {
            fail("search exact filters contain a duplicate field");
        }
    }
    if (!text.empty() && exact_filters.find("name") != exact_filters.end()) {
        fail("search text and exact filters both define name");
    }
    if (!exact_filters.empty() && cursor && cursor->source != "catalogue") {
        fail("filtered search cursor must remain on the catalogue lane");
    }
    std::string filters_json;
    if (!exact_filters.empty()) {
        filters_json = ",\"filters\":{";
        bool first = true;
        for (const auto& [field, value] : exact_filters) {
            if (!first) filters_json += ',';
            first = false;
            filters_json += json_escape(field) + ':' + json_escape(value);
        }
        filters_json += '}';
    }
    const auto params = "{\"query_id\":\"cpp-live\",\"root_id\":" + json_escape(root_id) +
                        (relative_path ? ",\"relative_path\":" + json_escape(*relative_path) : "") +
                        ",\"descendants\":true,\"text\":" + json_escape(text) +
                        filters_json +
                        (cursor ? ",\"cursor\":{\"source\":" +
                                      json_escape(cursor->source) +
                                      ",\"value\":" + json_escape(cursor->value) + "}"
                                : "") +
                        ",\"budget\":{\"max_results\":" + std::to_string(maximum_results) +
                        ",\"max_visited_entries\":100000,"
                        "\"max_stat_calls\":4096,\"max_wall_time_ms\":1000,"
                        "\"max_open_directories\":8,\"max_response_bytes\":131072}}";
    const auto response = call("orchestrator.search", "ORC-FE-001", 1, 0, params, true);
    auto parsed = parse_search_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    const auto& result = object(parsed.result(), "search result");
    const auto& response_root = object(parsed.root, "search response");
    SearchPageInfo page{
        string(field(response_root, "status"), "search.status"),
        string(field(result, "source"), "search.source"),
        boolean(field(result, "complete"), "search.complete"),
        std::nullopt,
        std::nullopt,
        {},
        {},
    };
    if (const auto generation = result.find("generation"); generation != result.end()) {
        page.generation = optional_unsigned_integer(generation->second,
                                                    "search.generation");
    }
    if (const auto cursor = result.find("cursor"); cursor != result.end() &&
        !std::holds_alternative<std::nullptr_t>(cursor->second.value)) {
        const auto& cursor_object = object(cursor->second, "search.cursor");
        page.cursor = SearchCursorInfo{
            string(field(cursor_object, "source"), "search.cursor.source"),
            string(field(cursor_object, "value"), "search.cursor.value")};
    }
    for (const auto& item : array(field(result, "results"), "search.results")) {
        const auto& record = object(item, "search result record");
        const auto& metadata = object(field(record, "metadata"), "search result metadata");
        const auto& object_record = object(field(record, "object"), "search result object");
        SearchResultInfo projected{
            string(field(metadata, "name"), "search result name"),
            string(field(object_record, "path"), "search result path"),
            string(field(metadata, "kind"), "search result kind"),
            unsigned_integer(field(metadata, "size"), "search result size"),
            boolean(field(record, "unavailable"), "search result unavailable"),
        };
        page.names.push_back(projected.name);
        page.results.push_back(std::move(projected));
    }
    return page;
}

SettingsSchemaInfo Client::settings_schema() {
    const auto response = call("orchestrator.settings.schema", "ORC-SET-001", 1, 0);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    const auto& result = object(parsed.result(), "settings schema result");
    SettingsSchemaInfo schema{
        string(field(result, "schema_revision"),
               "settings.schema_revision"),
        {},
    };
    for (const auto& item : array(field(result, "fields"),
                                  "settings schema fields")) {
        const auto& record = object(item, "settings schema field");
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
        for (const auto& choice : array(field(record, "choices"),
                                        "settings field choices")) {
            field_info.choices.push_back(string(choice, "settings field choice"));
        }
        schema.fields.push_back(std::move(field_info));
    }
    return schema;
}

SettingsSnapshotInfo Client::settings_snapshot() {
    const auto response = call("orchestrator.settings.snapshot", "ORC-SET-001", 1, 0);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_settings_snapshot(parsed.result());
}

SettingsCommitInfo Client::apply_setting(const std::uint64_t expected_revision,
                                         std::string id,
                                         SettingValue value) {
    return apply_settings(expected_revision,
                          {{std::move(id), std::move(value)}});
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
    const auto params = "{\"expected_revision\":" +
                        std::to_string(expected_revision) +
                        ",\"mutations\":" + mutations + "}";
    const auto response = call("orchestrator.settings.apply", "ORC-SET-001", 1, 0,
                               params);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_settings_commit(parsed.result());
}

SettingsCommitInfo Client::reset_setting(const std::uint64_t expected_revision,
                                         std::string id) {
    const auto params = "{\"expected_revision\":" +
                        std::to_string(expected_revision) +
                        ",\"mutations\":[{\"id\":" + json_escape(id) +
                        ",\"reset_to_default\":true}]}";
    const auto response = call("orchestrator.settings.apply", "ORC-SET-001", 1, 0,
                               params);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_settings_commit(parsed.result());
}

ServicesSnapshotInfo Client::services_snapshot() {
    const auto response = call("orchestrator.services.snapshot", "ORC-UI-001", 1, 0);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_services_snapshot(parsed.result());
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
    const auto response = call("orchestrator.services.command", "ORC-UI-001",
                               1, 0, params);
    auto parsed = parse_successful_response(
        response, "cpp-" + std::to_string(next_request_id_ - 1));
    const auto& result = object(parsed.result(), "service command result");
    return {
        string(field(result, "service_id"), "service command service_id"),
        string(field(result, "command_id"), "service command command_id"),
        string(field(result, "terminal"), "service command terminal"),
        string(field(result, "effect"), "service command effect"),
    };
}

VersionInfo Client::version() {
    const auto response = call("orchestrator.version", "ORC-LIF-001", 1, 0);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_version_info(parsed.result());
}

ReleaseInfo Client::release() {
    const auto response = call("orchestrator.release", "ORC-LIF-001", 1, 0);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_release_info(parsed.result());
}

StatusInfo Client::status() {
    const auto response = call("orchestrator.status", "ORC-LIF-001", 1, 0);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_status_info(parsed.result());
}

std::vector<ContractInfo> Client::contracts() {
    const auto response = call("orchestrator.contracts.list", "ORC-COM-001", 1, 0);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_contracts(parsed.result());
}

std::vector<AvailabilityInfo> Client::availability() {
    const auto response = call("orchestrator.availability.list", "ORC-COM-001", 1, 0);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    return parse_availability(parsed.result());
}

BootstrapSnapshot Client::bootstrap() {
    const auto response = call("orchestrator.frontend.bootstrap", "ORC-FE-001",
                               frontend_contract_major, frontend_contract_minor);
    auto parsed = parse_successful_response(response, "cpp-" + std::to_string(next_request_id_ - 1));
    const auto& result = object(parsed.result(), "frontend bootstrap result");
    const auto& schema = object(field(result, "schema"), "bootstrap.schema");
    const auto& identity = object(field(result, "snapshot"), "bootstrap.snapshot");
    const auto& routing = object(field(result, "routing"), "bootstrap.routing");
    const auto& fallback = object(field(routing, "direct_engine_fallback"),
                                  "bootstrap.routing.direct_engine_fallback");
    const auto& controls = object(field(result, "service_controls"),
                                  "bootstrap.service_controls");
    const auto& opening = object(field(result, "frontend_opening"),
                                 "bootstrap.frontend_opening");
    const auto& orchestrator_gate = object(field(opening, "orchestrator_gate"),
                                           "bootstrap.opening.orchestrator_gate");
    const auto& external_gates = object(field(opening, "external_gates"),
                                        "bootstrap.opening.external_gates");
    const auto& gui_forms_gate = object(field(external_gates, "gui_forms"),
                                        "bootstrap.opening.gui_forms");
    const auto& architect_gate = object(field(external_gates, "architect_direction"),
                                        "bootstrap.opening.architect_direction");
    const auto& opening_policy = object(field(opening, "policy"),
                                        "bootstrap.opening.policy");
    std::vector<FrontendGateBlockerInfo> opening_blockers;
    for (const auto& item : array(field(orchestrator_gate, "blockers"),
                                  "bootstrap.opening.blockers")) {
        const auto& blocker = object(item, "bootstrap opening blocker");
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
    const auto gui_forms = std::find_if(snapshot.availability.begin(),
                                        snapshot.availability.end(), [](const auto& item) {
        return item.id == "gui_forms.consumption_manifest";
    });
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
        snapshot.frontend_opening.gui_forms_gate.state != gui_forms->state ||
        snapshot.frontend_opening.gui_forms_gate.satisfied !=
            std::optional<bool>{gui_forms->state == "available"} ||
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
    for (const auto& required : release.required_contracts) {
        const auto found = std::find_if(contracts.begin(), contracts.end(), [&](const auto& item) {
            return item.id == required && item.stage == "stable" && item.executable;
        });
        if (found == contracts.end()) return false;
    }
    const auto bootstrap = std::find_if(availability.begin(), availability.end(), [](const auto& item) {
        return item.id == "frontend.bootstrap" && item.state == "available";
    });
    return bootstrap != availability.end();
}

void Client::shutdown() {
    (void)call("orchestrator.shutdown", "ORC-LIF-001", 1, 0);
    local_close(std::exchange(socket_, -1));
}

}  // namespace fileman::orchestrator

#else

namespace fileman::orchestrator {

std::filesystem::path default_runtime_directory() {
    return std::filesystem::temp_directory_path() / "fo-orchestrator";
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
