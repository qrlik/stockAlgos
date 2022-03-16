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

        const int minAtrSize = 15;
        const int maxAtrSize = 15;
        const double minStFactor = 3.0;
        const double maxStFactor = 3.0;

        const double minDealPercent = 5.0;
        const double maxDealPercent = 5.0;
        const int minLeverage = 50;
        const int maxLeverage = 50;

        const double minMinProfitPercent = 0.25;
        const double maxMinProfitPercent = 2.5;

        const double minliquidationOffsetPercent = 0.25;
        const double maxliquidationOffsetPercent = 0.1; // < liquidation
        const double minDynamicSLPercent = 0.25;
        const double maxDynamicSLPercent = 1.0;

        const int minTrendActivationWaitRange = 0;
        const int maxTrendActivationWaitRange = 5;
        const int minStopLossWaitRange = 0;
        const int maxStopLossWaitRange = 5;

        const double stepFloat = 0.25;
        const int stepInt = 5;

        bool inited = false;
    };
}
