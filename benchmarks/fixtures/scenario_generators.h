#pragma once
#include "../../include/engine/matching_engine.h"
#include "../../include/core/types.h"
namespace benchmark::utils {
struct ScenarioAction {
  enum class Type { Insert, Modify, Cancel } type;
  core::Order order; // For Insert and Modify
};

std::vector<ScenarioAction> generateSteadyFlowScenario(size_t totalOrders, uint64_t seed);
}  // namespace benchmarks::utils
