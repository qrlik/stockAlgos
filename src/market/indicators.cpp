#include "indicators.h"
#include "../utils/utils.h"
#include <deque>

using namespace market;

std::string market::atrTypeToString(eAtrType aType) {
    switch (aType) {
    case market::eAtrType::RMA:
        return "RMA";
    case market::eAtrType::EMA:
        return "EMA";
    case market::eAtrType::WMA:
        return "WMA";
    case market::eAtrType::SMA:
        return "SMA";
    default:
        return "NONE";
    }
}

eAtrType market::atrTypeFromString(const std::string& aStr) {
    if (aStr == "RMA") {
        return market::eAtrType::RMA;
    }
    else if (aStr == "EMA") {
        return market::eAtrType::EMA;
    }
    else if (aStr == "WMA") {
        return market::eAtrType::WMA;
    }
    else if (aStr == "SMA") {
        return market::eAtrType::SMA;
    }
    return market::eAtrType::NONE;
}

indicatorSystem::indicatorSystem(eAtrType aType, int aSize, double aStFactor):
    atrType(aType),
    atrSize(aSize),
    stFactor(aStFactor) {}

namespace {
    double calculateTrueRange(const candle& aCandle, const candle& aPrevCandle) {
        auto candleSize = aCandle.high - aCandle.low;
        auto highDelta = std::abs(aCandle.high - aPrevCandle.close);
        auto lowDelta = std::abs(aCandle.low - aPrevCandle.close);
        return std::max(candleSize, std::max(highDelta, lowDelta));
    }
}

double indicatorSystem::calculateTrueRangeWMA() const {
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

double indicatorSystem::calculateTrueRangeEMA(double aAlpha) {
    auto ema = 0.0;
    if (lastEma == 0.0) {
        lastEma = trList[0];
        ema = lastEma;
        for (auto it = trList.begin() + 1, ite = trList.end(); it != ite; ++it) {
            ema = aAlpha * (*it) + (1 - aAlpha) * lastEma;
            lastEma = ema;
        }
    }
    else {
        ema = aAlpha * trList.back() + (1 - aAlpha) * lastEma;
        lastEma = ema;
    }
    return ema;
}

double indicatorSystem::calculateTrueRangeMA() {
    switch (atrType) {
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

void indicatorSystem::calculateSuperTrend(candle& aCandle) {
    auto middlePrice = (aCandle.high + aCandle.low) / 2;

    auto upperBand = utils::round(middlePrice + stFactor * aCandle.atr, 2);
    auto lowerBand = utils::round(middlePrice - stFactor * aCandle.atr, 2);

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

void indicatorSystem::calculateRangeAtr(candle& aCandle) {
    if (prevCandle.time.empty()) {
        return;
    }
    auto currentTrueRange = calculateTrueRange(aCandle, prevCandle);
    trList.push_back(currentTrueRange);
    if (static_cast<int>(trList.size()) > atrSize) {
        trList.pop_front();
    }
    aCandle.atr = calculateTrueRangeMA();
}

void indicatorSystem::processCandle(candle& aCandle) {
    calculateRangeAtr(aCandle);
    calculateSuperTrend(aCandle);
    prevCandle = aCandle;
}

void indicatorSystem::getProcessedCandles(std::vector<candle>& aCandles, int aAmount) {
    for (auto& candle : aCandles) {
        processCandle(candle);
    }
    if (aAmount == 0 || aAmount >= static_cast<int>(aCandles.size())) {
        return;
    }
    auto eraseSize = aCandles.size() - aAmount;
    aCandles.erase(aCandles.begin(), aCandles.begin() + eraseSize);
}