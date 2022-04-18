#pragma once
#include "combinationFactory.h"
#include "../algorithm/moneyMaker.h"
#include "../market/indicators.h"
#include <mutex>

namespace calculation {
	struct finalData;
	class calculationSystem {
	public:
		calculationSystem(const std::string& aTicker, eCandleInterval aInterval);
		void calculate();
	private:
		void iterate(combinationFactory& aFactory, int aThread);
		void printProgress(size_t aIndex);
		void saveFinalData();

		finalData getData(const algorithm::moneyMaker& aMM);

		struct threadInfo {
			bool isCached(market::eAtrType aType, int aSize, double aFactor);
			void saveCache(market::eAtrType aType, int aSize, double aFactor);

			std::vector<finalData> finalData;
			double cachedStFactor = -1.0;
			int cachedAtrSize = -1;
			market::eAtrType cachedAtrType = market::eAtrType::NONE;
		};

		std::vector<threadInfo> threadsData;
		std::vector<candle> candlesSource;
		std::mutex printMutex;
		const size_t threadsCount = 8;
		const double weightPrecision = 0.0001;
		const int parabolaDegree = 7;
		const eCandleInterval interval;

		double progress = 0.0;
		size_t combinations = 0;
	};

	struct finalData {
		std::string atrType;
		double cash;
		double startCash; // no output
		double maxLossPercent;
		double RF;
		double stFactor;
		double dealPercent;
		double stopLossPercent;
		double minimumProfitPercent;
		double dynamicSLPercent;
		size_t profitableOrder;
		size_t profitableStreak;
		size_t unprofitableOrder;
		size_t unprofitableStreak;
		size_t touchTrendOrder;
		size_t breakTrendOrder;
		size_t longOrder;
		size_t shortOrder;
		int atrSize;
		int leverage;
		int activationWaiterModuleActivationWaitRange;
		int stopLossWaiterModuleStopLossWaitRange;
		bool dynamicStopLossTrendMode;
		bool trendTouchOpenerModuleActivationWaitMode;
		bool trendBreakOpenerModuleEnabled;
		bool trendBreakOpenerModuleActivationWaitMode;
		bool trendBreakOpenerModuleAlwaysUseNewTrend;
		bool activationWaiterModuleResetAllowed;
		bool activationWaiterModuleFullCandleCheck;
		bool stopLossWaiterModuleEnabled;
		bool stopLossWaiterModuleResetAllowed;
		bool stopLossWaiterModuleFullCandleCheck;
	};
}
