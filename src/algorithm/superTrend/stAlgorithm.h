#pragma once
#include "algorithm/algorithmBase.hpp"
#include "stAlgorithmData.h"
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

	class stAlgorithm final : public algorithmBase<stAlgorithmData> {
		friend class calculation::calculationSystem; // TO DO delete
	public:
		using baseClass = algorithmBase<stAlgorithmData>;
		static std::string stateToString(eState aState); // TO DO look
		static eState stateFromString(const std::string& aStr); // TO DO look

		stAlgorithm(const stAlgorithmData& aData);
		bool calculate(const std::vector<candle>& aCandles);
		bool operator==(const stAlgorithm& aOther) const;

		activationWaiter& getActivationWaiter() { return activationWaiterModule; }
		stopLossWaiter& getStopLossWaiter() { return stopLossWaiterModule; }
		double getLastUpSuperTrend() const { return lastUpSuperTrend; }
		double getLastDownSuperTrend() const { return lastDownSuperTrend; }
		double getSuperTrend() const;
		double getActualSuperTrend() const;
		eState getState() const { return state; }
		bool getIsTrendUp() const { return isTrendUp; }

		void setState(eState aState);

		bool isNewTrendChanged();

		void openOrder(eState aState, double aPrice);
		void updateOrderStopLoss(double aStopLoss);
	private:
		void preLoop() override;
		bool loop() override;
		void log() const override;
		void initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool updateOrder();
		bool checkTrend();
		void closeOrder();

		activationWaiter activationWaiterModule;
		stopLossWaiter stopLossWaiterModule;
		dynamicStopLoss dynamicStopLossModule;
		trendTouchOpener trendTouchOpenerModule;
		trendBreakOpener trendBreakOpenerModule;

		double lastUpSuperTrend = 0.0;
		double lastDownSuperTrend = 0.0;
		bool isTrendUp = false;
		bool isNewTrend = false;
		eState state = eState::NONE;

		const double stFactor = 0.0;
		const int atrSize = 0;
		const market::eAtrType atrType = market::eAtrType::NONE;
	};
}
