#include "combinationCalculator.h"
#include "combinationFactory.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../utils/utils.h"
#include <iostream>

using namespace calculation;

namespace {
	Json finalData;
	indicators::eAtrType cachedAtrType = indicators::eAtrType::NONE;
	int cachedAtrSize = -1;
	double cachedStFactor = -1.0;
	double progress = 0.0;

	bool isCached(indicators::eAtrType aType, int aSize, double aFactor) {
		return aType == cachedAtrType && aSize == cachedAtrSize && aFactor == cachedStFactor;
	}

	void saveCache(indicators::eAtrType aType, int aSize, double aFactor) {
		cachedAtrType = aType;
		cachedAtrSize = aSize;
		cachedStFactor = aFactor;
	}
}

void calculation::calculate() {
	auto json = utils::readFromJson("assets/candles/1h_year");
	auto candlesSource = utils::parseCandles(json);
	std::vector<candle> candles;
	combinationFactory factory;
	auto combinations = factory.getCombinationsAmount();
	factory.iterateCombination([combinations, &candlesSource, &candles](const algorithmData& aData, size_t aIndex) {
		if (!isCached(aData.atrType, aData.atrSize, aData.stFactor)) {
			candles = candlesSource;
			indicators::getProcessedCandles(candles, aData.atrType, aData.atrSize, aData.stFactor, 8760);
			saveCache(aData.atrType, aData.atrSize, aData.stFactor);
		}
		auto moneyMaker = algorithm::moneyMaker(aData, 100.0);
		for (const auto& candle : candles) {
			if (!moneyMaker.doAction(candle)) {
				break;
			}
		}
		finalData.push_back(moneyMaker.getFinalData());
		const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations, 2) * 100;
		if (newProgress > progress) {
			progress = newProgress;
			std::cout << std::to_string(progress) + "%\n";
		}
	});
	std::cout << std::to_string(finalData.size()) << '\n';
	utils::saveToJson("finalData", finalData);
}