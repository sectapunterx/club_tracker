#include "parser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace cc {

    static void Fail(std::size_t line, const std::string& msg) {
        std::ostringstream oss;
        oss << "Line " << line << ": " << msg;
        throw std::runtime_error(oss.str());
    }

    ParsedInput ParseFile(const std::filesystem::path& path) {
        std::ifstream in(path);
        if (!in) {
            throw std::runtime_error("Cannot open input file");
        }

        ParsedInput res;
        std::string line;
        std::size_t line_no = 0;

        auto next_line = [&]() -> std::string {
            std::getline(in, line);
            ++line_no;
            return line;
        };

        // 1. Table count
        {
            auto s = next_line();
            std::istringstream ss(s);
            if (!(ss >> res.cfg.table_count) || res.cfg.table_count == 0) {
                Fail(line_no, "Bad table count");
            }
        }

        // 2. Open / close
        {
            auto s = next_line();
            std::istringstream ss(s);
            std::string open_s, close_s;
            if (!(ss >> open_s >> close_s)) {
                Fail(line_no, "Bad open/close times");
            }
            res.cfg.open_time = Time::Parse(open_s);
            res.cfg.close_time = Time::Parse(close_s);
            if (res.cfg.open_time >= res.cfg.close_time) {
                Fail(line_no, "Open time must be before close time");
            }
        }

        // 3. Price
        {
            auto s = next_line();
            std::istringstream ss(s);
            if (!(ss >> res.cfg.hourly_price) || res.cfg.hourly_price == 0) {
                Fail(line_no, "Bad hourly price");
            }
        }

        // 4. Events
        while (std::getline(in, line)) {
            ++line_no;
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string time_s;
            int id_int;
            if (!(ss >> time_s >> id_int)) {
                Fail(line_no, "Bad event header");
            }
            IncomingEvent ev;
            ev.time = Time::Parse(time_s);
            ev.id = static_cast<EventId>(id_int);

            std::string tok;
            while (ss >> tok) {
                ev.payload.push_back(tok);
            }
            res.events.push_back(std::move(ev));
        }

        return res;
    }

}  // namespace cc