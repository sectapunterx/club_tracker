#ifndef COMPUTER_CLUB_CLIENT_HPP
#define COMPUTER_CLUB_CLIENT_HPP

#include <string>
#include <optional>

namespace cc {

    struct Client {
        std::string name;
        bool in_club = false;
        std::optional<std::size_t> table_id;  // nullopt if standing / waiting
    };

}  // namespace cc

#endif  // COMPUTER_CLUB_CLIENT_HPP