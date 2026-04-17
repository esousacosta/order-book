//
// Created by edesousacosta on 4/15/26.
//

#pragma once

#include <cstdint>
#include <vector>

namespace core {
    using Price = std::int64_t;
    using Quantity = std::int64_t;
    using OrderId = std::uint64_t;

    enum class Side {
        Buy,
        Sell
    };

    enum class OrderType {
        Limit,
        Market
    };

    struct Trade {
        Price price;
        Quantity quantity;
        OrderId maker;
        OrderId taker;
    };

    using Trades = std::vector<Trade>;
}
