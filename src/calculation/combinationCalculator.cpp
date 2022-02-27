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
	saveFinalData();
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
		moneyMaker.calculate(candles);
		threadInfo.finalData.push_back(moneyMaker.getFinalData());
		printProgress(aIndex);
	});
}

void calculationSystem::printProgress(size_t aIndex) {
	const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations, 2) * 100;
	if (newProgress > progress) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		std::cout << std::to_string(progress) + "%\n";
	}
}

void calculationSystem::saveFinalData() {
	Json value;
	for (auto& data : threadsData) {
		std::move(data.finalData.begin(), data.finalData.end(), std::back_inserter(value));
	}
	utils::saveToJson("finalData", value);
}