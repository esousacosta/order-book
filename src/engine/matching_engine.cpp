#include "engine/matching_engine.h"

#include "utils/logger.h"

namespace engine {
    void MatchingEngine::processOrder(core::Order &order) const {

        switch (order.side) {
            case core::Side::Buy:
                tryToMatchBuyOrder(order);
                break;
            case core::Side::Sell:
                tryToMatchSellOrder(order);
                break;
        }
    }

    void MatchingEngine::tryToMatchBuyOrder(core::Order &order) const {
        const auto bestMatchOrderOpt = book.getBestAskOrder();
        if (!bestMatchOrderOpt.has_value()) {
            book.addOrder(order);
            return;
        }

        core::Quantity unfilledQty = order.qty;
        while (unfilledQty > 0) {
            if (!book.getBestAskOrder().has_value()) break;

            auto &bestAskOrder = book.getBestAskOrder()->get();
            if (bestAskOrder.price > order.price) break;
            const core::Quantity tradeQty = std::min(unfilledQty, bestAskOrder.unfilledQty);
            unfilledQty -= tradeQty;
            utils::logger::log("Matched buy order " + std::to_string(order.id) + \
                " with open sell order " + std::to_string(bestAskOrder.id) + \
                " for quantity " + std::to_string(tradeQty) + \
                " at price " + std::to_string(bestAskOrder.price));
            // In a real system, we would also record the trade here
            if (tradeQty == bestAskOrder.unfilledQty) {
                utils::logger::log("Fully filled sell order " + std::to_string(bestAskOrder.id) + ", removing from book");
                book.cancelOrder(bestAskOrder.id);
            } else {
                utils::logger::log("Partially filled sell order " + std::to_string(bestAskOrder.id) + ", remaining unfilled quantity: " + std::to_string(bestAskOrder.unfilledQty - tradeQty));
                bestAskOrder.unfilledQty -= tradeQty;
            }
        }

        if (unfilledQty > 0) {
            utils::logger::log("Unfilled quantity for order " + std::to_string(order.id) + ": " + std::to_string(unfilledQty));
            order.unfilledQty = unfilledQty;
            book.addOrder(order);
        }
    }

    void MatchingEngine::tryToMatchSellOrder(core::Order &order) const {
        const auto bestMatchOrderOpt = book.getBestBidOrder();
        if (!bestMatchOrderOpt.has_value()) {
            book.addOrder(order);
            return;
        }

        core::Quantity unfilledQty = order.qty;
        while (unfilledQty > 0) {

            if (!book.getBestBidOrder().has_value()) break;

            auto &bestBidOrder = book.getBestBidOrder()->get();
            if (bestBidOrder.price < order.price) break;
            const core::Quantity tradeQty = std::min(unfilledQty, bestBidOrder.unfilledQty);
            unfilledQty -= tradeQty;
            utils::logger::log("Matched sell order " + std::to_string(order.id) + \
                " with open buy order " + std::to_string(bestBidOrder.id) + \
                " for quantity " + std::to_string(tradeQty) + \
                " at price " + std::to_string(bestBidOrder.price));
            // In a real system, we would also record the trade here
            if (tradeQty == bestBidOrder.unfilledQty) {
                utils::logger::log("Fully filled buy order " + std::to_string(bestBidOrder.id) + ", removing from book");
                book.cancelOrder(bestBidOrder.id);
            } else {
                utils::logger::log("Partially filled buy order " + std::to_string(bestBidOrder.id) + ", remaining unfilled quantity: " + std::to_string(bestBidOrder.unfilledQty - tradeQty));
                bestBidOrder.unfilledQty -= tradeQty;
            }
        }

        if (unfilledQty > 0) {
            utils::logger::log("Unfilled quantity for order " + std::to_string(order.id) + ": " + std::to_string(unfilledQty));
            order.unfilledQty = unfilledQty;
            book.addOrder(order);
        }
    }
}
