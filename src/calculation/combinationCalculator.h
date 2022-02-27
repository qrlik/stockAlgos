#pragma once
#include "combinationFactory.h"
#include "../indicators/indicators.h"
#include <mutex>

namespace calculation {
	class calculationSystem {
	public:
		calculationSystem();
		void calculate();
	private:
		void iterate(combinationFactory& aFactory, int aThread);
		void printProgress(size_t aIndex);
		void saveFinalData();

		struct threadInfo {
			bool isCached(indicators::eAtrType aType, int aSize, double aFactor);
			void saveCache(indicators::eAtrType aType, int aSize, double aFactor);

			Json finalData;
			double cachedStFactor = -1.0;
			int cachedAtrSize = -1;
			indicators::eAtrType cachedAtrType = indicators::eAtrType::NONE;
		};

		std::vector<threadInfo> threadsData;
		std::vector<candle> candlesSource;
		std::mutex printMutex;
		const int threadCount = 8;

		double progress = 0.0;
		size_t combinations = 0;
	};
}
