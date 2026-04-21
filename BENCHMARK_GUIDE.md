# BENCHMARK_GUIDE.md - Order Book Benchmarking Strategy

## Overview

This guide outlines the strategic approach to benchmarking the C++20 order matching engine. The goal is to establish rigorous performance baselines, validate complexity claims (especially O(1) cancellation), and create a data-driven foundation for future optimizations.

**Key Principle**: *Measure before optimizing*. This separates production-grade engineering from premature over-optimization.

---

## Why Benchmarking Matters for This Project

### Portfolio Value
- **Shows rigor**: Employers recognize "I measured performance" as a hallmark of serious quant dev thinking
- **Prevents over-optimization**: Data proves what actually matters (is logging the bottleneck or the data structure?)
- **Interview-ready answers**: "Can you handle 100k orders/second?" → You'll have the answer with supporting charts
- **Competitive edge**: Most student projects skip benchmarking; this separates you

### Project Validation
- **Proof of correctness**: Throughput + trade count validation ensures matching logic works under load
- **Complexity validation**: Measure O(1) cancellation latency curves to prove the claim
- **Design justification**: Document why `std::map<Price, std::list<Order>>` is the right choice vs. alternatives

---

## Key Metrics to Measure (Priority Order)

### 1. **Insertion Latency** (Primary benchmark)
- **What**: Time to add a single order via `OrderBook::addOrder()`
- **Why**: Core operation; proves scalability with book depth
- **Measure at**: 100, 1K, 10K, 100K, 1M orders in book
- **Target**: Should be O(log N) for map insertion; expect 1-10 μs even at 1M depth
- **Interpretation**: If latency grows with depth, the O(log N) claim holds

### 2. **Cancellation Latency** (Critical claim validation)
- **What**: Time to cancel an order via `OrderBook::cancelOrder()`
- **Why**: AGENTS.md claims O(1) via `orderIndex`; must prove it
- **Measure at**: Same depths as insertion (100, 1K, 10K, 100K, 1M)
- **Target**: Should be constant (~0.5-2 μs) regardless of book depth
- **Interpretation**: If latency stays flat, O(1) claim is validated; if it grows, orderIndex isn't working as expected

### 3. **Best Bid/Ask Lookup Latency** (Constant-time validation)
- **What**: Time for `getBestBidOrder()` and `getBestAskOrder()`
- **Why**: These calls happen in the matching loop; must be sub-microsecond
- **Measure at**: Various book depths
- **Target**: < 100 ns (near-constant)
- **Interpretation**: Validates that `std::map::begin()` is truly O(1)

### 4. **Throughput** (Integration-level metric)
- **What**: Orders processed per second via `MatchingEngine::processOrder()`
- **Why**: Real-world performance under realistic workloads
- **Measure at**: 10K, 100K, 1M order sequences under different scenarios (see below)
- **Target**: Goal is > 100K orders/sec on modern hardware (actual target TBD by baseline)
- **Interpretation**: Shows real-world capability; catches unexpected bottlenecks (e.g., allocator, logging)

### 5. **Memory Per Order**
- **What**: Peak heap size / total orders
- **Why**: Identifies memory scalability limits
- **Measure at**: 1M order scenario
- **Target**: < 200 bytes per order (Order struct + list node + map overhead)
- **Interpretation**: If memory bloats, overhead structures (orderIndex, etc.) may need review

---

## Benchmark Scenarios (Realistic Market Conditions)

Create generators for these scenarios to test under varied conditions:

### Scenario 1: **Steady Flow** (Baseline calm market)
- **Pattern**: Constant stream of orders with 1:1 buy/sell ratio
- **Market depth**: Random price levels (10-20 levels typically)
- **Cancellations**: Low (5%)
- **Purpose**: Proves baseline throughput under normal conditions
- **Example**: 100K orders, alternating buy/sell, continuous matching

### Scenario 2: **Layered Book** (Deep order book)
- **Pattern**: Many orders accumulated before matching spike
- **Market depth**: 1000+ price levels (deep book), then sudden matching
- **Cancellations**: Low (5%)
- **Purpose**: Tests scalability as book grows; validates O(log N) insertion, O(1) lookup
- **Example**: Add 100K orders to book at varied prices, then add 10K matching orders

### Scenario 3: **Cancel-Heavy** (High order modification rate)
- **Pattern**: Orders added, then cancelled/modified before matching
- **Cancellations**: High (70%+ of orders cancelled)
- **Market depth**: Moderate (100-500 levels)
- **Purpose**: Validates O(1) cancellation and modify-via-cancel-reinsert path
- **Example**: Add 1K orders, cancel 700, add 1K more, repeat

### Scenario 4: **Mixed Operations** (Realistic blend)
- **Pattern**: Weighted mix of add, cancel, modify, market orders
- **Weights**: 60% limit adds, 20% cancellations, 10% modifications, 10% market orders
- **Market depth**: Varies (50-500 levels)
- **Purpose**: Most realistic scenario; catches unexpected interactions
- **Example**: 100K orders with mixed operations per above weights

### Scenario 5: **Flash Crash** (Stress test)
- **Pattern**: Sudden imbalance (all buy orders, then all sell orders)
- **Market depth**: Grows then shrinks rapidly
- **Purpose**: Tests edge cases (market orders exhausting liquidity, rapid state changes)
- **Example**: 50K buy orders, then 50K sell orders matching against them

---

## Implementation Path

### Phase 1: Infrastructure Setup (1-2 hours)
1. **Add Google Benchmark dependency** to CMakeLists.txt
   - Reuse existing GoogleTest infrastructure
   - Create separate `order_book_benchmarks` target
   
2. **Create `/benchmarks/` directory structure**
   ```
   benchmarks/
   ├── CMakeLists.txt              # Benchmark-specific build config
   ├── micro/                       # Unit operation benchmarks
   │   ├── insertion_bench.cpp
   │   ├── cancellation_bench.cpp
   │   └── lookup_bench.cpp
   ├── integration/                 # Full engine scenarios
   │   └── engine_scenarios_bench.cpp
   ├── fixtures/                    # Shared benchmark utilities
   │   ├── scenario_generators.h
   │   └── benchmark_helpers.h
   └── README.md                    # How to run, interpret results
   ```

3. **Create benchmark utilities** (`benchmarks/fixtures/`)
   - `ScenarioGenerator`: Base class for scenario builders
   - Helpers: `createRandomBook()`, `createLayeredBook()`, etc.
   - Logging control: Benchmark versions with `LOG_LEVEL=NONE` and `LOG_LEVEL=INFO`

### Phase 2: Micro-Benchmarks (2-3 hours)
1. **Insertion Benchmark** (`micro/insertion_bench.cpp`)
   - Fixture: Pre-populate book to N levels
   - Benchmark: `bench_insertion_empty_book`, `bench_insertion_1k`, `bench_insertion_1m`, etc.
   - Use Google Benchmark ranges: `->RangeMultiplier(10)->Range(100, 1000000)`

2. **Cancellation Benchmark** (`micro/cancellation_bench.cpp`)
   - Fixture: Pre-populate book, track order IDs
   - Benchmark: `bench_cancellation_empty_book`, `bench_cancellation_1k`, etc.
   - **Critical**: Measure latency variability (should be flat, not increasing)

3. **Lookup Benchmark** (`micro/lookup_bench.cpp`)
   - Fixture: Pre-populate with varying depths
   - Benchmark: `bench_best_bid_lookup`, `bench_best_ask_lookup` at 1K, 10K, 1M
   - Note: Expected to be near-zero (< 100 ns)

### Phase 3: Integration Benchmarks (3-4 hours)
1. **Create scenario generators** (`fixtures/scenario_generators.h`)
   - Base class with virtual methods for scenario parameters
   - Implementations: `SteadyFlowScenario`, `LayeredBookScenario`, `CancelHeavyScenario`, etc.
   
2. **Engine throughput benchmark** (`integration/engine_scenarios_bench.cpp`)
   - Benchmark per scenario: `bench_steady_flow_10k`, `bench_steady_flow_100k`, etc.
   - Measure: wall-clock time, throughput (orders/sec), trade count validation
   - Include optional memory tracking (peak RSS)

### Phase 4: Baseline Establishment (1-2 hours)
1. **Run all benchmarks** on your hardware (with optimizations enabled: `-O3`)
2. **Capture system info**:
   ```bash
   lscpu > benchmark_baseline_hardware.txt
   uname -a >> benchmark_baseline_hardware.txt
   ```
3. **Create `BENCHMARK_BASELINE.md`** documenting:
   - CPU model, cache sizes
   - Compiler version, C++ standard
   - Build flags (`-O3`, `-march=native`, etc.)
   - Baseline numbers:
     * Insertion latency at 100, 1K, 1M orders
     * Cancellation latency at same depths
     * Lookup latency
     * Throughput by scenario (orders/sec)
   - Trade count validation (proof of correctness)

### Phase 5: Analysis & Documentation (1-2 hours)
1. **Create comparison script** (Python/bash)
   - Compare current run vs. baseline
   - Show % improvement/regression per metric
   - Flag any regressions > 5%

2. **Document in `benchmarks/README.md`**:
   - How to run each benchmark
   - How to interpret results
   - Expected ranges and what they mean
   - How to compare against baseline

---

## Two-Level Benchmarking Strategy

### Micro-Benchmarks
**What**: Isolated single operations
**Tool**: Google Benchmark with fixtures
**Pros**:
- Pinpoints exact bottleneck (e.g., is it map insertion or orderIndex update?)
- Repeatable, reproducible
- Validates complexity claims (O(1), O(log N))

**Cons**:
- Doesn't reflect real-world cache behavior
- Overhead of micro-benchmark harness may matter

**When to use**: Validating complexity claims; debugging performance regression

### Integration/Macro Benchmarks
**What**: Full `MatchingEngine` processing realistic order sequences
**Tool**: Google Benchmark with scenario generators
**Pros**:
- Real-world performance (cache behavior, allocator patterns)
- Catches unexpected interactions (e.g., logging impact)
- Comparable to "orders/second" marketing claims

**Cons**:
- Harder to isolate root cause
- Variance from OS scheduling, cache states

**When to use**: Proving end-to-end throughput; benchmarking against requirements

**Recommendation**: Use both. Start with integration benchmarks to establish overall throughput. If result is disappointing, dive into micro-benchmarks to find bottleneck.

---

## Critical Decision: Logging Impact

Your current code logs during operations. **This affects all benchmarks.**

### Approach 1: Benchmark Without Logging (Raw Engine Speed)
```cpp
// Compile with #define LOG_LEVEL=NONE
```
**Pros**: Measures true engine performance
**Cons**: Not representative of real deployments

### Approach 2: Benchmark With Logging (Real-World Impact)
```cpp
// Compile with #define LOG_LEVEL=INFO
```
**Pros**: Real-world representative
**Cons**: Logger overhead obscures engine bottlenecks

### Recommendation: **Do Both**
1. Run all benchmarks twice: with logging disabled, then with INFO logging
2. Document the difference as "logger overhead"
3. Example result:
   - Insertion (no logging): 2 μs
   - Insertion (with INFO): 5 μs
   - **Logger overhead: 3 μs per insertion**

This is portfolio gold: "I measured and documented that logging adds 60% overhead under typical load."

---

## Baseline Documentation Template

Create `BENCHMARK_BASELINE.md` with this structure:

```markdown
# Benchmark Baseline — [Date] on [Hostname]

## Hardware & Build Environment
- CPU: [model, cores, cache]
- Compiler: GCC/Clang [version] with `-O3 -march=native`
- C++ Standard: C++20
- Google Benchmark version: [version]

## Baseline Metrics (No Logging)

### Insertion Latency
| Book Depth | Latency (μs) | Notes |
|-----------|-------------|-------|
| 100       | 0.8         | Empty to 100 orders |
| 1K        | 1.2         | Linear growth expected |
| 10K       | 1.5         | O(log 10K) ≈ 13 ops |
| 1M        | 2.1         | O(log 1M) ≈ 20 ops |

### Cancellation Latency (Validates O(1))
| Book Depth | Latency (μs) | Variance | Notes |
|-----------|-------------|----------|-------|
| 100       | 0.5         | ±0.05    | Should be flat |
| 1K        | 0.51        | ±0.06    | Proves O(1) |
| 1M        | 0.52        | ±0.05    | Constant! |

### Best Bid/Ask Lookup
| Book Depth | Latency (ns) | Notes |
|-----------|------------|-------|
| 1K        | 45         | Near-constant |
| 1M        | 48         | Validates O(1) |

### Throughput (Orders/Second)
| Scenario | 10K Orders | 100K Orders | 1M Orders |
|----------|-----------|------------|----------|
| Steady Flow | 850K ops/s | 820K ops/s | 810K ops/s |
| Layered Book | 720K ops/s | 710K ops/s | 700K ops/s |
| Cancel-Heavy | 650K ops/s | 640K ops/s | 630K ops/s |

## Baseline Metrics (With INFO Logging)
[Same table structure, expect 30-40% overhead]

## Logger Overhead
- Insertion: +3.5 μs (80% overhead)
- Cancellation: +0.3 μs (60% overhead)
- Overall throughput: -35% under mixed load

## Key Observations
- O(1) cancellation validated: latency within 5% across all depths
- O(log N) insertion confirmed: 2.6x difference from 100 to 1M orders
- Logger is significant cost: consider async logging for production
```

---

## How to Run Benchmarks

### Initial Setup
```bash
# Build with benchmarking support
cmake -B build -S . -DBUILD_BENCHMARKS=ON
cmake --build build

# Or in CLion's cmake-build-debug
cd cmake-build-debug && make
```

### Run All Benchmarks
```bash
./build/order_book_benchmarks
```

### Run Specific Benchmark
```bash
./build/order_book_benchmarks --benchmark_filter="insertion"
```

### Compare Against Baseline
```bash
./build/order_book_benchmarks > current_run.json --benchmark_format=json
# Then use comparison script (TBD) to compare vs. BENCHMARK_BASELINE.md
```

---

## Interpreting Results

### Insertion Latency Curve
- **Expected**: Logarithmic growth (flat-ish)
- **Bad**: Linear growth → orderIndex not working efficiently
- **Good**: Growth factor 2-3x from 100 to 1M orders

### Cancellation Latency
- **Expected**: Flat line across all depths (< 5% variance)
- **Bad**: Upward trend → O(1) claim is false
- **Good**: All measurements within 10% of each other

### Throughput
- **Expected**: Decreases slightly as book grows (cache misses)
- **Bad**: Throughput drops > 20% as book depth increases
- **Good**: Steady-flow scenario > 500K orders/sec; cancel-heavy > 300K orders/sec

### Logger Overhead
- **Expected**: 30-50% throughput reduction with logging
- **Good**: Identifies logging as optimization target for Phase 4

---

## Portfolio/Interview Value

### What This Shows Employers
1. **Rigor**: "I measured before claiming performance"
2. **Systems thinking**: "I understood where the bottlenecks were"
3. **Quant dev credibility**: "I can prove my engine handles 100k orders/sec"
4. **Maturity**: "I tracked variance and edge cases"

### Interview-Ready Answers
- "What's the latency of your order insertion?" → [Show curve]
- "Can you prove O(1) cancellation?" → [Show flat latency across depths]
- "How many orders/second can you handle?" → [Show throughput by scenario]
- "Where's your bottleneck?" → [Show logger vs. engine breakdown]
- "What would you optimize first?" → [Data-driven answer with priority]

### Resume Bullet Points
- Established rigorous performance baseline for order matching engine; measured insertion, cancellation, and lookup latencies across 100–1M order depths
- Validated O(1) cancellation complexity claim via instrumented benchmarks; documented logger overhead (30-50%) separately
- Designed realistic market scenario generators (steady flow, deep book, cancel-heavy, flash crash) for stress testing

---

## Timing & Effort Estimate

| Phase | Time | Effort |
|-------|------|--------|
| Phase 1: Infrastructure | 1-2 hrs | Add Google Benchmark, create directory structure |
| Phase 2: Micro-benchmarks | 2-3 hrs | Write 3 benchmark files (insertion, cancel, lookup) |
| Phase 3: Integration benchmarks | 3-4 hrs | Scenario generators, engine throughput benchmarks |
| Phase 4: Baseline | 1-2 hrs | Run, capture hardware info, document numbers |
| Phase 5: Analysis & docs | 1-2 hrs | Create comparison script, README |
| **Total** | **8-13 hrs** | **One solid weekend** |

---

## Next Steps

1. **Add Google Benchmark to CMakeLists.txt** (skip if already present)
2. **Create `/benchmarks/` directory** with subdirectories
3. **Implement scenario generators** (`fixtures/scenario_generators.h`)
4. **Write micro-benchmarks** (insertion, cancellation, lookup)
5. **Write integration benchmarks** (steady flow, layered, cancel-heavy, mixed, flash crash)
6. **Run on your hardware** and capture baseline
7. **Document in BENCHMARK_BASELINE.md**
8. **Create comparison script** for regression detection
9. **Update main README.md** with link to benchmark results and methodology

---

## References

- **Google Benchmark docs**: https://github.com/google/benchmark
- **AGENTS.md Phase 1**: See project guide for context on what we're validating
- **CMake integration**: Your existing CMakeLists.txt structure (reuse GoogleTest pattern)

---

*This guide is a living document. Update BENCHMARK_BASELINE.md after each optimization to track improvement. Include commit messages that reference benchmark results ("Reduce insertion latency by 15% via cache locality fix"; "Logger overhead now 20% vs. 35%").*

