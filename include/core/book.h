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
        void addOrder(const Order& order) const;
        void cancelOrder(OrderId id) const;
        [[nodiscard]] std::optional<std::reference_wrapper<const Order>> getBestAskOrder() const;
        [[nodiscard]] std::optional<std::reference_wrapper<const Order>> getBestBidOrder() const;
        [[nodiscard]] bool hasBids() const;
        [[nodiscard]] bool hasAsks() const;

    private:
        struct Impl;
        Impl* impl = nullptr;
    };
}