#pragma once

#include <string>
#include <vector>

namespace raft {

struct LogEntry {
    int term;
    std::string command;
};

// Append-only log of entries. No networking, no conflict resolution yet --
// just storage.
class Log {
public:
    void append(LogEntry entry) { entries_.push_back(std::move(entry)); }

    // 1-indexed, matching Raft paper convention (index 0 means "no entry").
    const LogEntry& get(std::size_t index) const { return entries_.at(index - 1); }

    std::size_t size() const { return entries_.size(); }

private:
    std::vector<LogEntry> entries_;
};

} // namespace raft
