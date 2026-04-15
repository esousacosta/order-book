//
// Created by edesousacosta on 4/15/26.
//

#pragma once

#include <cstdint>

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
}
