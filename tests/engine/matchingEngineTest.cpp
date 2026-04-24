#include "core/book.h"
#include "engine/matching_engine.h"
#include <gtest/gtest.h>

using ::testing::Test;

class MatchingEngineTest : public Test {};

TEST_F(MatchingEngineTest, processOrder_emptyOrderBook_orderAddedToBids) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder(1, 100, 40, core::Side::Buy);

  // Act
  engine.processOrder(buyOrder);

  // Assert
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder.id);
  ASSERT_EQ(bookBid.price, buyOrder.price);
  ASSERT_EQ(bookBid.qty, buyOrder.qty);
  ASSERT_EQ(bookBid.side, buyOrder.side);
}

TEST_F(MatchingEngineTest, processOrder_emptyOrderBook_orderAddedToAsks) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder(1, 100, 40, core::Side::Sell);

  // Act
  engine.processOrder(sellOrder);

  // Assert
  ASSERT_TRUE(engine.bookHasAsks());
  const auto askOpt = engine.getBestAsk();
  ASSERT_TRUE(askOpt.has_value());
  const core::Order &bookAsk = askOpt.value();
  ASSERT_EQ(bookAsk.id, sellOrder.id);
  ASSERT_EQ(bookAsk.price, sellOrder.price);
  ASSERT_EQ(bookAsk.qty, sellOrder.qty);
  ASSERT_EQ(bookAsk.side, sellOrder.side);
}

TEST_F(
    MatchingEngineTest,
    processOrder_orderMatchesExistingOrderPartially_takerOrderPartiallyFilledAndBookUpdated) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder(1, 100, 40, core::Side::Sell);
  core::Order buyOrder(2, 100, 50, core::Side::Buy);
  engine.processOrder(sellOrder);

  // Act
  engine.processOrder(buyOrder);

  // Assert
  // Sell order should be fully filled and removed from the book
  ASSERT_FALSE(engine.bookHasAsks());
  // Buy order should be partially filled with 10 remaining quantity
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder.id);
  ASSERT_EQ(bookBid.price, buyOrder.price);
  ASSERT_EQ(bookBid.qty, 50);         // Remaining quantity after partial fill
  ASSERT_EQ(bookBid.unfilledQty, 10); // Remaining quantity after partial fill
  ASSERT_EQ(bookBid.side, buyOrder.side);
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 1u);
  const auto &trade = trades.front();
  ASSERT_EQ(trade.price, 100);
  ASSERT_EQ(trade.quantity, 40);
  ASSERT_EQ(trade.maker, sellOrder.id);
  ASSERT_EQ(trade.taker, buyOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_orderMatchesExistingOrderFully_bothOrdersFilledAndRemovedFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder(1, 100, 40, core::Side::Sell);
  core::Order buyOrder(2, 100, 40, core::Side::Buy);
  engine.processOrder(sellOrder);

  // Act
  engine.processOrder(buyOrder);

  // Assert
  // Both orders should be fully filled and removed from the book
  ASSERT_FALSE(engine.bookHasAsks());
  ASSERT_FALSE(engine.bookHasBids());
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 1u);
  const auto &trade = trades.front();
  ASSERT_EQ(trade.price, 100);
  ASSERT_EQ(trade.quantity, 40);
  ASSERT_EQ(trade.maker, sellOrder.id);
  ASSERT_EQ(trade.taker, buyOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_buyOrderMatchesMultipleExistingOrders_partiallyFillsIncomingOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder1(1, 100, 30, core::Side::Sell);
  core::Order sellOrder2(2, 95, 20, core::Side::Sell);
  core::Order sellOrder3(3, 90, 5, core::Side::Sell);
  core::Order buyOrder(4, 100, 40, core::Side::Buy);
  engine.processOrder(sellOrder1);
  engine.processOrder(sellOrder2);
  engine.processOrder(sellOrder3);

  // Act
  engine.processOrder(buyOrder);

  // Assert
  // Sell orders 2 and 3 should be fully filled and removed from the book
  ASSERT_TRUE(engine.bookHasAsks());
  // Buy order should be fully filled
  ASSERT_FALSE(engine.bookHasBids());
  const auto askOpt = engine.getBestAsk();
  ASSERT_TRUE(askOpt.has_value());
  const core::Order &bookAsk = askOpt.value();
  ASSERT_EQ(bookAsk.id, sellOrder1.id);
  ASSERT_EQ(bookAsk.price, sellOrder1.price);
  ASSERT_EQ(bookAsk.qty, 30);
  ASSERT_EQ(bookAsk.unfilledQty, 15);
  ASSERT_EQ(bookAsk.side, sellOrder1.side);
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 3u);
}

TEST_F(
    MatchingEngineTest,
    processOrder_sellOrderMatchesMultipleExistingOrders_partiallyFillsIncomingOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder1(1, 100, 30, core::Side::Buy);
  core::Order buyOrder2(2, 95, 10, core::Side::Buy);
  core::Order buyOrder3(3, 90, 5, core::Side::Buy);
  core::Order sellOrder(4, 92, 40, core::Side::Sell);
  engine.processOrder(buyOrder1);
  engine.processOrder(buyOrder2);
  engine.processOrder(buyOrder3);

  // Act
  engine.processOrder(sellOrder);

  // Assert
  // Buy orders 1 and 2 should be impacted
  ASSERT_FALSE(engine.bookHasAsks());
  // Sell order should be fully filled
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder3.id);
  ASSERT_EQ(bookBid.price, buyOrder3.price);
  ASSERT_EQ(bookBid.qty, buyOrder3.qty);
  ASSERT_EQ(bookBid.unfilledQty, 5);
  ASSERT_EQ(bookBid.side, buyOrder3.side);
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 2u);
}

TEST_F(MatchingEngineTest,
       modifyOrder_existingBuyOrderModifiedWithHigherPrice_orderUpdatedInBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder(1, 100, 40, core::Side::Buy);
  engine.processOrder(buyOrder);

  // Act
  engine.modifyOrder(buyOrder.id, 40, 105); // Modify price to a higher level

  // Assert
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder.id);
  ASSERT_EQ(bookBid.price, 105); // Price should be updated
  ASSERT_EQ(bookBid.qty, buyOrder.qty);
  ASSERT_EQ(bookBid.side, buyOrder.side);
}

TEST_F(MatchingEngineTest,
       modifyOrder_existingSellOrderModifiedWithLowerPrice_orderUpdatedInBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder(1, 100, 40, core::Side::Sell);
  engine.processOrder(sellOrder);

  // Act
  engine.modifyOrder(sellOrder.id, 40, 95); // Modify price to a lower level

  // Assert
  ASSERT_TRUE(engine.bookHasAsks());
  const auto askOpt = engine.getBestAsk();
  ASSERT_TRUE(askOpt.has_value());
  const core::Order &bookAsk = askOpt.value();
  ASSERT_EQ(bookAsk.id, sellOrder.id);
  ASSERT_EQ(bookAsk.price, 95); // Price should be updated
  ASSERT_EQ(bookAsk.qty, sellOrder.qty);
  ASSERT_EQ(bookAsk.side, sellOrder.side);
}

TEST_F(MatchingEngineTest, modifyOrder_nonExistingOrder_orderNotAddedToBook) {
  // Arrange
  engine::MatchingEngine engine;

  // Act
  engine.modifyOrder(999, 40, 105); // Attempt to modify a non-existing order

  // Assert
  ASSERT_FALSE(engine.bookHasBids());
  ASSERT_FALSE(engine.bookHasAsks());
}

TEST_F(
    MatchingEngineTest,
    modifyOrder_existingOrderModifiedWithSamePrice_orderQuantityUpdatedInBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder(1, 100, 40, core::Side::Buy);
  engine.processOrder(buyOrder);

  // Act
  engine.modifyOrder(buyOrder.id, 30,
                     100); // Modify quantity but keep price the same

  // Assert
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder.id);
  ASSERT_EQ(bookBid.price, buyOrder.price); // Price should remain unchanged
  ASSERT_EQ(bookBid.qty, 40);               // Quantity should be updated
  ASSERT_EQ(bookBid.unfilledQty, 30);       // Quantity should be updated
  ASSERT_EQ(bookBid.side, buyOrder.side);
}

TEST_F(
    MatchingEngineTest,
    processOrder_buyMarketOrderMatchesExistingSellOrders_fullyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder1(1, 100, 30, core::Side::Sell);
  core::Order sellOrder2(2, 95, 20, core::Side::Sell);
  core::Order buyMarketOrder(3, 0, 50, core::Side::Buy,
                             core::OrderType::Market);
  engine.processOrder(sellOrder1);
  engine.processOrder(sellOrder2);

  // Act
  engine.processOrder(buyMarketOrder);

  // Assert
  // Both sell orders should be fully filled and removed from the book
  ASSERT_FALSE(engine.bookHasAsks());
  // Buy market order should be fully filled
  ASSERT_FALSE(engine.bookHasBids());
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 2u);
  const auto &trade1 = trades[0];
  ASSERT_EQ(trade1.price, 95);
  ASSERT_EQ(trade1.quantity, 20);
  ASSERT_EQ(trade1.maker, sellOrder2.id);
  ASSERT_EQ(trade1.taker, buyMarketOrder.id);
  const auto &trade2 = trades[1];
  ASSERT_EQ(trade2.price, 100);
  ASSERT_EQ(trade2.quantity, 30);
  ASSERT_EQ(trade2.maker, sellOrder1.id);
  ASSERT_EQ(trade2.taker, buyMarketOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_sellMarketOrderMatchesExistingBuyOrders_fullyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder1(1, 100, 30, core::Side::Buy);
  core::Order buyOrder2(2, 105, 20, core::Side::Buy);
  core::Order sellMarketOrder(3, 0, 50, core::Side::Sell,
                              core::OrderType::Market);
  engine.processOrder(buyOrder1);
  engine.processOrder(buyOrder2);

  // Act
  engine.processOrder(sellMarketOrder);

  // Assert
  // Both buy orders should be fully filled and removed from the book
  ASSERT_FALSE(engine.bookHasBids());
  // Sell market order should be fully filled
  ASSERT_FALSE(engine.bookHasAsks());
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 2u);
  const auto &trade1 = trades[0];
  ASSERT_EQ(trade1.price, 105);
  ASSERT_EQ(trade1.quantity, 20);
  ASSERT_EQ(trade1.maker, buyOrder2.id);
  ASSERT_EQ(trade1.taker, sellMarketOrder.id);
  const auto &trade2 = trades[1];
  ASSERT_EQ(trade2.price, 100);
  ASSERT_EQ(trade2.quantity, 30);
  ASSERT_EQ(trade2.maker, buyOrder1.id);
  ASSERT_EQ(trade2.taker, sellMarketOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_buyMarketOrderMatchesExistingSellOrders_partiallyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder1(1, 100, 30, core::Side::Sell);
  core::Order sellOrder2(2, 95, 20, core::Side::Sell);
  core::Order buyMarketOrder(3, 0, 40, core::Side::Buy,
                             core::OrderType::Market);
  engine.processOrder(sellOrder1);
  engine.processOrder(sellOrder2);

  // Act
  engine.processOrder(buyMarketOrder);

  // Assert
  // Sell order 2 should be fully filled and removed from the book
  ASSERT_TRUE(engine.bookHasAsks());
  const auto askOpt = engine.getBestAsk();
  ASSERT_TRUE(askOpt.has_value());
  const core::Order &bookAsk = askOpt.value();
  ASSERT_EQ(bookAsk.id, sellOrder1.id);
  ASSERT_EQ(bookAsk.price, sellOrder1.price);
  ASSERT_EQ(bookAsk.qty, 30);
  ASSERT_EQ(bookAsk.unfilledQty, 10); // Remaining quantity after partial fill
  ASSERT_EQ(bookAsk.side, sellOrder1.side);
  // Buy market order should be fully filled
  ASSERT_FALSE(engine.bookHasBids());
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 2u);
  const auto &trade1 = trades[0];
  ASSERT_EQ(trade1.price, 95);
  ASSERT_EQ(trade1.quantity, 20);
  ASSERT_EQ(trade1.maker, sellOrder2.id);
  ASSERT_EQ(trade1.taker, buyMarketOrder.id);
  const auto &trade2 = trades[1];
  ASSERT_EQ(trade2.price, 100);
  ASSERT_EQ(trade2.quantity, 20);
  ASSERT_EQ(trade2.maker, sellOrder1.id);
  ASSERT_EQ(trade2.taker, buyMarketOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_sellMarketOrderMatchesExistingBuyOrders_partiallyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order buyOrder1(1, 100, 30, core::Side::Buy);
  core::Order buyOrder2(2, 105, 20, core::Side::Buy);
  core::Order sellMarketOrder(3, 0, 40, core::Side::Sell,
                              core::OrderType::Market);
  engine.processOrder(buyOrder1);
  engine.processOrder(buyOrder2);

  // Act
  engine.processOrder(sellMarketOrder);

  // Assert
  // Buy order 2 should be fully filled and removed from the book
  ASSERT_TRUE(engine.bookHasBids());
  const auto bidOpt = engine.getBestBid();
  ASSERT_TRUE(bidOpt.has_value());
  const core::Order &bookBid = bidOpt.value();
  ASSERT_EQ(bookBid.id, buyOrder1.id);
  ASSERT_EQ(bookBid.price, buyOrder1.price);
  ASSERT_EQ(bookBid.qty, 30);
  ASSERT_EQ(bookBid.unfilledQty, 10); // Remaining quantity after partial fill
  ASSERT_EQ(bookBid.side, buyOrder1.side);
  // Sell market order should be fully filled
  ASSERT_FALSE(engine.bookHasAsks());
  core::Trades trades = engine.getTrades();
  ASSERT_EQ(trades.size(), 2u);
  const auto &trade1 = trades[0];
  ASSERT_EQ(trade1.price, 105);
  ASSERT_EQ(trade1.quantity, 20);
  ASSERT_EQ(trade1.maker, buyOrder2.id);
  ASSERT_EQ(trade1.taker, sellMarketOrder.id);
  const auto &trade2 = trades[1];
  ASSERT_EQ(trade2.price, 100);
  ASSERT_EQ(trade2.quantity, 20);
  ASSERT_EQ(trade2.maker, buyOrder1.id);
  ASSERT_EQ(trade2.taker, sellMarketOrder.id);
}

TEST_F(
    MatchingEngineTest,
    processOrder_buyMarketOrderTooLargeForExistingSellOrders_partiallyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
  // Arrange
  engine::MatchingEngine engine;
  core::Order sellOrder(1, 100, 30, core::Side::Sell);
  core::Order buyMarketOrder(2, 0, 50, core::Side::Buy,
                             core::OrderType::Market);
  engine.processOrder(sellOrder);

  // Act
  engine.processOrder(buyMarketOrder);

  // Assert
  // Sell order should be fully filled and removed from the book
  ASSERT_FALSE(engine.bookHasAsks());
  // Buy market order should be partially filled with 20 remaining quantity
  // Market orders are canceled if not fully filled, so it should not be added
  // to the book
  ASSERT_FALSE(engine.bookHasBids());
    core::Trades trades = engine.getTrades();
    ASSERT_EQ(trades.size(), 1u);
    const auto &trade = trades.front();
    ASSERT_EQ(trade.price, 100);
    ASSERT_EQ(trade.quantity, 30);
    ASSERT_EQ(trade.maker, sellOrder.id);
    ASSERT_EQ(trade.taker, buyMarketOrder.id);
}

TEST_F(MatchingEngineTest,
      processOrder_sellMarketOrderTooLargeForExistingBuyOrders_partiallyFillsMarketOrderAndRemovesFilledOrdersFromBook) {
    // Arrange
    engine::MatchingEngine engine;
    core::Order buyOrder(1, 100, 30, core::Side::Buy);
    core::Order sellMarketOrder(2, 0, 50, core::Side::Sell,
                                core::OrderType::Market);
    engine.processOrder(buyOrder);

    // Act
    engine.processOrder(sellMarketOrder);

    // Assert
    // Buy order should be fully filled and removed from the book
    ASSERT_FALSE(engine.bookHasBids());
    // Sell market order should be partially filled with 20 remaining quantity
    // Market orders are canceled if not fully filled, so it should not be added
    // to the book
    ASSERT_FALSE(engine.bookHasAsks());
    core::Trades trades = engine.getTrades();
    ASSERT_EQ(trades.size(), 1u);
    const auto &trade = trades.front();
    ASSERT_EQ(trade.price, 100);
    ASSERT_EQ(trade.quantity, 30);
    ASSERT_EQ(trade.maker, buyOrder.id);
    ASSERT_EQ(trade.taker, sellMarketOrder.id);
}