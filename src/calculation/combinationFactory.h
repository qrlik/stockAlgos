#pragma once
#include "../structs/algorithmData.h"
#include <functional>
#include <optional>

namespace calculation {

	class combinationFactory {
	public:
        using iterateCallback = std::function<void(const algorithmData&, size_t)>;
        combinationFactory(size_t aThreadsCount);
        void iterateCombination(int aPosition, iterateCallback aCallback);
        size_t getCombinationsAmount() const;
	private:
        size_t getCurrentIndex() const;
        void generateSuperTrend(int aPosition);
        void generateSuperTrend();
        void generateDeal(int aPosition);
        void generatePercent(int aPosition);
        void generateDynamicSL(int aPosition);
        void generateOpener(int aPosition);
        void generateActivation(int aPosition);
        void generateStop(int aPosition);
        void onIterate(int aPosition);

        std::vector<std::vector<algorithmData>> combinationsData;
        std::vector<algorithmData> data;
        std::vector<iterateCallback> callbacks;
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

        const double minMinProfitPercent = 0.5;
        const double maxMinProfitPercent = 2.5;

        const double minActivationPercent = 0.0; // <= liqudation
        const double minStopLossPercent = 0.5; // <= liqudation
        const double minDynamicSLPercent = 0.5;
        const double maxDynamicSLPercent = 2.0; // <= liqudation

        const int minTrendActivationWaitRange = 0;
        const int maxTrendActivationWaitRange = 0;
        const int minStopLossWaitRange = 0;
        const int maxStopLossWaitRange = 0;

        const double stepFloat = 0.5; // 0.25
        const int stepInt = 5;

        bool inited = false;
    };
}
