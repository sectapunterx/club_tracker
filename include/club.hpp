#ifndef COMPUTER_CLUB_CLUB_HPP
#define COMPUTER_CLUB_CLUB_HPP

#include <deque>
#include <unordered_map>

#include "client.hpp"
#include "parser.hpp"
#include "table.hpp"

namespace cc {

    class Club {
    public:
        explicit Club(const Config& cfg);

        // Process a chronological list of events, appending results to `log`.
        void Run(const std::vector<IncomingEvent>& events,
                 std::vector<OutgoingEvent>& log);

        // After Run() outputs per‑table stats.
        [[nodiscard]] const std::vector<Table>& tables() const { return tables_; }

    private:
        void HandleArrived(const IncomingEvent& ev, std::vector<OutgoingEvent>& log);
        void HandleSeated(const IncomingEvent& ev, std::vector<OutgoingEvent>& log);
        void HandleWaiting(const IncomingEvent& ev, std::vector<OutgoingEvent>& log);
        void HandleLeft(const IncomingEvent& ev, std::vector<OutgoingEvent>& log);

        void SeatClient(std::size_t table_idx, const std::string& name,
                        Time time, EventId outgoing_id,
                        std::vector<OutgoingEvent>& log);

        void DropClient(const std::string& name, Time time,
                        std::vector<OutgoingEvent>& log,
                        bool emit_left_event = true);

        Config cfg_;
        std::vector<Table> tables_;
        std::deque<std::string> queue_;  // FIFO waiting clients
        std::unordered_map<std::string, Client> clients_;
    };

}  // namespace cc

#endif  // COMPUTER_CLUB_CLUB_HPP