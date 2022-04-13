#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>

namespace calculation {

	class combinationFactory {
	public:
		using iterateCallback = std::function<void(const algorithmData&, size_t)>;
		combinationFactory(size_t aThreadsCount);
		size_t getCombinationsAmount() const;
		size_t getCurrentIndex() const;
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
		const double stopCashFactor = 0.4;

		const int minAtrSize = 15;
		const int maxAtrSize = 15;
		const int atrSizeStep = 5;

		const double minStFactor = 1.0;
		const double maxStFactor = 21.0;
		const double stFactorStep = 2.0;

		const double minDealPercent = 5.0;
		const double maxDealPercent = 5.0;
		const double dealPercentStep = 0.25;

		const int leverage = 100;
		const double orderSize = 2500.0;

		const double minLiquidationOffsetPercent = 0.05;
		const int offsetSteps = 0;

		const double minMinProfitPercent = 1.0;
		const double maxMinProfitPercent = 5.0;
		const double minProfitPercentStep = 1.0;

		const double minDynamicSLPercent = 2.5;
		const double maxDynamicSLPercent = 15.0;
		const double dynamicSLPercentStep = 2.5;
		const bool useDynamicSLPercent = true;

		const int minTrendActivationWaitRange = 0;
		const int maxTrendActivationWaitRange = 5;
		const int minStopLossWaitRange = 0;
		const int maxStopLossWaitRange = 5;

		bool inited = false;
	};
}
