#include <gtest/gtest.h>
#include <club.hpp>

TEST(ClubRun, SingleClientCompleteSession)
{
    cc::Config cfg{1u, cc::Time{600u}, cc::Time{700u}, 10u};
    cc::Club club(cfg);

    std::vector<cc::IncomingEvent> events{
        {cc::Time{600u}, cc::EventId::kClientArrived, {"Alice"}},
        {cc::Time{605u}, cc::EventId::kClientSeated, {"Alice", "1"}},
        {cc::Time{650u}, cc::EventId::kClientLeft, {"Alice"}}
    };
    std::vector<cc::OutgoingEvent> log;
    club.Run(events, log);

    EXPECT_EQ(log[0].time, cc::Time{600u});
    EXPECT_EQ(log[0].id, cc::EventId::kError);

    EXPECT_EQ(log[1].id, cc::EventId::kClientArrived);
    EXPECT_EQ(log[1].payload, "Alice");

    EXPECT_EQ(log[2].id, cc::EventId::kClientSeated);
    EXPECT_EQ(log[2].payload, "Alice 1");

    EXPECT_EQ(log[3].id, cc::EventId::kClientSeated);
    EXPECT_EQ(log[3].payload, "Alice 1");

    EXPECT_EQ(log[4].id, cc::EventId::kClientLeft);
    EXPECT_EQ(log[4].payload, "Alice");

    EXPECT_EQ(log[5].id, cc::EventId::kOutgoingLeft);
    EXPECT_EQ(log[5].payload, "Alice");

    EXPECT_EQ(log.back().time, cc::Time{700u});
    EXPECT_EQ(log.back().id, cc::EventId::kError);

    const auto& table = club.tables()[0];
    EXPECT_EQ(table.busy_minutes, 45u);
    EXPECT_EQ(table.revenue, 10u);
}

TEST(ClubRun, HandlesWaitingAndAutomaticSeatingOnDrop)
{
    cc::Config cfg{1u, cc::Time{0u}, cc::Time{200u}, 5u};
    cc::Club club(cfg);

    std::vector<cc::IncomingEvent> events{
        {cc::Time{10u}, cc::EventId::kClientArrived, {"Bob"}},
        {cc::Time{10u}, cc::EventId::kClientSeated, {"Bob", "1"}},
        {cc::Time{20u}, cc::EventId::kClientArrived, {"Carol"}},
        {cc::Time{20u}, cc::EventId::kClientWaiting, {"Carol"}},
        {cc::Time{50u}, cc::EventId::kClientLeft, {"Bob"}}
    };
    std::vector<cc::OutgoingEvent> log;
    club.Run(events, log);

    // Carol should be seated automatically when Bob leaves
    bool sawOutgoingSeatedForCarol = false;
    for (auto& e : log) {
        if (e.id == cc::EventId::kOutgoingSeated && e.payload == "Carol 1" && e.time == cc::Time{50u})
            sawOutgoingSeatedForCarol = true;
    }
    EXPECT_TRUE(sawOutgoingSeatedForCarol);
}

TEST(ClubRun, QueueOverflowRemovesClient)
{
    cc::Config cfg{1u, cc::Time{0u}, cc::Time{100u}, 1u};
    cc::Club club(cfg);

    std::vector<cc::IncomingEvent> events{
        {cc::Time{5u}, cc::EventId::kClientArrived, {"A"}},
        {cc::Time{5u}, cc::EventId::kClientSeated, {"A", "1"}},
        {cc::Time{10u}, cc::EventId::kClientArrived, {"B"}},
        {cc::Time{10u}, cc::EventId::kClientWaiting, {"B"}},
        {cc::Time{15u}, cc::EventId::kClientArrived, {"C"}},
        {cc::Time{15u}, cc::EventId::kClientWaiting, {"C"}}
    };
    std::vector<cc::OutgoingEvent> log;
    club.Run(events, log);

    bool sawOutgoingLeftForC = false;
    for (auto& e : log) {
        if (e.id == cc::EventId::kOutgoingLeft && e.payload == "C" && e.time == cc::Time{15u})
            sawOutgoingLeftForC = true;
    }
    EXPECT_TRUE(sawOutgoingLeftForC);
}

TEST(ClubRun, ArrivingBeforeOpenGeneratesError)
{
    cc::Config cfg{2u, cc::Time{100u}, cc::Time{200u}, 10u};
    cc::Club club(cfg);

    std::vector<cc::IncomingEvent> events{
        {cc::Time{50u}, cc::EventId::kClientArrived, {"Dave"}}
    };
    std::vector<cc::OutgoingEvent> log;
    club.Run(events, log);

    EXPECT_EQ(log[1].id, cc::EventId::kClientArrived);
    EXPECT_EQ(log[2].id, cc::EventId::kError);
    EXPECT_EQ(log[2].payload, "NotOpenYet");
}

TEST(ClubRun, RemainingClientsDroppedAtClose)
{
    cc::Config cfg{2u, cc::Time{0u}, cc::Time{100u}, 2u};
    cc::Club club(cfg);

    std::vector<cc::IncomingEvent> events{
        {cc::Time{10u}, cc::EventId::kClientArrived, {"Eve"}},
        {cc::Time{10u}, cc::EventId::kClientSeated, {"Eve", "1"}},
        {cc::Time{20u}, cc::EventId::kClientArrived, {"Frank"}}
    };
    std::vector<cc::OutgoingEvent> log;
    club.Run(events, log);

    bool sawOutgoingLeftForEve = false;
    bool sawOutgoingLeftForFrank = false;
    for (auto& e : log) {
        if (e.id == cc::EventId::kOutgoingLeft && e.payload == "Eve")
            sawOutgoingLeftForEve = true;
        if (e.id == cc::EventId::kOutgoingLeft && e.payload == "Frank")
            sawOutgoingLeftForFrank = true;
    }
    EXPECT_TRUE(sawOutgoingLeftForEve);
    EXPECT_TRUE(sawOutgoingLeftForFrank);
}