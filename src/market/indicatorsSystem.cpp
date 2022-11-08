#include "indicatorsSystem.h"
#include "algorithm/algorithmDataBase.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace market;

indicatorsSystem::indicatorsSystem(const indicatorsData& aData, const std::string& ticker) :
	data(aData), marketData(MARKET_SYSTEM->getMarketData(ticker)) {}

bool indicatorsSystem::operator==(const indicatorsSystem& aOther) const {
	auto result = true;
	result &= utils::isEqual(utils::round(atr, marketData.getPricePrecision(0.0)), aOther.atr);

	result &= utils::isEqual(superTrend, aOther.superTrend);
	result &= trendIsUp == aOther.trendIsUp;

	result &= utils::isEqual(firstMA, aOther.firstMA);
	result &= utils::isEqual(secondMA, aOther.secondMA);

	result &= utils::isEqual(rsi, aOther.rsi, 0.01);
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
		else if (key == "rsi") {
			rsi = value.get<double>();
		}
	}
}

namespace {
	double calculateTrueRange(const candle& aCandle, const candle& aPrevCandle) {
		auto candleSize = aCandle.high - aCandle.low;
		auto highDelta = std::abs(aCandle.high - aPrevCandle.close);
		auto lowDelta = std::abs(aCandle.low - aPrevCandle.close);
		return utils::maxFloat(candleSize, utils::maxFloat(highDelta, lowDelta));
	}

	double calculateCloseDelta(const candle& aCandle, const candle& aPrevCandle) {
		return aCandle.close - aPrevCandle.close;
	}

	double calculateWMA(const std::deque<double>& aList) {
		int weight = 0;
		double trAndWeightSum = 0.0;
		for (auto tr : aList) {
			++weight;
			trAndWeightSum += weight * tr;
		}
		std::vector<int> weights(weight + 1);
		std::iota(weights.begin(), weights.end(), 0);

		return trAndWeightSum / std::accumulate(weights.begin(), weights.end(), 0);
	}

	double calculateEMA(const std::deque<double>& aList, double aAlpha, bool aIsFirst, double lastValue) {
		if (aIsFirst) { // prevCandle.time.empty()
			return aList.back();
		}
		return aAlpha * aList.back() + (1 - aAlpha) * lastValue;
	}

	double calculateCustomMASwitch(market::eAtrType aType, const std::deque<double>& aList, bool aIsFirst, double lastValue) {
		switch (aType) {
		case market::eAtrType::SMA:
			return std::accumulate(aList.begin(), aList.end(), 0.0) / aList.size();
		case market::eAtrType::WMA:
			return calculateWMA(aList);
		case market::eAtrType::EMA:
			return calculateEMA(aList, 2.0 / (aList.size() + 1), aIsFirst, lastValue);
		case market::eAtrType::RMA:
			return calculateEMA(aList, 1.0 / aList.size(), aIsFirst, lastValue);
		default:
			assert(false && "calculateTrueRangeMA NONE type");
			return 0.0;
		}
	}
}

double indicatorsSystem::calculateCustomMA(market::eAtrType aType, const std::deque<double>& aList, double lastValue) const {
	return calculateCustomMASwitch(aType, aList, prevCandle.time.empty(), lastValue);
}

bool indicatorsSystem::calculateRSI(candle& aCandle) {
	if (!data.isRSI()) {
		return true;
	}
	const auto closeDelta = calculateCloseDelta(aCandle, (prevCandle.time.empty() ? aCandle : prevCandle));
	const auto upDelta = (utils::isGreater(closeDelta, 0.0)) ? closeDelta : 0.0;
	const auto downDelta = (utils::isLess(closeDelta, 0.0)) ? std::abs(closeDelta) : 0.0;
	upList.push_back(upDelta);
	downList.push_back(downDelta);
	if (static_cast<int>(upList.size()) > data.getRsiSize()) {
		upList.pop_front();
		downList.pop_front();
	}
	upMa = calculateCustomMA(market::eAtrType::RMA, upList, upMa);
	downMa = calculateCustomMA(market::eAtrType::RMA, downList, downMa);
	const auto rs = (utils::isGreater(downMa, 0.0)) ? upMa / downMa : 0.0;
	rsi = (utils::isGreater(rs, 0.0)) ? 100.0 - 100.0 / (1.0 + rs) : 100.0;
	return static_cast<int>(upList.size()) >= data.getRsiSize();
}

void indicatorsSystem::calculateSuperTrend(candle& aCandle) {
	if (!data.isSuperTrend()) {
		return;
	}
	auto middlePrice = (aCandle.high + aCandle.low) / 2;

	const auto pricePrecision = marketData.getPricePrecision(aCandle.high);
	auto upperBand = utils::round(middlePrice + data.getStFactor() * atr, pricePrecision);
	auto lowerBand = utils::round(middlePrice - data.getStFactor() * atr, pricePrecision);

	upperBand = (utils::isLess(upperBand, lastUpperBand) || utils::isGreater(prevCandle.close, lastUpperBand)) ? upperBand : lastUpperBand;
	lowerBand = (utils::isGreater(lowerBand, lastLowerBand) || utils::isLess(prevCandle.close, lastLowerBand)) ? lowerBand : lastLowerBand;

	if (utils::isEqual(superTrend, lastUpperBand)) {
		trendIsUp = utils::isGreater(aCandle.close, upperBand);
	}
	else {
		trendIsUp = utils::isGreater(aCandle.close, lowerBand);
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
	atr = calculateCustomMA(data.getAtrType(), trList, atr);
	return static_cast<int>(trList.size()) >= data.getAtrSize();
}

bool indicatorsSystem::calculateMA(candle& aCandle) {
	if (!data.isMA()) {
		return true;
	}
	closeList.push_back(aCandle.close);
	firstMASum += aCandle.close;
	secondMASum += aCandle.close;
	if (closeList.size() > data.getFirstMASize()) {
		firstMASum -= closeList[closeList.size() - 1 - data.getFirstMASize()];
	}
	if (closeList.size() > data.getSecondMASize()) {
		secondMASum -= closeList[closeList.size() - 1 - data.getSecondMASize()];
	}
	if (closeList.size() < std::max(data.getFirstMASize(), data.getSecondMASize())) {
		return false;
	}
	if (closeList.size() > std::max(data.getFirstMASize(), data.getSecondMASize())) {
		closeList.pop_front();
	}
	firstMA = (data.getFirstMASize() > 0) ? firstMASum / data.getFirstMASize() : 0.0;
	secondMA = (data.getSecondMASize() > 0) ? secondMASum / data.getSecondMASize() : 0.0;
	firstMA = utils::round(firstMA, marketData.getPricePrecision(aCandle.high));
	secondMA = utils::round(secondMA, marketData.getPricePrecision(aCandle.high));
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
	result &= calculateRSI(aCandle);
	result &= checkSkip();
	if (result != inited) {
		if (inited) {
			utils::logError("indicatorsSystem::processCandle wrong inited state");
		}
		inited = result;
	}
	prevCandle = aCandle;
}