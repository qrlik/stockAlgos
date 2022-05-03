#pragma once
#include "algorithm/superTrend/stAlgorithmData.h"
#include <functional>
#include <optional>
#include <unordered_set>

namespace calculation {

	class combinationFactory {
	public:
		using iterateCallback = std::function<void(const algorithm::stAlgorithmData&, size_t)>;
		combinationFactory(size_t aThreadsAmount);
		size_t getCombinationsAmount() const;
		size_t getCurrentIndex() const;
		int getMaxAtrSize() const { return maxAtrSize; }
		const std::vector<algorithm::stAlgorithmData>& getThreadData(int aThread);
		void incrementThreadIndex(int aThread);
		void onFinish();
	private:
		void generateCombinations(size_t aIndex);
		void iterateCombination(size_t aIndex, const std::string& aName, const Json& aValue);
		void onIterate();

		void checkNewCode();
		void generateSuperTrend();
		void generateDeal();
		void generatePercent();
		void generateDynamicSL();
		void generateOpener();
		void generateActivation();
		void generateStop();

		Json settings;
		std::vector<std::vector<algorithm::stAlgorithmData>> combinationsData;
		std::vector<size_t> indexes;
		size_t combinations = 0;
		const size_t threadsAmount = 0;

		const int minAtrSize = 50;
		const int maxAtrSize = 100;
		const int atrSizeStep = 25;

		const double minStFactor = 10.0;
		const double maxStFactor = 20.0;
		const double stFactorStep = 5.0;

		const double dealPercent = 5.0;
		const int leverage = 100;
		const double orderSize = 2500.0;
		const double startCash = 75'000;
		const double maxLossCash = 25'000;
		const double maxLossPercent = 35;

		const double minLiquidationOffsetPercent = 0.05;
		const int liquidationOffsetSteps = 0;
		const double minProfitPercent = 0.1;

		const double minDynamicSLPercent = 7.0;
		const double maxDynamicSLPercent = 12.0;
		const double dynamicSLPercentStep = 2.5;

		const int minTrendActivationWaitRange = 0;
		const int maxTrendActivationWaitRange = 1;
		const int minStopLossWaitRange = 0;
		const int maxStopLossWaitRange = 1;

		const std::unordered_set<bool> dynamicSLTrendModeFlags = { false };
		const std::unordered_set<bool> touchOpenerActivationWaitModeFlags = { true, false };

		const std::unordered_set<bool> breakOpenerEnabledFlags = { false };
		const std::unordered_set<bool> breakOpenerActivationWaitModeFlags = { true, false };
		const std::unordered_set<bool> alwaysUseNewTrendFlags = { true, false };

		const std::unordered_set<bool> activationWaiterResetAllowedFlags = { true, false };
		const std::unordered_set<bool> activationWaiterFullCandleCheckFlags = { true, false };

		const std::unordered_set<bool> stopLossWaiterEnabledFlags = { true };
		const std::unordered_set<bool> stopLossWaiterResetAllowedFlags = { true, false };
		const std::unordered_set<bool> stopLossWaiterFullCandleCheckFlags = { true, false };
	};
}
