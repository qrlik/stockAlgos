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

		struct threadInfo {
			bool isCached(indicators::eAtrType aType, int aSize, double aFactor);
			void saveCache(indicators::eAtrType aType, int aSize, double aFactor);

			double cachedStFactor = -1.0;
			int cachedAtrSize = -1;
			indicators::eAtrType cachedAtrType = indicators::eAtrType::NONE;
		};

		std::vector<candle> candlesSource;
		std::vector<threadInfo> threadsData;
		std::mutex printMutex;
		const int threadCount = 8;

		double progress = 0.0;
		int combinations = 0;
	};
}
