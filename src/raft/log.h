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

    // Three cases: (1) index is the next free slot -> appended. (2) index
    // already holds an entry with this term -> no-op, returns true (retried
    // AppendEntries RPCs must be idempotent). (3) index already holds an
    // entry with a different term, or skips ahead of the log -> rejected,
    // returns false. On rejection the caller resolves the mismatch itself,
    // typically via cut_off_from, then retries.
    bool append_at(std::size_t index, LogEntry entry) {
        if (index >= 1 && index <= entries_.size()) {
            return entries_[index - 1].term == entry.term;
        }
        if (index != entries_.size() + 1) return false;
        entries_.push_back(std::move(entry));
        return true;
    }

private:
    std::vector<LogEntry> entries_;
};

} // namespace raft
