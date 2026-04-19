# AGENTS.md - Order Book Project Guide

## Architecture Overview

This is a **C++20 financial order matching engine** with three core layers:

1. **Core (`include/core/`, `src/core/`)**: Order book data structure and domain types
   - `OrderBook`: Price-level map with FIFO ordering within each level (using `std::map<Price, std::list<Order>>`)
   - Uses Pimpl pattern (pointer to implementation) to hide complexity
   - Index for O(1) order cancellation via `std::unordered_map<OrderId, iterator>`

2. **Engine (`include/engine/`, `src/engine/`)**: Matching logic and trade execution
   - `MatchingEngine`: Stateful processor that owns the OrderBook instance
   - Template-based matching leveraging `Comparator` and `GetBestOrderFunc` for Buy/Sell symmetry
   - Handles Limit and Market order types with different match semantics

3. **Utilities (`include/utils/`, `src/utils/`)**: Cross-cutting concerns
   - Logger: Provides formatted output using `std::format` (C++20 feature)

## Critical Design Patterns

### Order Book Structure
```cpp
// Bids: price descending (highest first) - std::greater<>
// Asks: price ascending (lowest first) - std::less<>
std::map<Price, OrderList, std::greater<>> bids;  // Buy side
std::map<Price, OrderList> asks;                   // Sell side
```
This ensures `bids.begin()` and `asks.begin()` always return best prices for matching.

### Matching Strategy
- **Buy orders** match against `getBestAskOrder()` using `std::less` (must accept lower or equal ask price)
- **Sell orders** match against `getBestBidOrder()` using `std::greater` (must accept higher or equal bid price)
- Template function `tryToMatchReceivedOrder` abstracts this symmetry to avoid duplication

### Order State Management
- Orders track `unfilledQty` (mutable) alongside original `qty` (immutable)
- `modifyOrder` may cancel and re-add an order (cancellation is O(1) due to orderIndex)
- Market orders auto-cancel remaining quantity if not fully filled; Limit orders remain in book

## Build & Development Commands

```bash
cmake -B build -S . && cmake --build build       # Standard build
cmake --build build -- VERBOSE=1                  # Debug output
cd cmake-build-debug && make                      # CLion build dir
./cmake-build-debug/order_book                    # Run executable
```

- **C++ Standard**: C++20 (for `std::format`, structured bindings, template features)
- **Build System**: CMake 3.16+
- **Header-Only Dependencies**: None currently required

## Common Workflows

### Adding a New Order Type
1. Extend `enum class OrderType` in `types.h`
2. Update matching logic in `MatchingEngine::tryToMatchReceivedOrder()` (line 47 has the pattern)
3. Update `Order` constructors in `order.cpp` if needed
4. Add logging in `matching_engine.cpp::matchOrders()` for visibility

### Debugging Order Flow
- Enable `log()` calls already present in `book.cpp` (addOrder, cancelOrder, modifyOrder)
- Logs in `matching_engine.cpp::processOrder()` and `matchOrders()` show full trade lifecycle
- Check test case in `main.cpp` as reference (orders o1-o6)

### Modifying Order Book Data Structure
- Only touch `OrderBook::Impl` struct (line 14-23 in `book.cpp`)
- Keep `orderIndex` in sync for O(1) cancellation
- Ensure best-price accessors (`getBestBidOrder`, `getBestAskOrder`) still work

## Code Style & Conventions

- **Namespaces**: `core::`, `engine::`, `utils::logger::`
- **Types**: Custom aliases in `types.h` (e.g., `using Price = std::int64_t`, not `double`)
- **Const Correctness**: Methods like `cancelOrder()` use const iterators where safe
- **Optional Returns**: Use `std::optional<std::reference_wrapper<Order>>` to avoid invalid references
- **Logging**: Use `std::format` with type conversion to string (e.g., `std::to_string(id)`)

## Key Files at a Glance

| File | Purpose | Key Insight |
|------|---------|------------|
| `types.h` | Domain types & enums | Single source of truth for Price, OrderId, Side, Trade structs |
| `book.h/cpp` | Order storage & retrieval | Pimpl hides dual-map architecture; use only public APIs |
| `order.h/cpp` | Order entity | Only `unfilledQty` mutates; `qty` and `price` are set at creation/modify |
| `matching_engine.h/cpp` | Trade execution | Template method reduces code duplication for Buy/Sell cases |
| `main.cpp` | Integration example | Shows realistic order sequence: basic adds, modify, market order |

## Testing Gaps & Future Considerations

- No unit test suite present (tests folder empty)
- No test build configuration in CMakeLists.txt
- Consider adding parametrized matching tests once order types expand
- Market order cancellation (line 103) returns silently—consider event/callback system

## Namespace Hierarchy

```
core::
  ├─ Order, OrderBook
  ├─ Price, Quantity, OrderId, Side, OrderType, Trade
  └─ Impl (private implementation detail)
engine::
  └─ MatchingEngine
utils::
  └─ logger::log()
```

Respect these boundaries; cross-namespace dependencies should be one-directional (engine → core → nothing).

---

# Portfolio Development Roadmap

*This section guides your journey to turn this exercise into a production-grade portfolio project that demonstrates quant dev capabilities.*

## Phase 1: Validate & Benchmark Current Implementation (Weeks 1-2)

**Why this matters**: Before optimizing, you need baselines and confidence that the core logic is bulletproof. Employers want to see rigor.

**Strategic milestones** (do NOT implement yet—analyze first):
- **Test coverage**: How would you verify the matching engine handles these edge cases?
  - Same price, multiple orders (FIFO correctness)
  - Partially filled then modified orders
  - Market orders with insufficient liquidity
  - Zero-quantity edge cases
- **Performance benchmarking**: What metrics would matter for a real exchange?
  - Order insertion latency (single order, 1M order book)
  - Cancellation latency (O(1) guarantee proof)
  - Best-bid/ask lookup latency
  - Throughput: orders/second with realistic market scenarios
- **Memory profiling**: Where would memory blow up?
  - Memory per order
  - Cache-line efficiency of dual-map structure
  - Index overhead vs. traversal speed tradeoff

**Questions to explore** (no code):
- Can your logger become a bottleneck? What would a realistic logging strategy look like?
- Is the Pimpl pattern hiding implementation details effectively, or could you expose data structure choices?

---

## Phase 2: Production-Grade Error Handling & Resilience (Weeks 3-4)

**Why this matters**: Real exchanges must survive bad inputs and failures gracefully. This separates hobby projects from production.

**Strategic areas** (think before coding):
- **Input validation**: What invariants must hold?
  - Negative prices/quantities?
  - OrderId collisions?
  - Order modification loops (modify same order repeatedly)?
- **State consistency**: When can your data structures diverge?
  - What happens if `orderIndex` gets out of sync with the maps?
  - How would you detect and recover from inconsistency?
  - Should OrderBook have an `invariant()` debug method?
- **Partial failure handling**: Market orders that can't fill—silent return or explicit feedback?
  - Consider: What should a real matching engine return to the client?
  - Trade confirmation vs. rejection vs. partial acceptance

**Questions to explore**:
- Should OrderBook methods throw exceptions or return error codes?
- How would a feed handler (consumer of MatchingEngine) know what happened?

---

## Phase 3: Advanced Market Microstructure (Weeks 5-7)

**Why this matters**: This is what separates toy projects from "I understand real markets." Quant funds care about this.

**New order types to design for** (research, don't code):
- **Stop orders**: At what price do they activate? How do you avoid race conditions?
- **Iceberg orders**: Visible vs. hidden qty—how would your data structure represent this?
- **Pegged orders**: Orders that adjust to best bid/ask—what's the interaction with modifications?
- **Fill-or-Kill (FOK)**: Must fill entirely or reject—where does this logic live? Engine or book?

**Market scenarios to think through**:
- **Liquidity provider vs. taker asymmetry**: Does your matching handle this?
- **Auctions/opening**: How would you batch match orders instead of continuous matching?
- **Market depth/book export**: Can you efficiently compute "top 10 bids/asks"?

**Questions to explore**:
- Which order types would you tackle first for maximum resume impact?
- How would adding a new order type change the matching template design?

---

## Phase 4: Performance Optimization & Scaling (Weeks 8-10)

**Why this matters**: Proving you can write fast C++ is a major quant dev differentiator.

**Areas to investigate** (analyze, don't optimize prematurely):
- **Data structure alternatives**: 
  - Could a custom skip-list or B-tree beat `std::map` for your access patterns?
  - What about separating hot data (best bid/ask) from cold data (deep book)?
  - Would `std::vector` + binary search be faster than `std::map` for immutable snapshots?
- **Compiler optimizations**: 
  - Inline hints for hot paths (matching loop)?
  - Profile-guided optimization (PGO) candidate?
- **SIMD opportunities**: Could vector matching (batch matching) exploit SIMD?
- **Allocation strategy**:
  - Object pools for frequently created/destroyed orders?
  - How often does reallocation hurt latency?

**Benchmarking setup** (before any code):
- Write synthetic market scenarios (steady flow, flash crash, layering)
- Establish baseline for current implementation
- Document CPU cycles, cache misses, allocations per scenario

---

## Phase 5: Real-World Resilience & Observability (Weeks 11-13)

**Why this matters**: Production systems don't just work—they survive, report, and recover.

**Questions to answer**:
- **Crash recovery**: If your engine crashes, how do you restore order book state?
  - Should you persist a trade journal? Order snapshots?
- **Audit trail**: Can you replay all trades from the beginning?
- **Metrics & observability**:
  - Order-to-trade latency histogram?
  - Fill rates by price level?
  - Rejected orders and why?
- **Stress testing**:
  - Can you handle 100k orders/second?
  - What happens when the book becomes very deep (millions of orders)?
  - Memory limits?

**Integration angle**:
- How would you expose this engine to a live market feed (websocket, FIX protocol)?
- What interface does a risk manager need? (position limits, notional exposure, etc.)

---

## Phase 6: Algorithm Variations & Competitive Features (Weeks 14-16)

**Why this matters**: Demonstrates depth of understanding. Real exchanges support niche strategies.

**Research topics** (no code—understand the trade-offs):
- **Matching algorithms**:
  - Price-time priority (current)
  - Pro-rata allocation (commodity futures)
  - Call auction (opening session)
  - Which would be easiest to add? Hardest?
- **Latency-critical optimizations**:
  - Can you guarantee sub-microsecond best-price lookup?
  - Lock-free data structures for multi-threaded matching?
- **Regulatory features**:
  - Do you handle order size thresholds?
  - Position limits?
  - Wash-trade detection?

---

## Phase 7: Documentation & Presentation (Weeks 17-18)

**Why this matters**: Portfolio projects are only as good as their explanation. Hiring managers spend 5 minutes reading your README.

**Create**:
- **Architecture deep-dive**: Explain why you chose `std::map<Price, std::list<Order>>` over alternatives
- **Design decisions doc**: Why Pimpl? Why templates? What's the thread-safety story?
- **Performance analysis**: Latency/throughput numbers with realistic workloads
- **Walkthrough example**: Step through a realistic multi-order scenario (like main.cpp but explained)
- **Known limitations & future work**: Shows maturity (e.g., "no multi-threading support; would use lock-free queues")

**Code showcase**:
- Can someone read your matching template and understand the symmetry trick?
- Are edge cases (market order cancellation, partial fills) clearly handled?

---

## Recommended Execution Order by Resume Impact

1. **Start here**: Phase 1 + Phase 2 (Weeks 1-4)
   - Shows rigor and production thinking
   - Prevents embarrassing bugs in interviews
   
2. **Then**: Phase 3 (Weeks 5-7)
   - Demonstrates market knowledge (separates you from general SW engineers)
   
3. **Then**: Phase 4 (Weeks 8-10)
   - Proves you can write fast C++ (critical for quant)
   
4. **Finish with**: Phase 5 + Phase 7 (Weeks 11-18)
   - Real-world thinking + excellent communication

**Skip Phase 6 initially** unless you have a specific market microstructure expertise to showcase.

---

## Interview-Ready Questions You Should Be Able to Answer

Prepare to explain these (great indicators of deep understanding):

- "Why did you choose `std::greater<>` for bids but `std::less<>` for asks?"
- "What's the time complexity of each operation? Can you prove the O(1) cancellation?"
- "Walk me through a buy order matching against multiple sell orders at different prices."
- "How would you add support for [new order type]? What breaks in your design?"
- "Your market orders silently cancel unfilled qty—is that realistic? What would you change?"
- "How would you extend this to support multiple symbols/currencies?"
- "Can this handle 100k orders/second? How do you know?"

---

## Mentor's Notes

- **Don't skip validation**: Real codebases spend 40% of code on error handling. Test-driven development forces this.
- **Measure before optimizing**: Pick one scenario (e.g., matching 1k orders), run it 100x, record time. Change something. Re-run. Only improvements supported by data matter.
- **Explain your tradeoffs**: "I chose X over Y because..." is more impressive than "I used the STL container that seemed right."
- **Treat this like a real codebase**: Code reviews, git commit messages, refactoring old decisions—show evolution.
- **Benchmark against naive implementation**: Keep a simple version around. Prove your optimization actually wins.

This roadmap spans ~18 weeks at 5-10 hours/week. A portfolio-grade project by week 18 is your goal.

