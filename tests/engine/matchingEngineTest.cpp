#include <gtest/gtest.h>

#include "core/book.h"
#include "engine/matching_engine.h"

using ::testing::Test;

class MatchingEngineTest : public Test {
protected:
  core::OrderBook book;
  engine::MatchingEngine engine{book};
};

TEST_F(MatchingEngineTest,
       processOrder_limitBuyOnEmptyBook_orderRestsInBidBookAndReturnsNoTrades) {
  core::Order buyOrder(1, 100, 40, core::Side::Buy);

  const auto trades = engine.processOrder(buyOrder);

  ASSERT_TRUE(trades.empty());
  ASSERT_TRUE(book.hasBids());
  ASSERT_FALSE(book.hasAsks());
  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buyOrder.id);
  EXPECT_EQ(bestBid->get().price, buyOrder.price);
  EXPECT_EQ(bestBid->get().unfilledQty, buyOrder.qty);
}

TEST_F(
    MatchingEngineTest,
    processOrder_limitSellOnEmptyBook_orderRestsInAskBookAndReturnsNoTrades) {
  core::Order sellOrder(1, 100, 40, core::Side::Sell);

  const auto trades = engine.processOrder(sellOrder);

  ASSERT_TRUE(trades.empty());
  ASSERT_TRUE(book.hasAsks());
  ASSERT_FALSE(book.hasBids());
  const auto bestAsk = book.getBestAskOrder();
  ASSERT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk->get().id, sellOrder.id);
  EXPECT_EQ(bestAsk->get().price, sellOrder.price);
  EXPECT_EQ(bestAsk->get().unfilledQty, sellOrder.qty);
}

TEST_F(MatchingEngineTest,
       processOrder_crossingLimitOrder_orderFullyMatchesAndLeavesBookEmpty) {
  core::Order restingSell(1, 100, 40, core::Side::Sell);
  core::Order incomingBuy(2, 100, 40, core::Side::Buy);
  engine.processOrder(restingSell);

  const auto trades = engine.processOrder(incomingBuy);

  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].price, 100);
  EXPECT_EQ(trades[0].quantity, 40);
  EXPECT_EQ(trades[0].maker, restingSell.id);
  EXPECT_EQ(trades[0].taker, incomingBuy.id);
  EXPECT_FALSE(book.hasAsks());
  EXPECT_FALSE(book.hasBids());
}

TEST_F(MatchingEngineTest,
       processOrder_partialFill_restsRemainingIncomingQuantityInBook) {
  core::Order restingSell(1, 100, 40, core::Side::Sell);
  core::Order incomingBuy(2, 100, 50, core::Side::Buy);
  engine.processOrder(restingSell);

  const auto trades = engine.processOrder(incomingBuy);

  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 40);
  EXPECT_FALSE(book.hasAsks());
  ASSERT_TRUE(book.hasBids());
  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, incomingBuy.id);
  EXPECT_EQ(bestBid->get().unfilledQty, 10);
}

TEST_F(MatchingEngineTest,
       processOrder_buySweep_matchesMultipleAskLevelsInPricePriority) {
  core::Order ask1(1, 100, 30, core::Side::Sell);
  core::Order ask2(2, 95, 20, core::Side::Sell);
  core::Order ask3(3, 90, 5, core::Side::Sell);
  core::Order incomingBuy(4, 100, 40, core::Side::Buy);
  engine.processOrder(ask1);
  engine.processOrder(ask2);
  engine.processOrder(ask3);

  const auto trades = engine.processOrder(incomingBuy);

  ASSERT_EQ(trades.size(), 3u);
  EXPECT_EQ(trades[0].price, 90);
  EXPECT_EQ(trades[1].price, 95);
  EXPECT_EQ(trades[2].price, 100);
  ASSERT_TRUE(book.hasAsks());
  ASSERT_FALSE(book.hasBids());
  const auto bestAsk = book.getBestAskOrder();
  ASSERT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk->get().id, ask1.id);
  EXPECT_EQ(bestAsk->get().unfilledQty, 15);
}

TEST_F(MatchingEngineTest,
       processOrder_marketOrderWithInsufficientLiquidity_doesNotRestRemainder) {
  core::Order restingSell(1, 100, 30, core::Side::Sell);
  core::Order incomingBuyMarket(2, 0, 50, core::Side::Buy,
                                core::OrderType::Market);
  engine.processOrder(restingSell);

  const auto trades = engine.processOrder(incomingBuyMarket);

  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 30);
  EXPECT_FALSE(book.hasAsks());
  EXPECT_FALSE(book.hasBids());
}

TEST_F(MatchingEngineTest,
       processOrder_nonCrossingLimitOrder_doesNotTradeAndRestsOnBook) {
  core::Order restingAsk(1, 105, 25, core::Side::Sell);
  core::Order incomingBuy(2, 100, 25, core::Side::Buy);
  engine.processOrder(restingAsk);

  const auto trades = engine.processOrder(incomingBuy);

  ASSERT_TRUE(trades.empty());
  EXPECT_TRUE(book.hasAsks());
  EXPECT_TRUE(book.hasBids());
}

TEST_F(MatchingEngineTest,
       modifyOrder_existingOrder_updatesOrderAtNewPriceLevel) {
  core::Order buyOrder(1, 100, 40, core::Side::Buy);
  engine.processOrder(buyOrder);

  engine.modifyOrder(buyOrder.id, 40, 103);

  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buyOrder.id);
  EXPECT_EQ(bestBid->get().price, 103);
  EXPECT_EQ(bestBid->get().unfilledQty, 40);
}

TEST_F(MatchingEngineTest,
       modifyOrder_existingOrder_updatesOrderWithNewQuantityLower) {
  core::Order buyOrder(1, 100, 40, core::Side::Buy);
  engine.processOrder(buyOrder);

  engine.modifyOrder(buyOrder.id, 35, 100);

  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buyOrder.id);
  EXPECT_EQ(bestBid->get().price, 100);
  EXPECT_EQ(bestBid->get().unfilledQty, 35);
}

TEST_F(MatchingEngineTest,
       modifyOrder_existingOrder_updatesOrderWithNewQuantityHigher) {
  core::Order buyOrder(1, 100, 40, core::Side::Buy);
  engine.processOrder(buyOrder);

  engine.modifyOrder(buyOrder.id, 45, 100);

  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buyOrder.id);
  EXPECT_EQ(bestBid->get().price, 100);
  EXPECT_EQ(bestBid->get().unfilledQty, 45);
}
TEST_F(MatchingEngineTest,
       processOrder_multipleCalls_returnsOnlyCurrentCallTrades) {
  core::Order sell1(1, 100, 40, core::Side::Sell);
  core::Order buy1(2, 100, 40, core::Side::Buy);
  core::Order buyNoMatch(3, 90, 10, core::Side::Buy);

  engine.processOrder(sell1);
  const auto firstCallTrades = engine.processOrder(buy1);
  const auto secondCallTrades = engine.processOrder(buyNoMatch);

  ASSERT_EQ(firstCallTrades.size(), 1u);
  EXPECT_TRUE(secondCallTrades.empty());
}