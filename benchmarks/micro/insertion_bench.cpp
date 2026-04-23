#include <benchmark/benchmark.h>
#include <cmath>
#include <cstdint>
#include <random>
#include <sys/wait.h>

#include "../fixtures/benchmark_helpers.h"
#include "core/book.h"
#include "core/types.h"

namespace {
struct BenchConfig {
  uint64_t initialDepth;
  uint64_t opsBatchSize;
  uint64_t seed;
};

inline BenchConfig cfg(const benchmark::State &state) {
  // Args:
  // arg0: initial book depth (number of orders)
  // arg1: batch size for insertions (number of orders to insert per iteration)
  // arg2: random seed for book population
  return BenchConfig{.initialDepth = static_cast<uint64_t>(state.range(0)),
                     .opsBatchSize = static_cast<uint64_t>(state.range(1)),
                     .seed = static_cast<uint64_t>(state.range(2))};
}
} // namespace

void populateDeterministicBook(core::OrderBook &book, uint64_t depth,
                               core::Price priceStart, uint64_t width,
                               uint64_t seed) {
  std::mt19937_64 rng(seed);
  std::uniform_int_distribution<uint64_t> priceDist(0, width - 1);
  std::uniform_int_distribution<uint64_t> qtyDist(1, 1000);
  std::uniform_int_distribution<int> sideDist(0, 1);

  for (uint64_t i = 0; i < depth; ++i) {
    core::OrderId orderId = static_cast<core::OrderId>(i);
    const core::Price price =
        priceStart + static_cast<core::Price>(priceDist(rng));
    const core::Quantity qty = static_cast<core::Quantity>(qtyDist(rng));
    const auto side = static_cast<core::Side>(sideDist(rng));
    core::Order order(orderId, price, qty, side);
    book.addOrder(order);
  }
}

// Benchmarks for insertion performance under various conditions
// ------------------- Insertion success: same level -------------------
static void
BM_insertion_success_same_price_level_batched(benchmark::State &state) {
  BenchConfig config = cfg(state);

  core::OrderBook book;
  const core::Price basePrice = 1'000'000;
  const uint64_t domainWidth =
      1024; // Price levels will be in [basePrice, basePrice + domainWidth)$
  populateDeterministicBook(book, config.initialDepth, basePrice, domainWidth,
                            config.seed);

  core::OrderId nextId{static_cast<uint64_t>(config.initialDepth)};
  uint64_t successCount = 0;
  constexpr core::Quantity qty{100};
  constexpr auto side{core::Side::Buy};

  for (auto _ : state) {
    for (uint64_t i = 0; i < config.opsBatchSize; ++i) {
      ++nextId;
      core::Order order(nextId, basePrice, qty,
                        side); // All orders have the same price level
      book.addOrder(order);
      benchmark::DoNotOptimize(order);
      ++successCount;
    }
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(successCount));
  state.counters["ops"] =
      benchmark::Counter(successCount, benchmark::Counter::kIsRate);
  state.counters["batch_ops"] = benchmark::Counter(config.opsBatchSize);
}

// ------------------- Insertion success: variable price levels (market style)
// -------------------
static void BM_insertion_success_variable_price_level_outside_domain(
    benchmark::State &state) {
  BenchConfig config = cfg(state);

  core::OrderBook book;
  const core::Price basePrice = 2'000'000;
  const uint64_t domainWidth = std::max<uint64_t>(
      1024, config.initialDepth); // Price levels will be in [basePrice,
                                  // basePrice + domainWidth)$
  populateDeterministicBook(book, config.initialDepth, basePrice, domainWidth,
                            config.seed);

  core::OrderId nextId{static_cast<core::OrderId>(config.initialDepth)};
  core::Price newPrice =
      basePrice +
      static_cast<core::Price>(
          domainWidth +
          1); // Start inserting at a new price level above the existing ones
  uint64_t successCount = 0;
  constexpr core::Quantity qty{100};
  constexpr auto side{core::Side::Buy};

  for (auto _ : state) {
    for (uint64_t i = 0; i < config.opsBatchSize; ++i) {
      ++nextId;
      core::Order order(nextId, newPrice++, qty,
                        side); // All orders have the same price level
      book.addOrder(order);
      benchmark::DoNotOptimize(order);
      ++successCount;
    }
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(successCount));
  state.counters["ops"] =
      benchmark::Counter(successCount, benchmark::Counter::kIsRate);
  state.counters["batch_ops"] = benchmark::Counter(config.opsBatchSize);
}

// ------------------- Insertion success: variable price levels (inside domain)
// -------------------
static void BM_insertion_success_variable_price_level_inside_domain(
    benchmark::State &state) {
  BenchConfig config = cfg(state);

  core::OrderBook book;
  const core::Price basePrice = 2'000'000;
  const uint64_t domainWidth = std::max<uint64_t>(
      1024, config.initialDepth); // Price levels will be in [basePrice,
                                  // basePrice + domainWidth)$
  populateDeterministicBook(book, config.initialDepth, basePrice, domainWidth,
                            config.seed);

  core::OrderId nextId{static_cast<core::OrderId>(config.initialDepth)};
  uint64_t successCount = 0;
  constexpr core::Quantity qty{100};
  constexpr auto side{core::Side::Buy};
  uint64_t seq = 0;

  for (auto _ : state) {
    for (uint64_t i = 0; i < config.opsBatchSize; ++i) {
      ++nextId;
      // Deterministic, bounded, variable offset (no RNG in timed loop)
      const uint64_t offset =
          (seq * 11400714819323198485ull + config.seed) % domainWidth;
      core::Price price = basePrice + static_cast<core::Price>(offset);
      ++seq;

      core::Order order(nextId, price, qty,
                        side); // All orders have the same price level
      book.addOrder(order);
      benchmark::DoNotOptimize(order);
      ++successCount;
    }
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(static_cast<int64_t>(successCount));
  state.counters["ops"] =
      benchmark::Counter(successCount, benchmark::Counter::kIsRate);
  state.counters["batch_ops"] = benchmark::Counter(config.opsBatchSize);
  state.counters["domain_width"] = benchmark::Counter(domainWidth);
}

BENCHMARK(BM_insertion_success_same_price_level_batched)
    ->Args({1000, 64, 42})
    ->Args({10000, 64, 42})
    ->Args({100000, 64, 42})
    ->Args({300000, 64, 42})
    ->Args({600000, 64, 42})
    ->Args({1000000, 64, 42})
    ->Args({2000000, 64, 42})
    ->Args({5000000, 64, 42})
    ->Args({10000000, 64, 42});

BENCHMARK(BM_insertion_success_variable_price_level_outside_domain)
    ->Args({1000, 64, 42})
    ->Args({10000, 64, 42})
    ->Args({100000, 64, 42})
    ->Args({300000, 64, 42})
    ->Args({600000, 64, 42})
    ->Args({1000000, 64, 42})
    ->Args({2000000, 64, 42})
    ->Args({5000000, 64, 42})
    ->Args({10000000, 64, 42});

BENCHMARK(BM_insertion_success_variable_price_level_inside_domain)
    ->Args({1000, 64, 42})
    ->Args({10000, 64, 42})
    ->Args({100000, 64, 42})
    ->Args({300000, 64, 42})
    ->Args({600000, 64, 42})
    ->Args({1000000, 64, 42})
    ->Args({2000000, 64, 42})
    ->Args({5000000, 64, 42})
    ->Args({10000000, 64, 42});