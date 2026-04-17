#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(core::Order& order) const;

    private:
        core::OrderBook book;
        void matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder) const;
        void handlePartiallyFilledOrder(const core::Order &receivedOrder) const;
        template <typename Comparator, typename GetBestOrderFunc>
        void tryToMatchReceivedOrder(core::Order &receivedOrder, GetBestOrderFunc getBestOrder, Comparator comparePrices) const;
    };
}