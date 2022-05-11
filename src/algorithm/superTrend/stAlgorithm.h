#pragma once
#include "algorithm/algorithmBase.hpp"
#include "stAlgorithmData.h"
#include "market/indicatorsSystem.h"
#include "activationWaiter.h"
#include "dynamicStopLoss.h"
#include "stopLossWaiter.h"
#include "trendBreakOpener.h"
#include "trendTouchOpener.h"

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	enum class eCustomState {
		STOP_LOSS_WAIT = 3,
		ACTIVATION_WAIT = 4
	};

	class stAlgorithm final : public algorithmBase<stAlgorithmData> {
		friend class calculation::calculationSystem; // TO DO delete
	public:
		using baseClass = algorithmBase<stAlgorithmData>;
		stAlgorithm(const stAlgorithmData& aData);
		bool operator==(const stAlgorithm& aOther) const;

		activationWaiter& getActivationWaiter() { return activationWaiterModule; }
		stopLossWaiter& getStopLossWaiter() { return stopLossWaiterModule; }
		double getLastUpSuperTrend() const { return lastUpSuperTrend; }
		double getLastDownSuperTrend() const { return lastDownSuperTrend; }
		double getSuperTrend() const;
		double getActualSuperTrend() const;
		bool getIsTrendUp() const { return isTrendUp; }

		bool isNewTrendChanged();

		void updateOrderStopLoss(double aStopLoss);
	private:
		void preLoop() override;
		bool loop() override;
		void onOpenOrder() override;
		void onCloseOrder(double aProfit) override;
		void log() const override;
		void initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool updateOrder();
		bool checkTrend();

		activationWaiter activationWaiterModule;
		stopLossWaiter stopLossWaiterModule;
		dynamicStopLoss dynamicStopLossModule;
		trendTouchOpener trendTouchOpenerModule;
		trendBreakOpener trendBreakOpenerModule;

		double lastUpSuperTrend = 0.0;
		double lastDownSuperTrend = 0.0;
		bool isTrendUp = false;
		bool isNewTrend = false;
	};
}
