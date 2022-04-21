#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>
#include <unordered_set>

namespace calculation {

	class combinationFactory {
	public:
		using iterateCallback = std::function<void(const algorithmData&, size_t)>;
		combinationFactory(size_t aThreadsCount);
		size_t getCombinationsAmount() const;
		size_t getCurrentIndex() const;
		int getMaxAtrSize() const { return maxAtrSize; }
		const std::vector<algorithmData>& getThreadData(int aThread);
		void incrementThreadIndex(int aThread);
		void onFinish();
	private:
		void generateSuperTrend();
		void generateDeal();
		void generatePercent();
		void generateDynamicSL();
		void generateOpener();
		void generateActivation();
		void generateStop();
		void onIterate();

		std::vector<std::vector<algorithmData>> combinationsData;
		std::vector<size_t> indexes;
		size_t combinations = 0;
		const size_t threadsCount = 1;

		const int minAtrSize = 10;
		const int maxAtrSize = 150;
		const int atrSizeStep = 20;

		const double minStFactor = 3.0;
		const double maxStFactor = 41.0;
		const double stFactorStep = 2.0;

		const double minDealPercent = 5.0;
		const double maxDealPercent = 5.0;
		const double dealPercentStep = 0.25;

		const int leverage = 50;
		const double orderSize = 2500.0;
		const double startCash = 75'000;
		const double maxLossCash = 25'000;
		const double maxLossPercent = 35;

		const double minLiquidationOffsetPercent = 0.05;
		const int offsetSteps = 2;

		const double minMinProfitPercent = 0.1;
		const double maxMinProfitPercent = 0.1;
		const double minProfitPercentStep = 0.1;

		const double minDynamicSLPercent = 0.25;
		const double maxDynamicSLPercent = 3.0;
		const double dynamicSLPercentStep = 0.25;

		const std::unordered_set<bool> stopLossWaiterEnabledFlags = { true };
		const std::unordered_set<bool> breakOpenerEnabledFlags = { true, false };
		const std::unordered_set<bool> dynamicSLTrendModeFlags = { false };

		const int minTrendActivationWaitRange = 0;
		const int maxTrendActivationWaitRange = 3;
		const int minStopLossWaitRange = 0;
		const int maxStopLossWaitRange = 3;

		bool inited = false;
	};
}
