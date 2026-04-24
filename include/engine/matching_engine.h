#pragma once

#include "../core/book.h"
#include "core/types.h"

namespace engine {
class MatchingEngine {
public:
  MatchingEngine() = default;
  MatchingEngine(core::OrderBook &&book_) : book(std::move(book_)) {}
  MatchingEngine(core::OrderBook &book_) : book(book_) {}
  void processOrder(core::Order &order);
  void modifyOrder(core::OrderId orderId, core::Quantity newQty,
                   core::Price newPrice);
  bool bookHasBids() const { return book.hasBids(); }
  bool bookHasAsks() const { return book.hasAsks(); }
  std::optional<std::reference_wrapper<core::Order>> getBestBid() const {
    return book.getBestBidOrder();
  }
  std::optional<std::reference_wrapper<core::Order>> getBestAsk() const {
    return book.getBestAskOrder();
  }
  core::Trades getTrades() const { return trades; }

private:
  core::OrderBook book;
  core::Trades trades;
  core::Trade matchOrders(core::Order &freshOrder,
                          core::Order &bestExistingOrder);
  void handlePartiallyFilledOrder(const core::Order &receivedOrder);
  template <typename Comparator, typename GetBestOrderFunc>
  void tryToMatchReceivedOrder(core::Order &receivedOrder,
                               GetBestOrderFunc getBestOrder,
                               Comparator comparePrices);
};
} // namespace engine