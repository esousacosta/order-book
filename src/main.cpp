#include "core/order.h"
#include "engine/matching_engine.h"
#include "utils/logger.h"

using namespace core;

int main() {
    engine::MatchingEngine engine;

    Order o1(1, 100, 10, Side::Buy);
    Order o2(2, 100, 5, Side::Sell);

    engine.processOrder(o1);
    engine.processOrder(o2);

    utils::logger::log("Processed orders o1 and o2");

    return 0;
}