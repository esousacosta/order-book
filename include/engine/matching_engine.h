#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(core::Order& order) const;

    private:
        core::OrderBook book;
        void tryToMatchBuyOrder(core::Order &order) const;
        void tryToMatchSellOrder(core::Order &order) const;
    };
}