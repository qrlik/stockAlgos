#pragma once
#include "../structs/algorithmData.h"
#include "../structs/candle.h"
#include "../structs/orderData.h"
#include "../structs/statistic.h"
#include "../indicators/indicators.h"
#include "activationWaiter.h"
#include "dynamicStopLoss.h"
#include "stopLossWaiter.h"
#include "trendBreakOpener.h"
#include "trendTouchOpener.h"

namespace algorithm {
    class moneyMaker {
    public:
        enum class eState : unsigned char {
            NONE = 0,
            LONG = 1,
            SHORT = 2,
            STOP_LOSS_WAIT = 3,
            ACTIVATION_WAIT = 4
        };
        static std::string stateToString(eState aState);

        moneyMaker(const algorithmData& aData, double aCash);
        void calculate(const std::vector<candle>& aCandles);

        activationWaiter& getActivationWaiter();
        orderData& getOrder();
        const candle& getCandle() const;
        double getLastUpSuperTrend() const;
        double getLastDownSuperTrend() const;
        eState getState() const;
        void setState(eState aState);
        void setWithLogs(bool aState);
        bool getIsTrendUp() const;

        double getSuperTrend() const;
        double getActualSuperTrend() const;
        double getTrendActivation(double aSuperTrend) const;
        bool isNewTrendChanged();

        void openOrder(eState aState, double aPrice);
    private:
        bool doAction(const candle& aCandle);
        bool update();
        bool updateCandles(const candle& aCandle);
        void updateTrends();
        bool updateOrder();
        bool checkTrend();
        void closeOrder();
        void log();
        double getStopLossPrice() const;
        double getMinimumProfitPrice() const;

        activationWaiter activationWaiterModule;
        stopLossWaiter stopLossWaiterModule;
        dynamicStopLoss dynamicStopLossModule;
        trendTouchOpener trendTouchOpenerModule;
        trendBreakOpener trendBreakOpenerModule;
        statistic stats;

        double lastUpSuperTrend = 0.0;
        double lastDownSuperTrend = 0.0;
        bool isTrendUp = false;
        bool isNewTrend = false;
        eState state = eState::NONE;

        orderData order;
        candle curCandle;
        candle prevCandle;

        const double stFactor = 0.0;
        const int atrSize = 0;
        const indicators::eAtrType atrType = indicators::eAtrType::NONE;

        const double activationPercent = 0.0;
        const double stopLossPercent = 0.0;
        const double minimumProfitPercent = 0.0;
        const double dealPercent = 0.0;

        const double startCash = 0.0;
        const double stopCash = 0.0;
        double cash = 0.0;
        const int leverage = 0;

        bool stopCashBreak = false;
        bool fullCheck = false;
        bool withLogs = false;
        bool inited = false;
    };
}
