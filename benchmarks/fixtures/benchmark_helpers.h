#pragma once
#include "core/book.h"
#include "core/order.h"

namespace benchmark::utils {
    void populateRandomBook(core::OrderBook &book, size_t numOrders, core::Price priceStart, core::Price priceEnd,
                            core::Quantity qtyMin, core::Quantity qtyMax);
    void populateLayeredBook(core::OrderBook &book, size_t layers, size_t ordersPerLayer, core::Price priceStart,
                        core::Price priceStep, core::Quantity qtyStart, core::Quantity qtyStep);
}