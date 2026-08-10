#pragma once

#include "gui_forms/controls/panel/date_time_picker/date_time_value/date_time_value.hpp"

#include <array>
#include <string>

namespace gui_forms {

struct DateTimeFormatProvider final {
    std::array<std::string, 12> month_names;
    std::array<std::string, 12> abbreviated_month_names;
    std::array<std::string, 7> day_names;
    std::array<std::string, 7> abbreviated_day_names;
    std::string long_date_pattern{"dddd, MMMM d, yyyy"};
    std::string short_date_pattern{"M/d/yyyy"};
    std::string time_pattern{"h:mm tt"};
    std::string am_designator{"AM"};
    std::string pm_designator{"PM"};

    [[nodiscard]] static DateTimeFormatProvider english_united_states();
};

} // namespace gui_forms
