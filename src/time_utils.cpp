#include "time_utils.hpp"

namespace cc {

    Time Time::Parse(const std::string& str) {
        if (str.size() != 5 || str[2] != ':') {
            throw std::invalid_argument("Bad time format");
        }
        int h = std::stoi(str.substr(0, 2));
        int m = std::stoi(str.substr(3, 2));
        if (h < 0 || h > 23 || m < 0 || m > 59) {
            throw std::invalid_argument("Bad time value");
        }
        return Time{static_cast<std::uint16_t>(h * 60 + m)};
    }

    std::string Time::ToString() const {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << minutes_ / 60 << ':'
            << std::setw(2) << std::setfill('0') << minutes_ % 60;
        return oss.str();
    }

}  // namespace cc