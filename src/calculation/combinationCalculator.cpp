#include "combinationCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../utils/utils.h"
#include <iostream>
#include <future>

using namespace calculation;

calculationSystem::calculationSystem() {
	auto json = utils::readFromJson("assets/candles/1h_year");
	candlesSource = utils::parseCandles(json);
	threadsData = std::vector<threadInfo>(threadCount);
}

bool calculationSystem::threadInfo::isCached(indicators::eAtrType aType, int aSize, double aFactor) {
	return aType == cachedAtrType && aSize == cachedAtrSize && aFactor == cachedStFactor;
}

void calculationSystem::threadInfo::saveCache(indicators::eAtrType aType, int aSize, double aFactor) {
	cachedAtrType = aType;
	cachedAtrSize = aSize;
	cachedStFactor = aFactor;
}

void calculationSystem::calculate() {
	std::vector<std::future<void>> futures;
	auto factory = combinationFactory();
	combinations = factory.getCombinationsAmount();
	factory.reset();
	for (auto i = 0; i < threadCount; ++i) {
		futures.push_back(std::async(std::launch::async, [this, &factory, i]() {return iterate(factory, i); }));
	}
	for (auto& future : futures) {
		future.wait();
	}
}

void calculationSystem::iterate(combinationFactory& aFactory, int aThread) {
	std::vector<candle> candles;
	aFactory.iterateCombination(aThread, [this, &candles, aThread](const algorithmData& aData, size_t aIndex) {
		auto& threadInfo = threadsData[aThread];
		if (!threadInfo.isCached(aData.atrType, aData.atrSize, aData.stFactor)) {
			candles = candlesSource;
			indicators::getProcessedCandles(candles, aData.atrType, aData.atrSize, aData.stFactor, 8760);
			threadInfo.saveCache(aData.atrType, aData.atrSize, aData.stFactor);
		}
		auto moneyMaker = algorithm::moneyMaker(aData, 100.0);
		for (const auto& candle : candles) {
			if (!moneyMaker.doAction(candle)) {
				break;
			}
		}
		printProgress(aIndex);
	});
}

void calculationSystem::printProgress(size_t aIndex) {
	const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations, 1);
	if (newProgress > progress) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		std::cout << std::to_string(progress) + "%\n";
	}
}