#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>

namespace calculation {

	class combinationFactory {
	public:
        using iterateCallback = std::function<void(const algorithmData&, size_t)>;
        combinationFactory();
        void iterateCombination(int aPosition, iterateCallback aCallback);
        size_t getCombinationsAmount() const;
        void reset();
	private:
        void iterateCombination();
        void generateSuperTrend(int aPosition);
        void generateSuperTrend();
        void generateDeal(int aPosition);
        void generatePercent(int aPosition);
        void generateDynamicSL(int aPosition);
        void generateOpener(int aPosition);
        void generateActivation(int aPosition);
        void generateStop(int aPosition);
        void onIterate(int aPosition);

        std::vector<algorithmData> data;
        std::vector<iterateCallback> callbacks;
        std::vector<size_t> combinations;

        const int minAtrSize = 15;
        const int maxAtrSize = 15;
        const double minStFactor = 3.0;
        const double maxStFactor = 3.0;

        const double minDealPercent = 0.5;
        const double maxDealPercent = 5.0;
        const int minLeverage = 50;
        const int maxLeverage = 50;

        const double minMinProfitPercent = 0.5;
        const double maxMinProfitPercent = 2.5;

        const double minActivationPercent = 0.0;
        const double minStopLossPercent = 0.5;
        const double minDynamicSLPercent = 0.5;

        const int minTrendActivationWaitRange = 0;
        const int maxTrendActivationWaitRange = 3;
        const int minStopLossWaitRange = 0;
        const int maxStopLossWaitRange = 3;

        const double stepFloat = 0.5;
        const int stepInt = 5;

        bool callbackInited = false;
    };
}
