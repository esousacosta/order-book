//
// Created by edesousacosta on 4/15/26.
//
#pragma once

#include <functional>
#include <optional>

#include "core/order.h"

namespace core {
    class OrderBook {
    public:
        OrderBook();

        ~OrderBook();
        std::optional<Order> getOrder(OrderId orderId) const ;

        void addOrder(const Order &order);

        void cancelOrder(OrderId orderId) const;

        void modifyOrder(OrderId orderId, Quantity newRemainingQty, Price newPrice);

        std::optional<std::reference_wrapper<Order> > getBestAskOrder() const;

        std::optional<std::reference_wrapper<Order> > getBestBidOrder() const;

        bool hasBids() const;

        bool hasAsks() const;

        size_t getBidCount() const;

        size_t getAskCount() const;

    private:
        struct Impl;
        Impl *impl = nullptr;
    };
}
