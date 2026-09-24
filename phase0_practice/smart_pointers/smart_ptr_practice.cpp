// Throwaway Phase 0 practice: smart pointers instead of manual new/delete.
#include <iostream>
#include <memory>
#include <vector>

struct LogEntry {
    int term;
    std::string command;
    LogEntry(int t, std::string c) : term(t), command(std::move(c)) {}
    ~LogEntry() { std::cout << "  ~LogEntry(" << command << ")\n"; }
};

int main() {
    // unique_ptr: single owner. A log owns its entries -- nobody else should.
    std::vector<std::unique_ptr<LogEntry>> log;
    log.push_back(std::make_unique<LogEntry>(1, "set x=1"));
    log.push_back(std::make_unique<LogEntry>(1, "set y=2"));
    std::cout << "log has " << log.size() << " entries\n";

    // Ownership transfers, never copies -- log no longer owns entry 0 after this.
    std::unique_ptr<LogEntry> taken = std::move(log[0]);
    std::cout << "taken entry command: " << taken->command << "\n";
    std::cout << "log[0] is now null: " << (log[0] == nullptr) << "\n";

    // shared_ptr: multiple owners, refcounted. Used when the same object
    // legitimately needs to be reachable from more than one place at once --
    // e.g. a Session kept alive by both "this node" and a pending async callback.
    std::shared_ptr<LogEntry> shared_entry = std::make_shared<LogEntry>(2, "set z=3");
    std::cout << "refcount after creation: " << shared_entry.use_count() << "\n";
    {
        std::shared_ptr<LogEntry> second_owner = shared_entry;
        std::cout << "refcount with second owner: " << shared_entry.use_count() << "\n";
    } // second_owner destroyed here, refcount drops
    std::cout << "refcount after scope exit: " << shared_entry.use_count() << "\n";

    std::cout << "end of main -- destructors run now:\n";
    return 0;
}
