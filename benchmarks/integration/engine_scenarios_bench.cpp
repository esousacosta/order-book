#include <benchmark/benchmark.h>
#include <iostream>

#include "../fixtures/benchmark_helpers.h"
#include "../fixtures/scenario_generators.h"
#include "core/book.h"
#include "engine/matching_engine.h"

template <typename ...Args>
static void BM_engine_steady_flow(benchmark::State &state, Args&&... args) {
  auto args_tuple = std::make_tuple(std::forward<Args>(args)...);
  if constexpr (sizeof...(Args) == 0) {
    std::cout << "Volume of orders not specified, using default of 1 million." << std::endl;
    args_tuple = std::make_tuple(1'000'000);
  }
  benchmark::utils::BenchConfig config = benchmark::utils::cfg(state);

  core::OrderBook book;
  engine::MatchingEngine matchingEngine{book};

  // Generate a realistic order flow scenario
  std::vector<benchmark::utils::ScenarioAction> actions  = benchmark::utils::generateSteadyFlowScenario(std::get<0>(args_tuple), config.seed);

  uint64_t processedOrders = 0;
  for (auto _ : state) {
    for (const auto& action : actions) {
      matchingEngine.processOrder(const_cast<core::Order&>(action.order));
      ++processedOrders;
    }
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(static_cast<int64_t>(processedOrders));
  state.counters["ops"] =
      benchmark::Counter(processedOrders, benchmark::Counter::kIsRate);
}

BENCHMARK_CAPTURE(BM_engine_steady_flow, 1000_deep_1M_orders, 1'000'000)
->Args({1000000, 64, 42});