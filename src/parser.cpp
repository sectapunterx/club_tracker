#include "parser.hpp"

#include <charconv>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>

namespace {

[[noreturn]] void Fail(std::size_t line, const std::string& msg) {
    std::ostringstream oss;
    oss << "Line " << line << ": " << msg;
    throw cc::ValidationError(oss.str());
}

// ---------- helpers for numeric conversion ---------------------------------
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

bool IsDigits(std::string_view s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

/// Read next non‑empty line, preserving original numbering.
std::string ReadNonEmpty(std::ifstream& in, std::string& buf, std::size_t& line) {
    while (std::getline(in, buf)) {
        ++line;
        if (!buf.empty()) return buf;
    }
    Fail(line + 1, "unexpected EOF");   // never returns
}

}  // namespace

namespace cc {

ParsedInput ParseFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open '" + path.string() + '\'');

    ParsedInput out;
    std::string line;
    std::size_t line_no = 0;

    // ---------- 1. table count -------------------------------------------------
    {
        std::string s = ReadNonEmpty(in, line, line_no);
        std::istringstream ss(s);
        std::string tok, extra;
        if (!(ss >> tok) || ss >> extra)
            Fail(line_no, "expected single integer table count");
        out.cfg.table_count = ToUInt<std::size_t>(tok, "table count", line_no);
    }

    // ---------- 2. open / close times -----------------------------------------
    {
        std::istringstream ss(ReadNonEmpty(in, line, line_no));
        std::string open_s, close_s, extra;
        if (!(ss >> open_s >> close_s) || (ss >> extra))
            Fail(line_no, "expected two times: <open> <close>");

        out.cfg.open_time  = Time::Parse(open_s);
        out.cfg.close_time = Time::Parse(close_s);
        if (!(out.cfg.open_time < out.cfg.close_time))
            Fail(line_no, "open time must be earlier than close time");
    }

    // ---------- 3. hourly price -----------------------------------------------
    {
        std::string s = ReadNonEmpty(in, line, line_no);
        std::istringstream ss(s);
        std::string price_s, extra;
        if (!(ss >> price_s) || (ss >> extra))
            Fail(line_no, "expected single integer hourly price");
        out.cfg.hourly_price =
            ToUInt<std::uint32_t>(price_s, "hourly price", line_no);
    }

    // ----------- 4. events --------------------------------------------------
    Time last_time{0};

    while (std::getline(in, line)) {
        ++line_no;
        if (line.empty()) continue;

        std::istringstream ss(line);

        std::string time_tok;
        std::string id_tok;
        std::string first_payload;

        //  Обязательный минимум: три токена
        if (!(ss >> time_tok >> id_tok >> first_payload))
            Fail(line_no, "event must be: <time> <id> <payload>");

        if (!IsDigits(id_tok))
            Fail(line_no, "event id must be positive integer");
        int id_int = ToUInt<int>(id_tok, "event id", line_no);

        if (!NameOk(first_payload))
            Fail(line_no, "invalid client name: " + first_payload);

        IncomingEvent ev;
        ev.time = Time::Parse(time_tok);
        ev.id   = static_cast<EventId>(id_int);
        ev.payload.push_back(std::move(first_payload));

        /*  Остальные токены → payload          */
        std::string tok;
        while (ss >> tok) ev.payload.push_back(tok);

        if (ev.time < last_time)
            Fail(line_no, "events out of chronological order");
        last_time = ev.time;

        /*  Если второй payload – цифры, считаем это номером стола  */
        if (ev.payload.size() >= 2 && IsDigits(ev.payload[1])) {
            auto table =
                ToUInt<std::size_t>(ev.payload[1], "table id", line_no);
            if (table == 0 || table > out.cfg.table_count)
                Fail(line_no, "table id out of range (1.." +
                               std::to_string(out.cfg.table_count) + ')');
        }

        out.events.push_back(std::move(ev));
    }


    return out;
}

}  // namespace cc
