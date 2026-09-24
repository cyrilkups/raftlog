# RaftLog — Build Plan

## What this is

RaftLog: 5 computers that agree on the same ordered list of writes, even if one crashes or the network splits. The agreement rule is called Raft. You have a resume written for this project before it exists — this plan builds the real thing so every claim is provable, not assumed:

1. 5-node cluster, writes confirmed ("committed") in under 15ms.
2. Data saved to disk safely (write-ahead log), with periodic cleanup (snapshots) cutting disk use 65% and restart time 40%.
3. Network layer (Boost.Asio) reuses connections instead of opening new ones per message, protects itself from overload, handles 20,000+ requests/sec.
4. Tested by deliberately crashing machines and splitting the network 1,000+ times with no unsafe outcome.

You're new to both C++ and Raft, with a few weeks part-time. Scope is capped to what a portfolio project needs — no production extras.

**Rule:** every number above must come from a script or test you actually ran, saved in the repo. If the measured number differs from the resume, update the resume.

**Time check:** the phases below add to ~25–38 days of work — at part-time pace, 5–8 calendar weeks, not just "a few." Phases 0–2 (working core) fit in a few weeks; Phases 3–6 (networking, storage, chaos, benchmarks) will likely stretch further.

**One precision note:** the resume claims *write* safety only, not reads — a lagging machine can serve slightly stale reads. That's expected; don't claim otherwise.

---

## Phase 0 — Learn the Basics (3–5 days)

**Raft:** Read the Raft paper, focusing on the core rules and safety proof. Skip the section on changing cluster membership live — not needed. Use an interactive visualizer to see it in action.
*Done when:* you can explain from memory what a term is, why election timers are randomized, what "commit on majority" means, and why a new entry must match the existing log before being added.

**C++ practice** (throwaway code):
1. CMake project + one test — nail the build/test loop.
2. Two programs talking over a socket via Boost.Asio — the riskiest tech here; practice it in isolation.
3. A toy example using smart pointers instead of manual memory management.

*Done when:* all three run, and you can narrate step-by-step what happens when the network program's messages arrive.

---

## Phase 1 — Project Skeleton

```
RaftLog/
  CMakeLists.txt
  src/raft/          # agreement logic, no networking
  src/rpc/            # networking layer
  src/storage/        # WAL + snapshots
  src/node/           # wires it into a runnable program
  tests/
  bench/
  fault_injection/
  tools/
  docs/RESULTS.md      # measured numbers
```

- CMake + Boost (confirm installed via Homebrew).
- Catch2 or GTest — pick one.
- spdlog for logging — needed once machines talk to each other.
- Hand-rolled message format — skip gRPC, not needed at this scope.
- Optional: GitHub Actions running tests on push.

*Done when:* one command builds the project and runs a trivial passing test, committed to git.

---

## Phase 2 — Core Agreement Logic (~1–1.5 weeks, the biggest phase)

Simulate all 5 machines as objects in one program first, passing messages via function calls — no real networking yet. This isolates logic bugs from networking bugs.

**2a. Log:** append, look up, cut off entries from a point (for fixing conflicts). *Done:* tests cover append, conflict resolution, duplicate/out-of-order rejection.

**2b. Leader election:** followers become candidates and request votes if they don't hear from a leader in time. *Done:* a 5-node simulation reliably elects one leader normally, and a forced "two candidates at once" test still converges to one leader within a bounded number of tries. This is the most common place to get Raft subtly wrong — give it a dedicated test.

**2c. Replication & commit:** leader sends entries to followers; once a majority confirm, the entry is committed. *Done:* a committed write applies identically on every reachable node; a lagging follower catches up correctly.

**2d. Duplicate-request handling:** tag each write with a client ID + sequence number so a retried request (e.g., after a leader crash) isn't applied twice. Without this, the "safe writes" claim doesn't hold. *Done:* a test sends the same request twice, including across a simulated leader change, and it applies once.

**2e. Fault safety:** simulate a leader crashing mid-task and the network splitting into two groups, then healing. *Done:* a committed entry is never lost or overwritten, and the minority side can never commit on its own. Build this fault-injection tool to be reusable — Phase 5 extends it.

---

## Phase 3 — Real Networking (~4–6 days)

Swap the in-process messaging for real network connections between separate programs. Leave the agreement logic untouched.

- Each machine keeps one open, reusable connection per peer instead of opening a new one per message.
- Messages carry a length header so partial/misread data doesn't corrupt state.
- **Backpressure:** cap how much can queue up for a slow machine instead of letting memory grow unbounded.
- **Concurrency:** run each machine's core logic on a single thread, avoiding the need for locking around shared state.

*Done when:* all Phase 2 tests pass again as 5 separate real programs over real connections, plus a new test: kill one mid-run and confirm recovery.

**De-risk:** before wiring this into Raft, build a standalone two-program ping-pong test with the backpressure logic and get that working on its own first.

---

## Phase 4 — Disk Storage (~4–6 days)

- **Write-ahead log:** every confirmed entry is written to disk before being official, so a crashed machine can replay the file on restart. *Done:* force-kill, restart, recover exact last state.
- **Group commit:** batch multiple writes into one disk sync instead of syncing per write. Per-write disk syncs cap throughput well under 1,000/sec regardless of anything else — this is the real lever for the 20,000+ req/sec target. *Done:* measure and record throughput with batching on vs. off.
- **Snapshots:** periodically save a state summary and discard the history it covers. *Done:* (a) result is identical with or without snapshotting; (b) measured disk-space reduction with vs. without; (c) measured restart-time reduction with vs. without. These two measurements back the 65%/40% resume claim — don't guess them.

---

## Phase 5 — Fault Injection Harness (~3–5 days)

Extend the Phase 2e fault tool to run against the real networked cluster.

- Fault types: process kill, network partition (drop messages between groups in software), delay/reorder, and combinations.
- Each trial: start the cluster, send writes, inject a random fault at a random time, let it heal, check invariants (at most one leader at a time, no committed entry lost or overwritten, eventual agreement).
- *Done when:* an unattended batch of trials (start at 50–100, scale to 1,000+) produces a report of trials run and any violations found. Finding and fixing a real bug this way is a good outcome, not a failure.

---

## Phase 6 — Benchmarks & Reconciling the Resume Numbers (~3–5 days)

Build a benchmark client: many simulated clients writing continuously; measure per-write commit time and sustained throughput.

1. **<15ms commit latency:** trivial on a single laptop (likely sub-millisecond) — add artificial inter-node delay to make the test meaningful, and report the actual condition tested.
2. **65%/40% storage & recovery reduction:** workload-dependent, won't land on these exact figures. Run the Phase 4 measurement and update the resume to match.
3. **20,000+ req/sec:** reachable with good batching (Phase 4). If the honest number is lower, report that instead, with test conditions stated.
4. **1,000+ fault injections:** easiest to hit exactly — just a trial count once Phase 5 exists. Make sure fault type/timing actually varies across trials.

**Rule:** every resume number should trace to a script that regenerates it. Keep `docs/RESULTS.md` with the exact command, config, and result for each.

---

## Common Pitfalls

| Pitfall | Cause | Fix |
|---|---|---|
| Crashes/corruption in async networking code | A callback runs after the object it depends on is destroyed | Practice this pattern in Phase 0; use smart pointers consistently; keep callback chains short |
| Two leaders at once, or no leader ever settles | The trickiest part of Raft to get right | Dedicated test forcing the tie scenario (Phase 2b); assert single-leader-per-term in every later test |
| Silent log corruption | Off-by-one errors in conflict resolution | Test exhaustively in-process (Phase 2) before adding networking |
| Reproducible bugs become flaky | Real networking adds timing variance | Keep the Phase 2 in-process suite as a permanent fast regression test |
| Scope creep past the timeline | Tempting to add "just one more feature" | Write an explicit non-goals list in the README (no live membership changes, no multi-region) |
| Rounding up benchmark numbers | Pressure to keep the original resume figures | Treat the measurement tools as required deliverables; `docs/RESULTS.md` is the source of truth |

---

## End-to-End Verification

- Build/test command passes after every phase; the Phase 2 suite stays green through Phase 6.
- After Phase 3: run 5 real processes locally, send writes, confirm live replication.
- After Phase 5: run the fault harness for 100+ trials, confirm no violations (or a documented fix).
- After Phase 6: run the benchmark, record results and exact commands in `docs/RESULTS.md`.

## First Files to Create
- `CMakeLists.txt`
- `src/raft/raft_node.{h,cpp}` (Phase 2)
- `src/rpc/` (Phase 3)
- `src/storage/wal.{h,cpp}`, `src/storage/snapshot.{h,cpp}` (Phase 4)
- `fault_injection/`, `bench/` (Phases 5–6)
- `docs/RESULTS.md`

---

## Open Questions

- Target one laptop only, or also budget time for testing across real machines/VMs for a more meaningful latency number?
- Catch2 or GTest?
- Set up a GitHub repo now, or once there's real code?
- Finalize the non-goals list now, or revisit once the core works and remaining time is clearer?
</content>
