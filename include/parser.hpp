#ifndef COMPUTER_CLUB_PARSER_HPP
#define COMPUTER_CLUB_PARSER_HPP

#include <filesystem>
#include <vector>

#include "event.hpp"
#include "time_utils.hpp"

namespace cc {

    struct Config {
        std::size_t table_count = 0;
        Time open_time;
        Time close_time;
        std::uint32_t hourly_price = 0;
    };

    // Throws std::runtime_error on parsing problems.
    struct ParsedInput {
        Config cfg;
        std::vector<IncomingEvent> events;
    };

    ParsedInput ParseFile(const std::filesystem::path& path);

}  // namespace cc

#endif  // COMPUTER_CLUB_PARSER_HPP