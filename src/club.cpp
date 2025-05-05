#include "club.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace cc {

namespace {

constexpr std::string_view kErrNotOpenYet = "NotOpenYet";
constexpr std::string_view kErrYouShallNotPass = "YouShallNotPass";
constexpr std::string_view kErrPlaceIsBusy = "PlaceIsBusy";
constexpr std::string_view kErrClientUnknown = "ClientUnknown";
constexpr std::string_view kErrICanWaitNoLonger = "ICanWaitNoLonger!";

}  // namespace

Club::Club(const Config& cfg) : cfg_(cfg) {
  tables_.resize(cfg_.table_count);
  for (std::size_t i = 0; i < cfg_.table_count; ++i) {
    tables_[i].id = i + 1;  // 1‑based
  }
}

void Club::Run(const std::vector<IncomingEvent>& events,
               std::vector<OutgoingEvent>& log) {
  log.reserve(events.size() * 2 + 32);

  log.push_back({cfg_.open_time, EventId::kError, ""});

  for (const auto& ev : events) {
    switch (ev.id) {
      case EventId::kClientArrived:
        HandleArrived(ev, log);
        break;
      case EventId::kClientSeated:
        HandleSeated(ev, log);
        break;
      case EventId::kClientWaiting:
        HandleWaiting(ev, log);
        break;
      case EventId::kClientLeft:
        HandleLeft(ev, log);
        break;
      default:
        // For unknown IDs we just log error could extend.
        log.push_back({ev.time, EventId::kError, "BadEventId"});
    }
  }

  // Closing time: drop remaining seated/standing clients alphabetically.
  std::vector<std::string> still_inside;
  for (const auto& [name, cl] : clients_) {
    if (cl.in_club) still_inside.push_back(name);
  }
  std::ranges::sort(still_inside);
  for (const auto& name : still_inside) {
    DropClient(name, cfg_.close_time, log);
  }

  // Club closed.
  log.push_back({cfg_.close_time, EventId::kError, ""});
}

void Club::HandleArrived(const IncomingEvent& ev, std::vector<OutgoingEvent>& log) {
  log.push_back({ev.time, ev.id, ev.payload[0]});
  const auto& name = ev.payload[0];
  if (ev.time < cfg_.open_time || ev.time >= cfg_.close_time) {
    log.push_back({ev.time, EventId::kError, std::string(kErrNotOpenYet)});
    return;
  }
  auto& client = clients_[name];
  if (client.in_club) {
    log.push_back({ev.time, EventId::kError, std::string(kErrYouShallNotPass)});
    return;
  }
  client.name = name;
  client.in_club = true;
}

void Club::SeatClient(std::size_t table_idx, const std::string& name,
                      const Time time, const EventId outgoing_id,
                      std::vector<OutgoingEvent>& log) {
  Table& table = tables_[table_idx];
  table.occupant = name;
  table.occupied_since = time;
  clients_[name].table_id = table_idx;
  std::ostringstream oss;
  oss << name << ' ' << table.id;
  log.push_back({time, outgoing_id, oss.str()});
}

void Club::HandleSeated(const IncomingEvent& ev, std::vector<OutgoingEvent>& log) {
  log.push_back({ev.time, ev.id, ev.payload[0] + ' ' + ev.payload[1]});
  const auto& name = ev.payload[0];
  const std::size_t table_no = std::stoul(ev.payload[1]);
  if (table_no == 0 || table_no > tables_.size()) {
    log.push_back({ev.time, EventId::kError, "BadTable"});
    return;
  }
  const Table& table = tables_[table_no - 1];

  auto it = clients_.find(name);
  if (it == clients_.end() || !it->second.in_club) {
    log.push_back({ev.time, EventId::kError, std::string(kErrClientUnknown)});
    return;
  }
  const Client& client = it->second;

  if (table.IsBusy() && table.occupant != name) {
    log.push_back({ev.time, EventId::kError, std::string(kErrPlaceIsBusy)});
    return;
  }

  if (table.occupant == name) {
    log.push_back({ev.time, EventId::kError, std::string(kErrPlaceIsBusy)});
    return;
  }

  if (client.table_id) {
    Table& old_table = tables_[*client.table_id];
    const auto minutes = ev.time - old_table.occupied_since;
    old_table.busy_minutes += minutes;
    old_table.revenue += cfg_.hourly_price * MinutesToHoursRounded(minutes);
    old_table.occupant.reset();
  }

  SeatClient(table_no - 1, name, ev.time, EventId::kClientSeated, log);
}

void Club::HandleWaiting(const IncomingEvent& ev, std::vector<OutgoingEvent>& log) {
  log.push_back({ev.time, ev.id, ev.payload[0]});
  const auto& name = ev.payload[0];
  auto it = clients_.find(name);
  if (it == clients_.end() || !it->second.in_club) {
    log.push_back({ev.time, EventId::kError, std::string(kErrClientUnknown)});
    return;
  }
  for (const auto& t : tables_) {
    if (!t.IsBusy()) {
      log.push_back({ev.time, EventId::kError, std::string(kErrICanWaitNoLonger)});
      return;
    }
  }

  if (queue_.size() >= tables_.size()) {
    // Queue overflow – client goes away.
    log.push_back({ev.time, EventId::kOutgoingLeft, name});
    clients_.erase(name);
    return;
  }

  queue_.push_back(name);
}

void Club::DropClient(const std::string& name, Time time,
                      std::vector<OutgoingEvent>& log) {
  const auto it = clients_.find(name);
  if (it == clients_.end() || !it->second.in_club) return;

  if (it->second.table_id) {
    Table& table = tables_[*it->second.table_id];
    const auto minutes = time - table.occupied_since;
    table.busy_minutes += minutes;
    table.revenue += cfg_.hourly_price * MinutesToHoursRounded(minutes);
    table.occupant.reset();
    it->second.table_id.reset();

    if (!queue_.empty()) {
      const auto next_name = queue_.front();
      queue_.pop_front();
      SeatClient(table.id - 1, next_name, time, EventId::kOutgoingSeated, log);
    }
  }

  log.push_back({time, EventId::kOutgoingLeft, name});
  it->second.in_club = false;
}

void Club::HandleLeft(const IncomingEvent& ev, std::vector<OutgoingEvent>& log) {
  log.push_back({ev.time, ev.id, ev.payload[0]});
  DropClient(ev.payload[0], ev.time, log);
}

}  // namespace cc