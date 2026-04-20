#include "engine/matching_engine.h"

#include <format>

#include "utils/logger.h"

namespace engine {
    void MatchingEngine::processOrder(core::Order &order) {
        LOG_DEBUG(std::format("----- Processing {} order {} for quantity {}, at price {} -----",
                                       order.side == core::Side::Buy ? "buy" : "sell", std::to_string(order.id),
                                       std::to_string(order.qty), std::to_string(order.price)));
        core::Trades trades;
        switch (order.side) {
            case core::Side::Buy:
                trades = tryToMatchReceivedOrder(order, &core::OrderBook::getBestAskOrder, std::less<core::Price>{});
                break;
            case core::Side::Sell:
                trades = tryToMatchReceivedOrder(order, &core::OrderBook::getBestBidOrder, std::greater<core::Price>{});
                break;
        }

        for (const auto &[price, quantity, maker, taker]: trades) {
            LOG_INFO(std::format(
                "--> Executed trade: price {}, quantity {}, maker order id {}, taker order id {}",
                price, quantity, maker, taker));
        }
    }

    void MatchingEngine::modifyOrder(core::OrderId orderId, core::Quantity newQty, core::Price newPrice) {
       book.modifyOrder(orderId, newQty, newPrice);
    }

    template<typename Comparator, typename GetBestOrderFn>
    core::Trades MatchingEngine::tryToMatchReceivedOrder(core::Order &receivedOrder, GetBestOrderFn getBestOrder,
                                                         Comparator comparePrices) {
        if (const auto bestMatchOrderOpt = std::invoke(getBestOrder, book); !bestMatchOrderOpt.has_value()) {
            if (receivedOrder.type == core::OrderType::Limit) book.addOrder(receivedOrder);
            return {};
        }

        core::Trades trades;

        while (receivedOrder.unfilledQty > 0) {
            auto bestBookOrderOpt = std::invoke(getBestOrder, book);
            if (!bestBookOrderOpt.has_value()) break;
            auto &bestBookOrder = bestBookOrderOpt->get();
            if (receivedOrder.type == core::OrderType::Limit && comparePrices(receivedOrder.price, bestBookOrder.price))
                break;
            trades.push_back(matchOrders(receivedOrder, bestBookOrder));
        }

        handlePartiallyFilledOrder(receivedOrder);
        return trades;
    }

    core::Trade MatchingEngine::matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder) {
        std::string receivedOrderSide;
        std::string bestExistingOrderSide;
        if (freshOrder.side == core::Side::Buy) {
            receivedOrderSide = "buy";
            bestExistingOrderSide = "sell";
        } else {
            receivedOrderSide = "sell";
            bestExistingOrderSide = "buy";
        }

        core::Price tradePrice = bestExistingOrder.price;

        const core::Quantity tradeQty = std::min(freshOrder.unfilledQty, bestExistingOrder.unfilledQty);
        freshOrder.unfilledQty -= tradeQty;

        LOG_DEBUG(std::format("Matched {} order {} with open {} order {}, for quantity {}, at price {}",
                                       receivedOrderSide, std::to_string(freshOrder.id), bestExistingOrderSide,
                                       std::to_string(bestExistingOrder.id), tradeQty, tradePrice));

        const core::Trade trade{tradePrice, tradeQty, bestExistingOrder.id, freshOrder.id};

        if (freshOrder.unfilledQty == 0) {
            LOG_INFO(std::format("Fully filled {} order {}.", receivedOrderSide, freshOrder.id));
        }
        // In a real system, we would also record the trade here
        if (tradeQty == bestExistingOrder.unfilledQty) {
            LOG_INFO(std::format("Fully filled {} order {}; removing from book", bestExistingOrderSide,
                                           bestExistingOrder.id));
            book.cancelOrder(bestExistingOrder.id);
        } else {
            LOG_DEBUG(std::format("Partially filled {} order {}, remaining unfilled quantity: {}",
                                           bestExistingOrderSide, bestExistingOrder.id,
                                           bestExistingOrder.unfilledQty - tradeQty));
            bestExistingOrder.unfilledQty -= tradeQty;
        }
        return trade;
    }

    void MatchingEngine::handlePartiallyFilledOrder(const core::Order &receivedOrder) {
        if (receivedOrder.unfilledQty > 0) {
            LOG_DEBUG(std::format("Unfilled quantity for order {}: {}", std::to_string(receivedOrder.id),
                                           std::to_string(receivedOrder.unfilledQty)));
            if (receivedOrder.type == core::OrderType::Market) {
                LOG_DEBUG(std::format(
                    "Partially filled market order {}, remaining unfilled quantity: {}, canceling remaining quantity",
                    std::to_string(receivedOrder.id), std::to_string(receivedOrder.unfilledQty)));
                return; // Market orders are canceled if not fully filled
            }
            book.addOrder(receivedOrder);
        }
    }
}
