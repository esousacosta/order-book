#include <benchmark/benchmark.h>

#include "../fixtures/benchmark_helpers.h"
#include "core/book.h"

static void BM_insertion_random_1KOrders(benchmark::State &state) {
    core::OrderBook book;
    benchmark::utils::populateRandomBook(book, 1000, 100, 10000, 1, 1000);


    for (auto _ : state) {
        constexpr core::OrderId orderId{1001};
        constexpr core::Price price{5000};
        constexpr core::Quantity qty{100};
        constexpr auto side{core::Side::Buy};

        book.addOrder(core::Order(orderId, price, qty, side));
        book.cancelOrder(orderId);
    }
}

BENCHMARK(BM_insertion_random_1KOrders)->Unit(benchmark::kMicrosecond);

// === Main Function ===
BENCHMARK_MAIN();