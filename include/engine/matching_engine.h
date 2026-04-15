#pragma once

#include "../core/book.h"

namespace engine {
    class MatchingEngine {
    public:
        void processOrder(const core::Order& order) const;

    private:
        core::OrderBook book;
    };
}