#include "core/book.h"

#include <format>
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

    std::optional<Order> OrderBook::getOrder(OrderId orderId) const {
        const auto it = impl->orderIndex.find(orderId);
        if (it == impl->orderIndex.end()) return std::nullopt;
        return *(it->second);
    }

    void OrderBook::addOrder(const Order &order) {
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
        utils::logger::log(std::format("Added {} order {} to book for quantity {}, at price {}",
                                       order.side == Side::Buy ? "buy" : "sell", std::to_string(order.id),
                                       std::to_string(order.unfilledQty), std::to_string(order.price)));
    }

    void OrderBook::cancelOrder(OrderId orderId) const {
        const auto it = impl->orderIndex.find(orderId);
        if (it == impl->orderIndex.end()) return;

        if (const auto orderIt = it->second; orderIt->side == Side::Buy) {
            const auto price = orderIt->price;
            auto &level = impl->bids[price];
            utils::logger::log("Canceling buy order " + std::to_string(orderId) + " at price " + std::to_string(orderIt->price));
            level.erase(orderIt);
            if (level.empty()) impl->bids.erase(price);
        } else {
            const auto price = orderIt->price;
            auto &level = impl->asks[price];
            level.erase(orderIt);
            utils::logger::log("Canceling sell order " + std::to_string(orderId) + " at price " + std::to_string(orderIt->price));
            if (level.empty()) impl->asks.erase(price);
        }

        impl->orderIndex.erase(orderId);
    }

    void OrderBook::modifyOrder(OrderId orderId, Quantity newRemainingQty, Price newPrice) {
        const auto orderEntryIt = impl->orderIndex.find(orderId);
        // If it is end, return
        if (orderEntryIt == impl->orderIndex.end()) return;

        const auto& orderIt = orderEntryIt->second;
        if (auto& order = *orderIt; newPrice != orderIt->price || newRemainingQty > order.unfilledQty) {
            utils::logger::log(std::format("[MOD] Modifying order {}: new price {}, new quantity {}",
                                           std::to_string(orderId), std::to_string(newPrice), std::to_string(newRemainingQty)));
            cancelOrder(orderId);
            order.price = newPrice;
            order.unfilledQty = newRemainingQty;
            order.qty = newRemainingQty;
            addOrder(order);
        } else {
            utils::logger::log(std::format("[MOD] Modifying order {}: new quantity {} (price unchanged)",
                                           std::to_string(orderId), std::to_string(newRemainingQty)));
            order.unfilledQty = newRemainingQty;
        }
    }

    bool OrderBook::hasBids() const {
        return !impl->bids.empty();
    }

    bool OrderBook::hasAsks() const {
        return !impl->asks.empty();
    }

    size_t OrderBook::getAskCount() const {
        return impl->asks.size();
    }

    size_t OrderBook::getBidCount() const {
        return impl->bids.size();
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
