#include <gtest/gtest.h>

#include "core/book.h"
#include "core/order.h"
#include "core/types.h"
#include "engine/matching_engine.h"

using ::testing::Test;

class EndToEndTest : public Test {
 protected:
  // This test will cover a full flow of order processing, including adding
  // orders, matching them, and verifying the resulting trades and order book
  // state.
  core::OrderBook book;
  engine::MatchingEngine engine{book};
};

TEST_F(
    EndToEndTest,
    endToEnd_mixedSession_addMatchModifyCancel_stateTransitionsAreConsistent) {
  // This test will be implemented in the future to cover a complete end-to-end
  // scenario. For now, it serves as a placeholder for future integration
  // tests. The test will create a matching engine, add orders to the book,
  // process them, and verify that the trades are executed correctly and the
  // order book is updated as expected.
  core::Order sell1(1, 101, 20, core::Side::Sell);
  core::Order buy1(2, 99, 10, core::Side::Buy);
  core::Order buy2(3, 102, 25, core::Side::Buy);  // crosses sell1
  core::Order sellMarket(4, 0, 16, core::Side::Sell,
                         core::OrderType::Market);  // matches with buy2

  const auto t1 = engine.processOrder(sell1);
  const auto t2 = engine.processOrder(buy1);
  ASSERT_TRUE(t1.empty());
  ASSERT_TRUE(t2.empty());
  ASSERT_TRUE(book.hasAsks());
  ASSERT_TRUE(book.hasBids());

  const auto t3 = engine.processOrder(buy2);
  ASSERT_EQ(t3.size(), 1u);
  EXPECT_EQ(t3[0].price, 101);
  EXPECT_EQ(t3[0].quantity, 20);
  EXPECT_EQ(t3[0].maker, sell1.id);
  EXPECT_EQ(t3[0].taker, buy2.id);

  ASSERT_FALSE(book.hasAsks());
  ASSERT_TRUE(book.hasBids());
  auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buy2.id);
  EXPECT_EQ(bestBid->get().unfilledQty, 5);

  engine.modifyOrder(buy2.id, 15, 98);  // increase quantity and reduce price
  bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id,
            buy1.id);  // buy1 should now be the best bid after buy2's price is
                       // reduced
  EXPECT_EQ(bestBid->get().price, 99);

  book.cancelOrder(buy1.id);  // cancel buy1
  bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id,
            buy2.id);  // buy2 should now be the best bid again
  EXPECT_EQ(bestBid->get().price, 98);
  EXPECT_EQ(bestBid->get().unfilledQty, 15);

  const auto t4 = engine.processOrder(sellMarket);
  ASSERT_EQ(t4.size(), 1u);
  EXPECT_EQ(t4[0].price, 98);
  EXPECT_EQ(t4[0].quantity, 15);
  EXPECT_EQ(t4[0].maker, buy2.id);
  EXPECT_EQ(t4[0].taker, sellMarket.id);

  EXPECT_FALSE(book.hasAsks());
  EXPECT_FALSE(book.hasBids());
}

TEST_F(EndToEndTest,
       endToEnd_buyMarketExhaustsAsks_existingBidSideRemainsUnchanged) {
  core::Order sell1(1, 99, 10, core::Side::Sell);
  core::Order sell2(2, 100, 20, core::Side::Sell);
  core::Order buy1(3, 90, 7, core::Side::Buy);
  core::Order buyMarket(4, 0, 50, core::Side::Buy, core::OrderType::Market);

  engine.processOrder(sell1);
  engine.processOrder(sell2);
  engine.processOrder(buy1);

  const auto trades = engine.processOrder(buyMarket);

  ASSERT_EQ(trades.size(), 2u);
  EXPECT_EQ(trades[0].price, 99);
  EXPECT_EQ(trades[0].quantity, 10);
  EXPECT_EQ(trades[0].maker, sell1.id);
  EXPECT_EQ(trades[0].taker, buyMarket.id);
  EXPECT_EQ(trades[1].price, 100);
  EXPECT_EQ(trades[1].quantity, 20);
  EXPECT_EQ(trades[1].maker, sell2.id);
  EXPECT_EQ(trades[1].taker, buyMarket.id);

  EXPECT_FALSE(book.hasAsks());
  ASSERT_TRUE(book.hasBids());

  const auto bestBid = book.getBestBidOrder();
  ASSERT_TRUE(bestBid.has_value());
  EXPECT_EQ(bestBid->get().id, buy1.id);
  EXPECT_EQ(bestBid->get().price, 90);
  EXPECT_EQ(bestBid->get().unfilledQty, 7);
}

TEST_F(EndToEndTest,
       endToEnd_fifoSamePrice_thenModify_thenCompleteFill_bookEndsEmpty) {
  core::Order ask1(1, 100, 10, core::Side::Sell);
  core::Order ask2(2, 100, 15, core::Side::Sell);  // FIFO after ask1
  core::Order bid1(
      3, 100, 12,
      core::Side::Buy);  // should consume ask1 and partially fill ask2
  core::Order bid2(
      4, 99, 13,
      core::Side::Buy);  // After modification, should consume remaining ask2
  core::Order bid3(5, 97, 4, core::Side::Buy);

  engine.processOrder(ask1);
  engine.processOrder(ask2);

  const auto t1 = engine.processOrder(bid1);
  ASSERT_EQ(t1.size(), 2u);
  EXPECT_EQ(t1[0].price, 100);
  EXPECT_EQ(t1[0].quantity, 10);
  EXPECT_EQ(t1[0].maker, ask1.id);
  EXPECT_EQ(t1[0].taker, bid1.id);
  EXPECT_EQ(t1[1].price, 100);
  EXPECT_EQ(t1[1].quantity, 2);
  EXPECT_EQ(t1[1].maker, ask2.id);
  EXPECT_EQ(t1[1].taker, bid1.id);

  auto bestAsk = book.getBestAskOrder();
  ASSERT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk->get().id, ask2.id);
  EXPECT_EQ(bestAsk->get().unfilledQty, 13);

  engine.modifyOrder(ask2.id, 13, 99);
  bestAsk = book.getBestAskOrder();
  ASSERT_TRUE(bestAsk.has_value());
  EXPECT_EQ(bestAsk->get().id, ask2.id);
  EXPECT_EQ(bestAsk->get().price, 99);

  const auto t2 = engine.processOrder(bid2);
  ASSERT_EQ(t2.size(), 1u);
  EXPECT_EQ(t2[0].price, 99);
  EXPECT_EQ(t2[0].quantity, 13);
  EXPECT_EQ(t2[0].maker, ask2.id);
  EXPECT_EQ(t2[0].taker, bid2.id);

  EXPECT_FALSE(book.hasAsks());
  EXPECT_FALSE(book.hasBids());

  const auto t3 = engine.processOrder(bid3);
  ASSERT_TRUE(t3.empty());
  book.cancelOrder(bid3.id);
  EXPECT_FALSE(book.hasAsks());
  EXPECT_FALSE(book.hasBids());
}