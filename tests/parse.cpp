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

TEST(Parser, ValidConfigWithEvents) {
    const auto path = write_tmp("valid_with_events.txt",
                          "2\n09:00 21:00\n50\n"
                          "10:15 101 foo bar\n"
                          "20:45 202 baz\n");
    auto r = ParseFile(path);

    EXPECT_EQ(r.cfg.table_count, 2u);
    EXPECT_EQ(r.cfg.open_time,  Time{9 * 60});
    EXPECT_EQ(r.cfg.close_time, Time{21 * 60});
    EXPECT_EQ(r.cfg.hourly_price, 50u);
    ASSERT_EQ(r.events.size(), 2u);

    EXPECT_EQ(r.events[0].time, Time{10 * 60 + 15});
    EXPECT_EQ(r.events[0].id,   EventId{101});
    EXPECT_EQ(r.events[0].payload, (std::vector<std::string>{"foo", "bar"}));

    EXPECT_EQ(r.events[1].time, Time{20 * 60 + 45});
    EXPECT_EQ(r.events[1].id,   EventId{202});
    EXPECT_EQ(r.events[1].payload, (std::vector<std::string>{"baz"}));
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