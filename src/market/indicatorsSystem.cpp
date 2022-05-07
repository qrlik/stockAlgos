#include "indicatorsSystem.h"
#include "indicatorsData.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace market;

indicatorsSystem::indicatorsSystem(const indicatorsData& aData): data(aData) {}

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
	return aAlpha * trList.back() + (1 - aAlpha) * prevCandle.atr;
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
	auto upperBand = utils::round(middlePrice + data.getStFactor() * aCandle.atr, pricePrecision);
	auto lowerBand = utils::round(middlePrice - data.getStFactor() * aCandle.atr, pricePrecision);

	upperBand = (upperBand < lastUpperBand || lastClose > lastUpperBand) ? upperBand : lastUpperBand;
	lowerBand = (lowerBand > lastLowerBand || lastClose < lastLowerBand) ? lowerBand : lastLowerBand;

	auto trendIsUp = false;
	if (lastTrend == lastUpperBand) {
		trendIsUp = aCandle.close > upperBand;
	}
	else {
		trendIsUp = aCandle.close > lowerBand;
	}

	lastTrend = (trendIsUp) ? lowerBand : upperBand;
	lastUpperBand = upperBand;
	lastLowerBand = lowerBand;
	lastClose = aCandle.close;
	aCandle.superTrend = lastTrend;
	aCandle.trendIsUp = trendIsUp;
}

void indicatorsSystem::calculateRangeAtr(candle& aCandle) {
	if (!data.isAtr()) {
		return;
	}
	auto currentTrueRange = calculateTrueRange(aCandle, (prevCandle.time.empty() ? aCandle : prevCandle));
	trList.push_back(currentTrueRange);
	if (static_cast<int>(trList.size()) > data.getAtrSize()) {
		trList.pop_front();
	}
	aCandle.atr = calculateTrueRangeMA();
	prevCandle = aCandle;
}

void indicatorsSystem::processCandle(candle& aCandle) {
	calculateRangeAtr(aCandle);
	calculateSuperTrend(aCandle);
}

void indicatorsSystem::getProcessedCandles(std::vector<candle>& aCandles, int aAmount) {
	for (auto& candle : aCandles) {
		processCandle(candle);
	}
	if (aAmount == 0 || aAmount >= static_cast<int>(aCandles.size())) {
		return;
	}
	auto eraseSize = aCandles.size() - aAmount;
	aCandles.erase(aCandles.begin(), aCandles.begin() + eraseSize);
}