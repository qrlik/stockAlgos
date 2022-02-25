#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>

namespace calculation {

	class combinationFactory {
	public:
        combinationFactory();
        void iterateCombination(std::function<void(const algorithmData&)> aCallback);
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
        std::function<void(const algorithmData&)> callback;
        size_t combinations = 0;

        int minAtrSize = 15;
        int maxAtrSize = 15; // 25;
        double minStFactor = 3.0;
        double maxStFactor = 3.0; //5.0;

        double minDealPercent = 5.0; // 1.0;
        double maxDealPercent = 5.0;
        int minLeverage = 50;
        int maxLeverage = 50;

        double minMinProfitPercent = 0.5;// 1.0;
        double maxMinProfitPercent = 0.5;// 2.0;

        double minActivationPercent = 0.0;
        double minStopLossPercent = 1.0;
        double minDynamicSLPercent = 1.0;

        int minTrendActivationWaitRange = 0;
        int maxTrendActivationWaitRange = 3;
        int minStopLossWaitRange = 0;
        int maxStopLossWaitRange = 3;

        double stepFloat = 1.0;
        int stepInt = 5;

        bool callbackInited = false;
    };
}
