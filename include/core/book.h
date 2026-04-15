//
// Created by edesousacosta on 4/15/26.
//
#pragma once

#include "core/order.h"

namespace core {
    class OrderBook {
        public:
        OrderBook();
        ~OrderBook();
        void addOrder(const Order& order) const;
        void cancelOrder(OrderId id) const;

    private:
        struct Impl;
        Impl* impl = nullptr;
    };
}