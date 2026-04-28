#include "./scenario_generators.h"

#include <random>

namespace benchmark::utils {
std::vector<ScenarioAction> generateSteadyFlowScenario(size_t totalOrders, uint64_t seed) {
  std::vector<ScenarioAction> actions;
  std::mt19937_64 rng(seed);
  std::uniform_int_distribution<int> sideDist(0, 1);
  std::uniform_int_distribution<int> priceDist(990000, 1010000);
  std::uniform_int_distribution<int> qtyDist(1, 100);

    for (size_t i = 0; i < totalOrders; ++i) {
        core::OrderId id = static_cast<core::OrderId>(i + 1);
        core::Price price = priceDist(rng);
        core::Quantity qty = qtyDist(rng);
        core::Side side = sideDist(rng) == 0 ? core::Side::Buy : core::Side::Sell;
        actions.push_back({ScenarioAction::Type::Insert, {id, price, qty, side}});
    }

  return actions; // Placeholder: Implement a realistic scenario generation logic here
}
} // namespace benchmarks::utils