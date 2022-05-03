#pragma once
#include "combinationFactory.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "market/indicators.h"
#include <mutex>

namespace calculation {
	class calculationSystem {
	public:
		calculationSystem();
		void calculate();
	private:
		void loadSettings();
		void iterate(combinationFactory& aFactory, int aThread);
		void printProgress(size_t aIndex);
		void saveFinalData(const std::string& aTicker, eCandleInterval aInterval);

		std::vector<std::vector<Json>> threadsData;
		std::vector<candle> candlesSource;
		std::mutex printMutex;
		
		std::string algorithmType;
		std::vector<std::pair<std::string, eCandleInterval>> calculations;
		size_t threadsAmount = 0;
		double weightPrecision = 0.0;
		int parabolaDegree = 0;
		int atrSizeDegree = 0; // TO DO move it in indicator system inside algorithmn

		double progress = 0.0;
		size_t combinations = 0;
	};
}
