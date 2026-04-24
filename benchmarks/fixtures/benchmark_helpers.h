#pragma once
#include "core/book.h"
#include "core/order.h"
#include <benchmark/benchmark.h>

namespace benchmark::utils {
void populateRandomBook(core::OrderBook &book, size_t numOrders,
                        core::Price priceStart, core::Price priceEnd,
                        core::Quantity qtyMin, core::Quantity qtyMax);
void populateLayeredBook(core::OrderBook &book, size_t layers,
                         size_t ordersPerLayer, core::Price priceStart,
                         core::Price priceStep, core::Quantity qtyStart,
                         core::Quantity qtyStep);
struct BenchConfig {
  uint64_t initialDepth;
  uint64_t opsBatchSize;
  uint64_t seed;
};
BenchConfig cfg(const benchmark::State &state);
void populateDeterministicBook(core::OrderBook &book, uint64_t depth,
                               core::Price priceStart, uint64_t width,
                               uint64_t seed);
} // namespace benchmark::utils