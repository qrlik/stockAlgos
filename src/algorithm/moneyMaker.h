#pragma once
#include "../structs/algorithmData.h"
#include "../structs/candle.h"
#include "../indicators/indicators.h"
#include "activationWaiter.h"

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
        moneyMaker(const algorithmData& aData);

        eState getState() const;
        void setState(eState aState);
        bool getIsTrendUp() const;
        const candle& getCandle() const;

        double getSuperTrend() const;
        double getActualSuperTrend() const;
        double getTrendActivation(double aSuperTrend) const;
        bool isNewTrendChanged();

        void openOrder(eState aState, double aPrice);
    private:
        activationWaiter activationWaiterModule;
        //stopLossWaiterModule = None
        //dynamicStopLossModule = None
        //trendTouchOpenerModule = None
        //trendBreakOpenerModule = None
        //stats = None

        double lastUpSuperTrend = 0.0;
        double lastDownSuperTrend = 0.0;
        bool isTrendUp = false;
        bool isNewTrend = false;
        eState state = eState::NONE;

        //order = None
        candle curCandle;
        candle prevCandle;

        double stFactor = 0.0;
        size_t atrSize = 0;
        indicators::eAtrType atrType = indicators::eAtrType::NONE;

        double activationPercent = 0.0;
        double stopLossPercent = 0.0;
        double minimumProfitPercent = 0.0;
        double dealPercent = 0.0;

        const double tax = 0.0004;
        double cash = 0.0;
        double startCash = 0.0;
        double stopCash = 0.0;
        int leverage = 0;

        bool stopCashBreak = false;
        bool fullCheck = false;
        bool withLogs = true;
    };
}
