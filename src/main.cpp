#include <iostream>
#include <vector>

#include "club.hpp"
#include "parser.hpp"

static void PrintLog(const std::vector<cc::OutgoingEvent>& log) {
    auto dump = [](const cc::OutgoingEvent& ev) {
        if (ev.id == cc::EventId::kError && ev.payload.empty()) {
            std::cout << ev.time.ToString() << '\n';
            return;
        }
        std::cout << ev.time.ToString() << ' ' << static_cast<int>(ev.id) << ' '
                  << ev.payload << '\n';
    };
    for (const auto& ev : log) dump(ev);
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: computer_club <input_file>\n";
            return 1;
        }

        const auto parsed = cc::ParseFile(argv[1]);
        cc::Club club(parsed.cfg);

        std::vector<cc::OutgoingEvent> log;
        club.Run(parsed.events, log);

        PrintLog(log);

        for (const auto& t : club.tables()) {
            const std::uint16_t h = t.busy_minutes / 60;
            const std::uint16_t m = t.busy_minutes % 60;
            std::cout << t.id << ' ' << t.revenue << ' ' << cc::Time(h * 60 + m).ToString()
                      << '\n';
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }
    return 0;
}