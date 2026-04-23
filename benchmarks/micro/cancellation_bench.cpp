#include <benchmark/benchmark.h>

#include "../fixtures/benchmark_helpers.h"
#include "core/book.h"

static void BM_cancellation_random_1KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 1000, 100, 10000, 1, 1000);

    constexpr core::OrderId orderId{500};

    for (auto _ : state) {
        book.cancelOrder(orderId);
    }
}

static void BM_cancellation_random_10KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 10000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{5000};

    for (auto _ : state) {
        book.cancelOrder(orderId);
    }
}

static void BM_cancellation_random_100KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 100000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{50000};

    for (auto _ : state) {
        book.cancelOrder(orderId);
    }
}

static void BM_cancellation_random_1MOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 1000000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{500000};

    for (auto _ : state) {
        book.cancelOrder(orderId);
    }
}

static void BM_cancellation_random_10MOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 10000000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{5000000};

    for (auto _ : state) {
        book.cancelOrder(orderId);
    }
}

static void BM_cancellation_success(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, state.range(0), 1, 1024, 1, 1000);
    const core::OrderId orderId{static_cast<uint64_t>(state.range(0)) / 2};
    const auto existingOrder = book.getOrder(orderId);
    if (!existingOrder) {
        state.SkipWithError("target order does not exist");
        return;
    }

    for (auto _ : state) {
        book.cancelOrder(orderId);
        benchmark::ClobberMemory();

        // Re-add the order for the next iteration
        state.PauseTiming();
        book.addOrder(*existingOrder);
        state.ResumeTiming();
    }
}

// BENCHMARK(BM_cancellation_random_1KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_10KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_100KOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_1MOrders)->Unit(benchmark::kMicrosecond);
// BENCHMARK(BM_cancellation_random_10MOrders)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_cancellation_success)->RangeMultiplier(100)->Range(1, 1000000);