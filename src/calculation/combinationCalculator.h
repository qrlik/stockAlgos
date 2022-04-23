#pragma once
#include "combinationFactory.h"
#include "algorithm/moneyMaker.h"
#include "market/indicators.h"
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
		const std::string ticker;
		const size_t threadsAmount = 0;
		const double weightPrecision = 0.0;
		const int parabolaDegree = 0;
		const int atrSizeDegree = 0;
		const eCandleInterval interval;

		double progress = 0.0;
		size_t combinations = 0;
	};

	struct finalData {
		std::string atrType;
		double cash = -1.0;
		double startCash = -1.0; // no output
		double maxLossPercent = -1.0;
		double RF = -1.0;
		double stFactor = -1.0;
		double dealPercent = -1.0;
		double stopLossPercent = -1.0;
		double minimumProfitPercent = -1.0;
		double dynamicSLPercent = -1.0;
		size_t profitableOrder = 0;
		size_t profitableStreak = 0;
		size_t unprofitableOrder = 0;
		size_t unprofitableStreak = 0;
		size_t touchTrendOrder = 0;
		size_t breakTrendOrder = 0;
		size_t longOrder = 0;
		size_t shortOrder = 0;
		int atrSize = 0;
		int leverage = 0;
		int activationWaiterModuleActivationWaitRange = 0;
		int stopLossWaiterModuleStopLossWaitRange = 0;
		bool dynamicStopLossTrendMode = false;
		bool trendTouchOpenerModuleActivationWaitMode = false;
		bool trendBreakOpenerModuleEnabled = false;
		bool trendBreakOpenerModuleActivationWaitMode = false;
		bool trendBreakOpenerModuleAlwaysUseNewTrend = false;
		bool activationWaiterModuleResetAllowed = false;
		bool activationWaiterModuleFullCandleCheck = false;
		bool stopLossWaiterModuleEnabled = false;
		bool stopLossWaiterModuleResetAllowed = false;
		bool stopLossWaiterModuleFullCandleCheck = false;
	};
}
