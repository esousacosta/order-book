#include "engine/matching_engine.h"

namespace engine {
    void MatchingEngine::processOrder(const core::Order& order) const {
        book.addOrder(order);
    }
}