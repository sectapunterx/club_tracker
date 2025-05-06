#include "parser.hpp"

#include <charconv>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>

namespace {

/// ---------- helpers --------------------------------------------------------
[[noreturn]] void Fail(std::size_t line, const std::string& msg) {
    std::ostringstream oss;
    oss << "Line " << line << ": " << msg;
    throw cc::ValidationError(oss.str());
}

template <typename UInt>
UInt ToUInt(std::string_view token, const char* what, std::size_t line) {
    UInt v{};
    auto res = std::from_chars(token.data(), token.data() + token.size(), v);
    if (res.ec != std::errc{} || v == 0) {
        Fail(line, std::string("bad ") + what);
    }
    return v;
}

bool NameOk(std::string_view s) {
    static const std::regex kRe(R"(^[a-z0-9_-]+$)");
    return std::regex_match(s.begin(), s.end(), kRe);
}

bool LooksLikeTableId(const std::string& tok) {
    return !tok.empty() && std::all_of(tok.begin(), tok.end(), ::isdigit);
}

}  // namespace

namespace cc {

ParsedInput ParseFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open '" + path.string() + '\'');

    ParsedInput out;
    std::string line;
    std::size_t line_no = 0;

    auto getline_checked = [&]() -> std::string {
        if (!std::getline(in, line)) Fail(line_no + 1, "unexpected EOF");
        ++line_no;
        return line;
    };

    // ----------- 1. table count --------------------------------------------
    {
        std::string s = getline_checked();
        out.cfg.table_count =
            ToUInt<std::size_t>(std::string_view{s}, "table count", line_no);
    }

    // ----------- 2. open / close times -------------------------------------
    {
        std::istringstream ss(getline_checked());
        std::string open_s, close_s;
        if (!(ss >> open_s >> close_s))
            Fail(line_no, "expected two times separated by space");
        out.cfg.open_time  = Time::Parse(open_s);
        out.cfg.close_time = Time::Parse(close_s);
        if (!(out.cfg.open_time < out.cfg.close_time))
            Fail(line_no, "open time must be earlier than close time");
    }

    // ----------- 3. hourly price -------------------------------------------
    {
        std::string s = getline_checked();
        out.cfg.hourly_price =
            ToUInt<std::uint32_t>(std::string_view{s}, "hourly price", line_no);
    }

    // ----------- 4. events --------------------------------------------------
    Time last_time{0};
    while (std::getline(in, line)) {
        ++line_no;
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string time_s;
        int id_int;
        std::string name;

        if (!(ss >> time_s >> id_int >> name))
            Fail(line_no, "bad event header");

        if (!NameOk(name))
            Fail(line_no, "invalid client name: " + name);

        IncomingEvent ev;
        ev.time = Time::Parse(time_s);
        ev.id   = static_cast<EventId>(id_int);
        ev.payload.push_back(std::move(name));

        std::string tok;
        while (ss >> tok) ev.payload.push_back(tok);

        // chronological order
        if (ev.time < last_time)
            Fail(line_no, "events out of chronological order");
        last_time = ev.time;

        // if first extra token is pure digits, treat as table number
        if (ev.payload.size() >= 2 && LooksLikeTableId(ev.payload[1])) {
            std::size_t table = ToUInt<std::size_t>(ev.payload[1], "table id", line_no);
            if (table == 0 || table > out.cfg.table_count)
                Fail(line_no, "table id out of range");
        }

        out.events.push_back(std::move(ev));
    }

    return out;
}

}  // namespace cc
