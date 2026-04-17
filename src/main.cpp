#include "core/order.h"
#include "engine/matching_engine.h"
#include "utils/logger.h"

using namespace core;

int main() {
    engine::MatchingEngine engine;

    Order o1(1, 120, 15, Side::Buy);
    Order o2(2, 100, 5, Side::Sell);
    Order o3(3, 110, 25, Side::Sell);
    Order o4(4, 100, 50, Side::Buy);
    Order o5(5, 123, 28, Side::Buy);
    Order o6(6, 0, 47, Side::Sell, OrderType::Market);

    engine.processOrder(o1);
    engine.processOrder(o2);
    engine.processOrder(o3);
    engine.modifyOrder(o3.id, 12, 105);
    engine.processOrder(o4);
    engine.processOrder(o5);
    engine.processOrder(o6);

    utils::logger::log("Processed orders o1...o6");

    return 0;
}