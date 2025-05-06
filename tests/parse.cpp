#include <gtest/gtest.h>

#include "parser.hpp"
#include <filesystem>
#include <fstream>

using namespace cc;

namespace {

std::filesystem::path write_tmp(const std::string& name, const std::string& text) {
    auto p = std::filesystem::temp_directory_path() / name;
    std::ofstream ofs(p);
    ofs << text;
    return p;
}

}  // namespace

TEST(Parser, ValidConfigNoEvents) {
    const auto path = write_tmp("valid_no_events.txt",
                          "3\n08:00 22:00\n100\n");
    auto r = ParseFile(path);

    EXPECT_EQ(r.cfg.table_count, 3u);
    EXPECT_EQ(r.cfg.open_time,  Time{8 * 60});
    EXPECT_EQ(r.cfg.close_time, Time{22 * 60});
    EXPECT_EQ(r.cfg.hourly_price, 100u);
    EXPECT_TRUE(r.events.empty());
}

TEST(Parser, MissingFileThrows) {
    EXPECT_THROW(ParseFile("nonexistent_file.txt"), std::runtime_error);
}

TEST(Parser, ZeroTablesThrows) {
    const auto path = write_tmp("bad_table_count.txt",
                          "0\n08:00 10:00\n10\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, OpenNotBeforeCloseThrows) {
    const auto path = write_tmp("bad_times.txt",
                          "1\n12:00 12:00\n10\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, ZeroPriceThrows) {
    const auto path = write_tmp("bad_price.txt",
                          "1\n08:00 18:00\n0\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, BadEventLineThrows) {
    const auto path = write_tmp("bad_event.txt",
                          "1\n08:00 18:00\n20\ninvalid_line\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, NonNumericTableCountThrows) {
    const auto path = write_tmp("non_numeric_table_count.txt",
                          "abc\n08:00 18:00\n10\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, ExtraTokensInOpenCloseLineThrows) {
    const auto path = write_tmp("extra_open_close.txt",
                          "1\n08:00 18:00 extra\n10\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, ExtraTokensInPriceLineThrows) {
    const auto path = write_tmp("extra_price.txt",
                          "1\n08:00 18:00\n10 extra\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, NonNumericEventIdThrows) {
    const auto path = write_tmp("non_numeric_event_id.txt",
                          "1\n08:00 18:00\n10\n"
                          "09:00 foo bob\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, EventIdOutOfRangeThrows) {
    const auto path = write_tmp("event_id_out_of_range.txt",
                          "1\n08:00 18:00\n10\n"
                          "09:00 5 alice\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, InvalidClientNameThrows) {
    const auto path = write_tmp("invalid_client_name.txt",
                          "1\n08:00 18:00\n10\n"
                          "09:00 1 Bob\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, MissingEventTokensThrows) {
    const auto path = write_tmp("missing_event_tokens.txt",
                          "1\n08:00 18:00\n10\n"
                          "09:00 1\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, EventsOutOfOrderThrows) {
    const auto path = write_tmp("events_out_of_order.txt",
                          "1\n08:00 18:00\n10\n"
                          "10:00 1 alice\n"
                          "09:00 1 alice\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}

TEST(Parser, TableIdOutOfRangeThrows) {
    const auto path = write_tmp("table_id_out_of_range.txt",
                          "1\n08:00 18:00\n10\n"
                          "09:00 1 alice 2\n");
    EXPECT_THROW(ParseFile(path), std::runtime_error);
}