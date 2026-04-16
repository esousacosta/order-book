#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(core::Order& order) const;

    private:
        core::OrderBook book;
        void tryToMatchBuyOrder(core::Order &receivedOrder) const;
        void tryToMatchSellOrder(core::Order &receivedOrder) const;
        void matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder) const;
        void handlePartiallyFilledOrder(const core::Order &receivedOrder) const;

    };
}