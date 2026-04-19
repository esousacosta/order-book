#include <gtest/gtest.h>
#include "core/book.h"

using ::testing::Test;

class BookTest : public Test {
};

TEST_F(BookTest, addOrder_buyLimitOrder_orderAddedToBids) {
    // Arrange
    core::OrderBook book;
    const core::Order buyOrder(1, 100, 40, core::Side::Buy);

    // Act
    book.addOrder(buyOrder);

    // Assert
    ASSERT_TRUE(book.hasBids());
    const auto bidOpt = book.getBestBidOrder();
    ASSERT_TRUE(bidOpt.has_value());
    const core::Order& bookBid = bidOpt.value();
    ASSERT_EQ(bookBid.id, buyOrder.id);
    ASSERT_EQ(bookBid.price, buyOrder.price);
    ASSERT_EQ(bookBid.qty, buyOrder.qty);
    ASSERT_EQ(bookBid.side, buyOrder.side);
}

TEST_F(BookTest, addOrder_sellLimitOrder_orderAddedToAsks) {
    // Arrange
    core::OrderBook book;
    const core::Order sellOrder(1, 100, 40, core::Side::Sell);

    // Act
    book.addOrder(sellOrder);

    // Assert
    ASSERT_TRUE(book.hasAsks());
    const auto askOpt = book.getBestAskOrder();
    ASSERT_TRUE(askOpt.has_value());
    const core::Order& bookAsk = askOpt.value();
    ASSERT_EQ(bookAsk.id, sellOrder.id);
    ASSERT_EQ(bookAsk.price, sellOrder.price);
    ASSERT_EQ(bookAsk.qty, sellOrder.qty);
    ASSERT_EQ(bookAsk.side, sellOrder.side);
}

TEST_F(BookTest, getOrder_orderNotInTheBook_nullopt) {
    // Arrange
    core::OrderBook book;
    const core::Order buyOrder(1, 100, 40, core::Side::Buy);
    book.addOrder(buyOrder);

    // Act
    const auto orderOpt = book.getOrder(2);

    // Assert
    ASSERT_FALSE(orderOpt.has_value());
}

TEST_F(BookTest, getOrder_orderInTheBook_orderReturned) {
    // Arrange
    core::OrderBook book;
    const core::Order buyOrder(1, 100, 40, core::Side::Buy);
    book.addOrder(buyOrder);

    // Act
    const auto orderOpt = book.getOrder(1);

    // Assert
    ASSERT_TRUE(orderOpt.has_value());
    const auto& order = orderOpt.value();
    ASSERT_EQ(order.id, buyOrder.id);
    ASSERT_EQ(order.price, buyOrder.price);
    ASSERT_EQ(order.qty, buyOrder.qty);
}

TEST_F(BookTest, getAskCount_threeAddedSellOrdersAndThreeBuyOrders_sixTotalOrders) {
    // Arrange
    core::OrderBook book;
    const core::Order sellOrder1(1, 100, 40, core::Side::Sell);
    const core::Order sellOrder2(2, 110, 30, core::Side::Sell);
    const core::Order sellOrder3(3, 120, 20, core::Side::Sell);
    const core::Order buyOrder1(4, 90, 50, core::Side::Buy);
    const core::Order buyOrder2(5, 80, 60, core::Side::Buy);
    const core::Order buyOrder3(6, 70, 70, core::Side::Buy);
    book.addOrder(buyOrder1);
    book.addOrder(buyOrder2);
    book.addOrder(buyOrder3);
    book.addOrder(sellOrder1);
    book.addOrder(sellOrder2);
    book.addOrder(sellOrder3);

    // Act
    const auto askCount = book.getAskCount();
    const auto bidCount = book.getBidCount();

    // Assert
    ASSERT_EQ(askCount, 3);
    ASSERT_EQ(bidCount, 3);
}

TEST_F(BookTest, cancelOrder_cancelBuyOrder_orderCanceled) {
    // Arrange
    core::OrderBook book;
    const core::Order buyOrder(1, 100, 40, core::Side::Buy);
    const core::Order secondBuyOrder(2, 90, 30, core::Side::Buy);
    const core::Order sellOrder(3, 110, 20, core::Side::Sell);
    book.addOrder(buyOrder);
    book.addOrder(secondBuyOrder);
    book.addOrder(sellOrder);

    // Act
    book.cancelOrder(secondBuyOrder.id);
    book.cancelOrder(sellOrder.id);

    // Assert
    ASSERT_EQ(book.getBidCount(), 1);
    const auto bidOpt = book.getBestBidOrder();
    ASSERT_EQ(bidOpt->get().id, buyOrder.id);
    ASSERT_EQ(book.getAskCount(), 0);
}