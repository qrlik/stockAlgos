#include "indicatorsSystem.h"
#include "algorithm/algorithmDataBase.h"
#include "indicatorsData.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace market;

indicatorsSystem::indicatorsSystem(const indicatorsData& aData): data(aData) {}

bool indicatorsSystem::operator==(const indicatorsSystem& aOther) const {
	auto result = true;
	result &= utils::isEqual(atr, aOther.atr);

	result &= utils::isEqual(superTrend, aOther.superTrend);
	result &= trendIsUp == aOther.trendIsUp;

	result &= utils::isEqual(firstMA, aOther.firstMA);
	result &= utils::isEqual(secondMA, aOther.secondMA);
	return result;
}

void indicatorsSystem::initFromJson(const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	for (const auto& [key, value] : aValue.items()) {
		if (value.is_null()) {
			continue;
		}
		if (key == "atr") {
			atr = value.get<double>();
		}
		else if (key == "superTrend") {
			superTrend = value.get<double>();
		}
		else if (key == "trendIsUp") {
			trendIsUp = value.get<bool>();
		}
		else if (key == "firstMA") {
			firstMA = value.get<double>();
		}
		else if (key == "secondMA") {
			secondMA = value.get<double>();
		}
	}
}

namespace {
	double calculateTrueRange(const candle& aCandle, const candle& aPrevCandle) {
		auto candleSize = aCandle.high - aCandle.low;
		auto highDelta = std::abs(aCandle.high - aPrevCandle.close);
		auto lowDelta = std::abs(aCandle.low - aPrevCandle.close);
		return std::max(candleSize, std::max(highDelta, lowDelta));
	}
}

double indicatorsSystem::calculateTrueRangeWMA() const {
	int weight = 0;
	double trAndWeightSum = 0.0;
	for (auto tr : trList) {
		++weight;
		trAndWeightSum += weight * tr;
	}
	std::vector<int> weights(weight + 1);
	std::iota(weights.begin(), weights.end(), 0);

	return trAndWeightSum / std::accumulate(weights.begin(), weights.end(), 0);
}

double indicatorsSystem::calculateTrueRangeEMA(double aAlpha) {
	if (prevCandle.time.empty()) {
		return trList.back();
	}
	return aAlpha * trList.back() + (1 - aAlpha) * atr;
}

double indicatorsSystem::calculateTrueRangeMA() {
	switch (data.getAtrType()) {
	case market::eAtrType::SMA:
		return std::accumulate(trList.begin(), trList.end(), 0.0) / trList.size();
	case market::eAtrType::WMA:
		return calculateTrueRangeWMA();
	case market::eAtrType::EMA:
		return calculateTrueRangeEMA(2.0 / (trList.size() + 1));
	case market::eAtrType::RMA:
		return calculateTrueRangeEMA(1.0 / trList.size());
	default:
		assert(false && "calculateTrueRangeMA NONE type");
		return 0.0;
	}
}

void indicatorsSystem::calculateSuperTrend(candle& aCandle) {
	if (!data.isSuperTrend()) {
		return;
	}
	auto middlePrice = (aCandle.high + aCandle.low) / 2;

	const auto pricePrecision = MARKET_DATA->getPricePrecision();
	auto upperBand = utils::round(middlePrice + data.getStFactor() * atr, pricePrecision);
	auto lowerBand = utils::round(middlePrice - data.getStFactor() * atr, pricePrecision);

	upperBand = (upperBand < lastUpperBand || prevCandle.close > lastUpperBand) ? upperBand : lastUpperBand;
	lowerBand = (lowerBand > lastLowerBand || prevCandle.close < lastLowerBand) ? lowerBand : lastLowerBand;

	if (utils::isEqual(superTrend, lastUpperBand)) {
		trendIsUp = aCandle.close > upperBand;
	}
	else {
		trendIsUp = aCandle.close > lowerBand;
	}

	lastUpperBand = upperBand;
	lastLowerBand = lowerBand;
	superTrend = (trendIsUp) ? lowerBand : upperBand;
}

bool indicatorsSystem::calculateRangeAtr(candle& aCandle) {
	if (!data.isAtr()) {
		return true;
	}
	auto currentTrueRange = calculateTrueRange(aCandle, (prevCandle.time.empty() ? aCandle : prevCandle));
	trList.push_back(currentTrueRange);
	if (static_cast<int>(trList.size()) > data.getAtrSize()) {
		trList.pop_front();
	}
	atr = calculateTrueRangeMA();
	return static_cast<int>(trList.size()) >= data.getAtrSize();
}

bool indicatorsSystem::calculateMA(candle& aCandle) {
	if (!data.isMA()) {
		return true;
	}
	closeList.push_back(aCandle.close);
	firstMASum += aCandle.close;
	secondMASum += aCandle.close;
	if (closeList.size() > data.getFirstMA()) {
		firstMASum -= closeList[closeList.size() - 1 - data.getFirstMA()];
	}
	if (closeList.size() > data.getSecondMA()) {
		secondMASum -= closeList[closeList.size() - 1 - data.getSecondMA()];
	}
	if (closeList.size() < std::max(data.getFirstMA(), data.getSecondMA())) {
		return false;
	}
	if (closeList.size() > std::max(data.getFirstMA(), data.getSecondMA())) {
		closeList.pop_front();
	}
	firstMA = (data.getFirstMA() > 0) ? firstMASum / data.getFirstMA() : 0.0;
	secondMA = (data.getSecondMA() > 0) ? secondMASum / data.getSecondMA() : 0.0;
	firstMA = utils::round(firstMA, MARKET_DATA->getPricePrecision());
	secondMA = utils::round(secondMA, MARKET_DATA->getPricePrecision());
	return true;
}

bool indicatorsSystem::checkSkip() {
	++candlesCounter;
	return candlesCounter >= data.getSkipAmount();
}

void indicatorsSystem::processCandle(candle& aCandle) {
	auto result = true;
	result &= calculateMA(aCandle);
	result &= calculateRangeAtr(aCandle);
	calculateSuperTrend(aCandle);
	result &= checkSkip();
	if (result != inited) {
		if (inited) {
			utils::logError("indicatorsSystem::processCandle wrong inited state");
		}
		inited = result;
	}
	prevCandle = aCandle;
}