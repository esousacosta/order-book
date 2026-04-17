#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(core::Order& order);
        void modifyOrder(core::OrderId orderId, core::Quantity newQty, core::Price newPrice);

    private:
        core::OrderBook book;
        core::Trade matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder);
        void handlePartiallyFilledOrder(const core::Order &receivedOrder);
        template <typename Comparator, typename GetBestOrderFunc>
        core::Trades tryToMatchReceivedOrder(core::Order &receivedOrder, GetBestOrderFunc getBestOrder, Comparator comparePrices);
    };
}