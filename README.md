# RaftLog

A 5-node Raft cluster: agreement, write-ahead log + snapshots, a Boost.Asio networking layer, and a fault-injection harness. See `plan.md` for the build plan and `docs/RESULTS.md` for measured numbers.

## Non-goals

- No live cluster membership changes (fixed 5-node set)
- No multi-region / WAN deployment
- No read-path linearizability guarantees (writes only — see plan.md's precision note)
- No pluggable storage backends or transport protocols beyond what's built
