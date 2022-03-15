#include "indicators.h"
#include "../utils/utils.h"
#include <deque>

using namespace market;

namespace {
    double calculateTrueRange(const candle& aCandle, const candle& aPrevCandle) {
        auto candleSize = aCandle.high - aCandle.low;
        auto highDelta = std::abs(aCandle.high - aPrevCandle.close);
        auto lowDelta = std::abs(aCandle.low - aPrevCandle.close);
        return std::max(candleSize, std::max(highDelta, lowDelta));
    }

    double calculateTrueRangeWMA(std::deque<double>& aTrList) {
        int weight = 0;
        double trAndWeightSum = 0.0;
        for (auto tr : aTrList) {
            ++weight;
            trAndWeightSum += weight * tr;
        }
        std::vector<int> weights(weight + 1);
        std::iota(weights.begin(), weights.end(), 0);

        return trAndWeightSum / std::accumulate(weights.begin(), weights.end(), 0);
    }

    double calculateTrueRangeEMA(indicatorsData& aData, std::deque<double>& aTrList, double aAlpha) {
        auto ema = 0.0;
        if (aData.lastEma == 0.0) {
            aData.lastEma = aTrList[0];
            ema = aData.lastEma;
            for (auto it = aTrList.begin() + 1, ite = aTrList.end(); it != ite; ++it) {
                ema = aAlpha * (*it) + (1 - aAlpha) * aData.lastEma;
                aData.lastEma = ema;
            }
        }
        else {
            ema = aAlpha * aTrList.back() + (1 - aAlpha) * aData.lastEma;
            aData.lastEma = ema;
        }
        return ema;
    }

    double calculateTrueRangeMA(indicatorsData& aData, std::deque<double>& aTrList, eAtrType aType) {
        switch (aType) {
        case market::eAtrType::SMA:
            return std::accumulate(aTrList.begin(), aTrList.end(), 0.0) / aTrList.size();
        case market::eAtrType::WMA:
            return calculateTrueRangeWMA(aTrList);
        case market::eAtrType::EMA:
            return calculateTrueRangeEMA(aData, aTrList, 2.0 / (aTrList.size() + 1));
        case market::eAtrType::RMA:
            return calculateTrueRangeEMA(aData, aTrList, 1.0 / aTrList.size());
        default:
            assert(false && "calculateTrueRangeMA NONE type");
            return 0.0;
        }
    }

    void calculateSuperTrend(indicatorsData& aData, candle& aCandle, double aFactor) {
        auto middlePrice = (aCandle.high + aCandle.low) / 2;

        auto upperBand = utils::round(middlePrice + aFactor * aCandle.atr, 2);
        auto lowerBand = utils::round(middlePrice - aFactor * aCandle.atr, 2);

        upperBand = (upperBand < aData.lastUpperBand || aData.lastClose > aData.lastUpperBand) ? upperBand : aData.lastUpperBand;
        lowerBand = (lowerBand > aData.lastLowerBand || aData.lastClose < aData.lastLowerBand) ? lowerBand : aData.lastLowerBand;

        auto trendIsUp = false;
        if (aData.lastTrend == aData.lastUpperBand) {
            trendIsUp = aCandle.close > upperBand;
        }
        else {
            trendIsUp = aCandle.close > lowerBand;
        }

        aData.lastTrend = (trendIsUp) ? lowerBand : upperBand;
        aData.lastUpperBand = upperBand;
        aData.lastLowerBand = lowerBand;
        aData.lastClose = aCandle.close;
        aCandle.superTrend = aData.lastTrend;
        aCandle.trendIsUp = trendIsUp;
    }

    void calculateRangeAtr(indicatorsData& aData, std::vector<candle>& aCandles, eAtrType aType, size_t aSize) {
        std::deque<double> trList;
        for (size_t i = 1, size = aCandles.size(); i < size; ++i) {
            auto currentTrueRange = calculateTrueRange(aCandles[i], aCandles[i - 1]);
            trList.push_back(currentTrueRange);
            if (trList.size() > aSize) {
                trList.pop_front();
            }
            if (i >= aSize) {
                aCandles[i].atr = calculateTrueRangeMA(aData, trList, aType);
            }
        }
    }

    void calculateSuperTrends(indicatorsData& aData, std::vector<candle>& aCandles, double aFactor) {
        for (auto& candle : aCandles) {
            calculateSuperTrend(aData, candle, aFactor);
        }
    }
}

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

void market::getProcessedCandles(std::vector<candle>& aCandles, eAtrType aType, size_t aSize, double aFactor, size_t aAmount) {
    auto data = indicatorsData();
    calculateRangeAtr(data, aCandles, aType, aSize);
    calculateSuperTrends(data, aCandles, aFactor);
    if (aAmount >= aCandles.size() || aAmount == 0) {
        return;
    }
    auto eraseSize = aCandles.size() - aAmount;
    aCandles.erase(aCandles.begin(), aCandles.begin() + eraseSize);
}