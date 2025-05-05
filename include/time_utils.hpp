#ifndef COMPUTER_CLUB_TIME_UTILS_HPP
#define COMPUTER_CLUB_TIME_UTILS_HPP

#include <cstdint>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace cc {

    // HH:MM wrapper working on minutes‑since‑midnight.
    class Time {
    public:
        constexpr explicit Time(const std::uint16_t minutes = 0) : minutes_(minutes) {}

        // Parses "HH:MM". Throws std::invalid_argument on bad format.
        static Time Parse(const std::string& str);

        [[nodiscard]] std::string ToString() const;
        [[nodiscard]] constexpr std::uint16_t minutes() const { return minutes_; }

        // Arithmetic helpers.
        [[nodiscard]] constexpr std::uint16_t operator-(const Time rhs) const {
            return minutes_ - rhs.minutes_;
        }
        [[nodiscard]] constexpr Time operator+(const std::uint16_t delta) const {
            return Time{static_cast<std::uint16_t>(minutes_ + delta)};
        }
        [[nodiscard]] constexpr bool operator<(const Time rhs) const {
            return minutes_ < rhs.minutes_;
        }
        [[nodiscard]] constexpr bool operator<=(const Time rhs) const {
            return minutes_ <= rhs.minutes_;
        }
        [[nodiscard]] constexpr bool operator>=(const Time rhs) const {
            return minutes_ >= rhs.minutes_;
        }
        [[nodiscard]] constexpr bool operator==(const Time rhs) const {
            return minutes_ == rhs.minutes_;
        }

    private:
        std::uint16_t minutes_{}; // Minutes since midnight (0-1439)
    };

    // Ceil‑divides minutes to full hours.
    constexpr std::uint16_t MinutesToHoursRounded(const std::uint16_t minutes) {
        return static_cast<std::uint16_t>((minutes + 59) / 60);
    }

}  // namespace cc

#endif  // COMPUTER_CLUB_TIME_UTILS_HPP