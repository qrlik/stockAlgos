#pragma once
#include "structs/algorithmData.h"
#include "structs/candle.h"
#include "structs/orderData.h"
#include "structs/statistic.h"
#include "market/indicators.h"
#include "activationWaiter.h"
#include "dynamicStopLoss.h"
#include "stopLossWaiter.h"
#include "trendBreakOpener.h"
#include "trendTouchOpener.h"

namespace tests {
	class mmChecker;
}
namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	enum class eState : unsigned char {
		NONE = 0,
		LONG = 1,
		SHORT = 2,
		STOP_LOSS_WAIT = 3,
		ACTIVATION_WAIT = 4
	};

	class moneyMaker {
		friend class tests::mmChecker;
		friend class calculation::calculationSystem;
	public:
		static std::string stateToString(eState aState);
		static eState stateFromString(const std::string& aStr);

		moneyMaker(const algorithmData& aData);
		bool calculate(const std::vector<candle>& aCandles);
		bool operator==(const moneyMaker& aOther);

		activationWaiter& getActivationWaiter() { return activationWaiterModule; }
		stopLossWaiter& getStopLossWaiter() { return stopLossWaiterModule; }
		orderData& getOrder() { return order; }
		const candle& getCandle() const { return curCandle; }
		double getLastUpSuperTrend() const { return lastUpSuperTrend; }
		double getLastDownSuperTrend() const { return lastDownSuperTrend; }
		double getSuperTrend() const;
		double getActualSuperTrend() const;
		double getFullCash() const;
		double getCash() const { return cash; }
		double getStartCash() const { return startCash; }
		double getDealPercent() const { return dealPercent; }
		double getOrderSize() const { return orderSize; }
		double getLiquidationOffsetPercent() const { return liquidationOffsetPercent; }
		double getMinimumProfitPercent() const { return minimumProfitPercent; }
		int getLeverage() const { return leverage; }
		eState getState() const { return state; }
		bool getIsTrendUp() const { return isTrendUp; }
		bool getFullCheck() const { return fullCheck; }

		void setState(eState aState);
		void setWithLogs(bool aState);

		bool isNewTrendChanged();

		void openOrder(eState aState, double aPrice);
		bool doAction(const candle& aCandle);
	private:
		bool update();
		bool updateCandles(const candle& aCandle);
		void updateTrends();
		bool updateOrder();
		bool checkTrend();
		void closeOrder();
		void log();

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
		const market::eAtrType atrType = market::eAtrType::NONE;

		const double liquidationOffsetPercent = 0.0;
		const double minimumProfitPercent = 0.0;
		const double dealPercent = 0.0;
		const double orderSize = -1.0;

		const double startCash = 0.0;
		double cash = 0.0;
		const int leverage = 0;

		bool stopCashBreak = false;
		bool fullCheck = false;
		bool withLogs = false;
		bool inited = false;
	};
}
