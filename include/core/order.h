//
// Created by edesousacosta on 4/15/26.
//

#pragma once
#include "core/types.h"

namespace core {
    struct Order {
        OrderId id;
        Price price;
        Quantity qty;
        Side side;
        OrderType type = OrderType::Limit;
        Quantity unfilledQty;

        Order(OrderId id, Price price, Quantity qty, Side side);
    };
}