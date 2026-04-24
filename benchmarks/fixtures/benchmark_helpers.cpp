
#include "benchmark_helpers.h"
#include <random>

namespace benchmark::utils {

BenchConfig cfg(const benchmark::State &state) {
  // Args:
  // arg0: initial book depth (number of orders)
  // arg1: batch size for insertions (number of orders to insert per iteration)
  // arg2: random seed for book population
  return BenchConfig{.initialDepth = static_cast<uint64_t>(state.range(0)),
                     .opsBatchSize = static_cast<uint64_t>(state.range(1)),
                     .seed = static_cast<uint64_t>(state.range(2))};
}

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

void populateRandomBook(core::OrderBook &book, size_t numOrders,
                        core::Price priceStart, core::Price priceEnd,
                        core::Quantity qtyMin, core::Quantity qtyMax) {
  for (size_t i = 0; i < numOrders; ++i) {
    const core::Price price = priceStart + rand() % (priceEnd - priceStart + 1);
    const core::Quantity qty = qtyMin + rand() % (qtyMax - qtyMin + 1);
    const core::Side side =
        (rand() % 2 == 0) ? core::Side::Buy : core::Side::Sell;
    book.addOrder(core::Order(i, price, qty, side));
  }
}

void populateLayeredBook(core::OrderBook &book, size_t layers,
                         size_t ordersPerLayer, core::Price priceStart,
                         core::Price priceStep, core::Quantity qtyStart,
                         core::Quantity qtyStep) {
  for (size_t layer = 0; layer < layers; ++layer) {
    const core::Price price = priceStart + layer * priceStep;
    const core::Quantity qty = qtyStart + layer * qtyStep;
    for (size_t orderNum = 0; orderNum < ordersPerLayer; ++orderNum) {
      const core::Side side =
          (layer % 2 == 0) ? core::Side::Buy : core::Side::Sell;
      book.addOrder(
          core::Order(layer * ordersPerLayer + orderNum, price, qty, side));
    }
  }
}
} // namespace benchmark::utils
