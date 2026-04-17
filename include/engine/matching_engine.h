#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(core::Order& order) const;

    private:
        core::OrderBook book;
        core::Trade matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder) const;
        void handlePartiallyFilledOrder(const core::Order &receivedOrder) const;
        template <typename Comparator, typename GetBestOrderFunc>
        core::Trades tryToMatchReceivedOrder(core::Order &receivedOrder, GetBestOrderFunc getBestOrder, Comparator comparePrices) const;
    };
}