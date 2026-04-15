#include "core/book.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <list>

namespace core {
    using OrderList = std::list<Order>;

    struct OrderBook::Impl {
        using OrderList = std::list<Order>;

        // price -> orders (FIFO)
        std::map<Price, OrderList, std::greater<> > bids;
        std::map<Price, OrderList> asks;

        // order_id -> iterator (for O(1) cancel)
        std::unordered_map<OrderId, OrderList::iterator> orderIndex;
    };

    OrderBook::OrderBook() : impl(new Impl()) {
    }

    OrderBook::~OrderBook() {
        delete impl;
    }

    void OrderBook::addOrder(const Order &order) const {
        if (order.side == Side::Buy) {
            auto &level = impl->bids[order.price];
            level.push_back(order);
            const auto it = std::prev(level.end());
            impl->orderIndex[order.id] = it;
        } else {
            auto &level = impl->asks[order.price];
            level.push_back(order);
            impl->orderIndex[order.id] = std::prev(level.end());
        }
    }

    void OrderBook::cancelOrder(OrderId orderId) const {
        const auto it = impl->orderIndex.find(orderId);
        if (it == impl->orderIndex.end()) return;

        if (auto orderIt = it->second; orderIt->side == Side::Buy) {
            auto &level = impl->bids[orderIt->price];
            level.erase(orderIt);
            if (level.empty()) impl->bids.erase(orderIt->price);
        } else {
            auto &level = impl->asks[orderIt->price];
            level.erase(orderIt);
            if (level.empty()) impl->asks.erase(orderIt->price);
        }

        impl->orderIndex.erase(orderId);
    }
}
