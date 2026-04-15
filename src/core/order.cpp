//
// Created by edesousacosta on 4/15/26.
//

#include "core/order.h"

namespace core {
    Order::Order(OrderId id, Price price, Quantity qty, Side side): id(id), price(price), qty(qty), side(side) {}
}