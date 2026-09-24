#pragma once

#include <string>
#include <vector>

namespace raft {

struct LogEntry {
    int term;
    std::string command;
};

class Log {
public:
    void append(LogEntry entry) { entries_.push_back(std::move(entry)); }

    // 1-indexed, matching Raft paper convention (index 0 means "no entry").
    const LogEntry& get(std::size_t index) const { return entries_.at(index - 1); }

    std::size_t size() const { return entries_.size(); }

    // Discards this entry and everything after it. Used when a follower's
    // log conflicts with the leader's -- the leader finds the point of
    // agreement and the follower truncates from there before appending
    // the leader's entries.
    void cut_off_from(std::size_t index) {
        if (index < 1 || index > entries_.size()) return;
        entries_.resize(index - 1);
    }

private:
    std::vector<LogEntry> entries_;
};

} // namespace raft
