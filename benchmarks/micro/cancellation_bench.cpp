#include <benchmark/benchmark.h>

#include "../fixtures/benchmark_helpers.h"
#include "core/book.h"

static void BM_cancel_success_batched(benchmark::State &state) {
    const auto c = benchmark::utils::cfg(state);
    core::OrderBook book;
    core::Price basePrice = 1'000'000;
    benchmark::utils::populateDeterministicBook(book, c.initialDepth, basePrice, 1024, c.seed);

    const core::OrderId targetId = c.initialDepth / 2;
    const auto existingOrder = book.getOrder(targetId);
    if (!existingOrder) {
        state.SkipWithError("target order does not exist");
        return;
    }

    uint64_t cancelCount = 0;
    for (auto _ : state) {
        for (uint64_t i = 0; i < c.opsBatchSize; ++i) {
            book.cancelOrder(targetId);
            benchmark::DoNotOptimize(book);
            ++cancelCount;

            // Re-add the order for the next iteration
            state.PauseTiming();
            book.addOrder(*existingOrder);
            state.ResumeTiming();
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(static_cast<int64_t>(cancelCount));
    state.counters["ops"] = benchmark::Counter(cancelCount, benchmark::Counter::kIsRate);
    state.counters["batch_ops"] = benchmark::Counter(c.opsBatchSize);
}

BENCHMARK(BM_cancel_success_batched)
->Args({1000, 64, 42})
->Args({10000, 64, 42})
->Args({100000, 64, 42})
->Args({300000, 64, 42})
->Args({600000, 64, 42})
->Args({1000000, 64, 42})
->Args({2000000, 64, 42 })
->Args({5000000, 64, 42 });

// BENCHMARK(BM_cancellation_random_1KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_10KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_100KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_1MOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_10MOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_success)->RangeMultiplier(100)->Range(1, 1000000);