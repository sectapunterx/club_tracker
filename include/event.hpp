#ifndef COMPUTER_CLUB_EVENT_HPP
#define COMPUTER_CLUB_EVENT_HPP

#include <string>
#include <vector>

#include "time_utils.hpp"

namespace cc {

    enum class EventId : std::uint8_t {
        kClientArrived = 1,
        kClientSeated = 2,
        kClientWaiting = 3,
        kClientLeft = 4,
        // Outgoing only
        kOutgoingLeft = 11,
        kOutgoingSeated = 12,
        kError = 13,
      };

    struct IncomingEvent {
        Time time;
        EventId id{};
        std::vector<std::string> payload;  // Raw tokens after <id>
    };

    struct OutgoingEvent {
        Time time;
        EventId id{};
        std::string payload;  // Already formatted string after <id>
    };

}  // namespace cc

#endif  // COMPUTER_CLUB_EVENT_HPP