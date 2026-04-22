#include <benchmark/benchmark.h>

#include "../fixtures/benchmark_helpers.h"
#include "core/book.h"

static void BM_insertion_random_1KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 1000, 100, 10000, 1, 1000);

    constexpr core::OrderId orderId{1001};
    constexpr core::Price price{5000};
    constexpr core::Quantity qty{100};
    constexpr auto side{core::Side::Buy};

    for (auto _ : state) {
        book.addOrder(core::Order(orderId, price, qty, side));
    }
}

static void BM_insertion_random_10KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 10000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{10001};
    constexpr core::Price price{5000};
    constexpr core::Quantity qty{100};
    constexpr auto side{core::Side::Buy};
    for (auto _ : state) {
        book.addOrder(core::Order(orderId, price, qty, side));
    }
}

static void BM_insertion_random_100KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 100000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{100001};
    constexpr core::Price price{5000};
    constexpr core::Quantity qty{100};
    constexpr auto side{core::Side::Buy};
    for (auto _ : state) {
        book.addOrder(core::Order(orderId, price, qty, side));
    }
}

static void BM_insertion_random_1MOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 1000000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{1000001};
    constexpr core::Price price{5000};
    constexpr core::Quantity qty{100};
    constexpr auto side{core::Side::Buy};
    for (auto _ : state) {
        book.addOrder(core::Order(orderId, price, qty, side));
    }
}

static void BM_insertion_random_10MOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 10000000, 100, 10000, 1, 1000);
    constexpr core::OrderId orderId{10000001};
    constexpr core::Price price{5000};
    constexpr core::Quantity qty{100};
    constexpr auto side{core::Side::Buy};
    for (auto _ : state) {
        book.addOrder(core::Order(orderId, price, qty, side));
    }
}

BENCHMARK(BM_insertion_random_1KOrders)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_insertion_random_10KOrders)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_insertion_random_100KOrders)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_insertion_random_1MOrders)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_insertion_random_10MOrders)->Unit(benchmark::kMicrosecond);

// === Main Function ===
BENCHMARK_MAIN();