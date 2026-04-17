#include "engine/matching_engine.h"

#include <format>

#include "utils/logger.h"

namespace engine {
    void MatchingEngine::processOrder(core::Order &order) const {

        switch (order.side) {
            case core::Side::Buy:
                // tryToMatchBuyOrder(order);
                tryToMatchReceivedOrder(order, &core::OrderBook::getBestAskOrder, std::less<core::Price>{});
                break;
            case core::Side::Sell:
                // tryToMatchSellOrder(order);
                tryToMatchReceivedOrder(order, &core::OrderBook::getBestBidOrder, std::greater<core::Price>{});
                break;
        }
    }

    template <typename Comparator, typename GetBestOrderFunc>
    void MatchingEngine::tryToMatchReceivedOrder(core::Order &receivedOrder, GetBestOrderFunc getBestOrder, Comparator comparePrices) const {
        if (const auto bestMatchOrderOpt = std::invoke(getBestOrder, book); !bestMatchOrderOpt.has_value()) {
            if (receivedOrder.type == core::OrderType::Limit) book.addOrder(receivedOrder);
            return;
        }

        while (receivedOrder.unfilledQty > 0) {
            auto bestBookOrderOpt = std::invoke(getBestOrder, book);
            if (!bestBookOrderOpt.has_value()) break;
            auto &bestBookOrder = bestBookOrderOpt->get();
            if (receivedOrder.type == core::OrderType::Limit && comparePrices(receivedOrder.price, bestBookOrder.price)) break;
            matchOrders(receivedOrder, bestBookOrder);
        }

        handlePartiallyFilledOrder(receivedOrder);
    }

    void MatchingEngine::matchOrders(core::Order &freshOrder, core::Order &bestExistingOrder) const {
        std::string receivedOrderSide;
        std::string bestExistingOrderSide;
        if (freshOrder.side == core::Side::Buy) {
            receivedOrderSide = "buy";
            bestExistingOrderSide = "sell";
        } else {
            receivedOrderSide = "sell";
            bestExistingOrderSide = "buy";
        }

        core::Price tradePrice;
        if (freshOrder.type == core::OrderType::Market) {
            tradePrice = bestExistingOrder.price;
        } else {
            tradePrice = std::min(freshOrder.price, bestExistingOrder.price);
        }

        const core::Quantity tradeQty = std::min(freshOrder.unfilledQty, bestExistingOrder.unfilledQty);
        freshOrder.unfilledQty -= tradeQty;

        utils::logger::log(std::format("Matched {} order {} with open {} order {}, for quantity {}, at price {}",
            receivedOrderSide, std::to_string(freshOrder.id), bestExistingOrderSide, std::to_string(bestExistingOrder.id), tradeQty, tradePrice));

            if (freshOrder.unfilledQty == 0) {
                utils::logger::log(std::format("Fully filled {} order {}.", receivedOrderSide, freshOrder.id));
            }
            // In a real system, we would also record the trade here
            if (tradeQty == bestExistingOrder.unfilledQty) {
                utils::logger::log(std::format("Fully filled {} order {}; removing from book", bestExistingOrderSide, bestExistingOrder.id));
                book.cancelOrder(bestExistingOrder.id);
            } else {
                utils::logger::log(std::format("Partially filled {} order {}, remaining unfilled quantity: {}", bestExistingOrderSide, bestExistingOrder.id, bestExistingOrder.unfilledQty - tradeQty));
                bestExistingOrder.unfilledQty -= tradeQty;
            }
    }

    void MatchingEngine::handlePartiallyFilledOrder(const core::Order &receivedOrder) const {
        if (receivedOrder.unfilledQty > 0) {
            utils::logger::log(std::format("Unfilled quantity for order {}: {}", std::to_string(receivedOrder.id), std::to_string(receivedOrder.unfilledQty)));
            if (receivedOrder.type == core::OrderType::Market) {
                utils::logger::log(std::format("Partially filled market order {}, remaining unfilled quantity: {}, canceling remaining quantity", std::to_string(receivedOrder.id), std::to_string(receivedOrder.unfilledQty)));
                return; // Market orders are canceled if not fully filled
            }
            book.addOrder(receivedOrder);
        }
    }
}
