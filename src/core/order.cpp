//
// Created by edesousacosta on 4/15/26.
//

#include "core/order.h"

#include <algorithm>

namespace core {
    Order::Order(const OrderId id, const Price price, const Quantity qty, const Side side) : id(id), price(price),
        qty(qty),
        side(side), unfilledQty(qty) {
    }

    Order::Order(const OrderId id, const Price price, const Quantity qty, const Side side, const OrderType type) : Order(id, price, qty, side) {
        this->type = type;
    }
}
