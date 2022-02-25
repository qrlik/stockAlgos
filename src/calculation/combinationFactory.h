#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>

namespace calculation {

	class combinationFactory {
	public:
        using iterateCallback = std::function<void(const algorithmData&, size_t)>;
        combinationFactory();
        void iterateCombination(iterateCallback aCallback);
        size_t getCombinationsAmount() const;
	private:
        void iterateCombination();
        void generateSuperTrend();
        void generateDeal();
        void generatePercent();
        void generateDynamicSL();
        void generateOpener();
        void generateActivation();
        void generateStop();
        void onIterate();

        algorithmData data;
        iterateCallback callback;
        size_t combinations = 0;

        const int minAtrSize = 15;
        const int maxAtrSize = 15; // 25;
        const double minStFactor = 3.0;
        const double maxStFactor = 3.0; //5.0;

        const double minDealPercent = 5.0; // 1.0;
        const double maxDealPercent = 5.0;
        const int minLeverage = 50;
        const int maxLeverage = 50;

        const double minMinProfitPercent = 0.5;// 1.0;
        const double maxMinProfitPercent = 0.5;// 2.0;

        const double minActivationPercent = 0.0;
        const double minStopLossPercent = 1.0;
        const double minDynamicSLPercent = 1.0;

        const int minTrendActivationWaitRange = 0;
        const int maxTrendActivationWaitRange = 3;
        const int minStopLossWaitRange = 0;
        const int maxStopLossWaitRange = 3;

        const double stepFloat = 1.0;
        const int stepInt = 5;

        bool callbackInited = false;
    };
}
