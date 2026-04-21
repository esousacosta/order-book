#include "benchmark_helpers.h"

namespace benchmark::utils {
    void populateRandomBook(core::OrderBook &book, size_t numOrders, core::Price priceStart, core::Price priceEnd, core::Quantity qtyMin, core::Quantity qtyMax) {
        for (size_t i = 0; i < numOrders; ++i) {
            const core::Price price = priceStart + rand() % (priceEnd - priceStart + 1);
            const core::Quantity qty = qtyMin + rand() % (qtyMax - qtyMin + 1);
            const core::Side side = (rand() % 2 == 0) ? core::Side::Buy : core::Side::Sell;
            book.addOrder(core::Order(i, price, qty, side));
        }
    }

    void populateLayeredBook(core::OrderBook &book, size_t layers, size_t ordersPerLayer, core::Price priceStart, core::Price priceStep, core::Quantity qtyStart, core::Quantity qtyStep) {
        for (size_t layer = 0; layer < layers; ++layer) {
            const core::Price price = priceStart + layer * priceStep;
            const core::Quantity qty = qtyStart + layer * qtyStep;
            for (size_t orderNum = 0; orderNum < ordersPerLayer; ++orderNum) {
                const core::Side side = (layer % 2 == 0) ? core::Side::Buy : core::Side::Sell;
                book.addOrder(core::Order(layer * ordersPerLayer + orderNum, price, qty, side));
            }
        }
    }
}
