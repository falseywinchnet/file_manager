#pragma once

#include <compare>
#include <cstdint>

namespace gui_forms {

struct DateTimeValue final {
    std::int32_t year{2000};
    std::uint8_t month{1U};
    std::uint8_t day{1U};
    std::uint8_t hour{};
    std::uint8_t minute{};
    std::uint8_t second{};
    std::uint16_t millisecond{};

    friend constexpr auto operator<=>(const DateTimeValue&,
                                      const DateTimeValue&) = default;
};

[[nodiscard]] bool valid_date_time(DateTimeValue value) noexcept;
[[nodiscard]] std::uint8_t days_in_month(std::int32_t year,
                                         std::uint8_t month) noexcept;
[[nodiscard]] DateTimeValue add_days(DateTimeValue value,
                                     std::int64_t days) noexcept;
[[nodiscard]] std::uint8_t day_of_week(DateTimeValue value) noexcept;

} // namespace gui_forms
