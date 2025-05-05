#ifndef COMPUTER_CLUB_TABLE_HPP
#define COMPUTER_CLUB_TABLE_HPP

#include <cstdint>
#include <string>
#include <optional>

#include "time_utils.hpp"

namespace cc {

    struct Table {
        std::size_t id = 0;          // 1‑based index
        std::optional<std::string> occupant;  // nullopt if free
        Time occupied_since{};       // valid only if occupant
        std::uint32_t revenue = 0;   // money earned today (currency units)
        std::uint32_t busy_minutes = 0;  // total minutes occupied today

        [[nodiscard]] bool IsBusy() const { return occupant.has_value(); }
    };

}  // namespace cc

#endif  // COMPUTER_CLUB_TABLE_HPP