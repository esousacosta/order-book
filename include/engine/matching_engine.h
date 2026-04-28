#pragma once

#include "../core/book.h"
#include "core/types.h"

namespace engine {
class MatchingEngine {
 public:
  MatchingEngine() = default;
  explicit MatchingEngine(core::OrderBook& externalBook)
      : book(&externalBook) {}

  core::Trades processOrder(core::Order& order);
  void modifyOrder(core::OrderId orderId,
                   core::Quantity newQty,
                   core::Price newPrice);

 private:
  core::OrderBook ownedBook;
  core::OrderBook* book = &ownedBook;

  core::Trade matchOrders(core::Order& freshOrder,
                          core::Order& bestExistingOrder);
  void handlePartiallyFilledOrder(const core::Order& receivedOrder);
  template <typename Comparator, typename GetBestOrderFunc>
  core::Trades tryToMatchReceivedOrder(core::Order& receivedOrder,
                                       GetBestOrderFunc getBestOrder,
                                       Comparator comparePrices);
};
}  // namespace engine