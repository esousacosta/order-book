#include "core/book.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <list>

#include "utils/logger.h"

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
            utils::logger::log("Canceling buy order " + std::to_string(orderId) + " at price " + std::to_string(orderIt->price));
            level.erase(orderIt);
            if (level.empty()) impl->bids.erase(orderIt->price);
        } else {
            auto &level = impl->asks[orderIt->price];
            level.erase(orderIt);
            utils::logger::log("Canceling sell order " + std::to_string(orderId) + " at price " + std::to_string(orderIt->price));
            if (level.empty()) impl->asks.erase(orderIt->price);
        }

        impl->orderIndex.erase(orderId);
    }

    bool OrderBook::hasBids() const {
        return !impl->bids.empty();
    }

    bool OrderBook::hasAsks() const {
        return !impl->asks.empty();
    }

    std::optional<std::reference_wrapper<Order>> OrderBook::getBestAskOrder() const {
        if (!hasAsks()) return std::nullopt;
        auto &bestLevelList = impl->asks.begin()->second;
        return bestLevelList.empty() ? std::nullopt : std::make_optional(std::ref(bestLevelList.front()));
    }

    std::optional<std::reference_wrapper<Order>> OrderBook::getBestBidOrder() const {
        if (!hasBids()) return std::nullopt;
        auto &bestLevelList = impl->bids.begin()->second;
        return bestLevelList.empty() ? std::nullopt : std::make_optional(std::ref(bestLevelList.front()));
    }
}
