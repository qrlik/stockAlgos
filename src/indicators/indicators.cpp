#include "indicators.h"
#include "../utils/utils.h"
#include <deque>

using namespace indicators;

namespace {
    double lastEma = 0.0;
    double lastUpperBand = 0.0;
    double lastLowerBand = 0.0;
    double lastTrend = 0.0;
    double lastClose = 0.0;

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

    double calculateTrueRangeEMA(std::deque<double>& aTrList, double aAlpha) {
        auto ema = 0.0;
        if (lastEma == 0.0) {
            lastEma = aTrList[0];
            ema = lastEma;
            for (auto it = aTrList.begin() + 1, ite = aTrList.end(); it != ite; ++it) {
                ema = aAlpha * (*it) + (1 - aAlpha) * lastEma;
                lastEma = ema;
            }
        }
        else {
            ema = aAlpha * aTrList.back() + (1 - aAlpha) * lastEma;
            lastEma = ema;
        }
        return ema;
    }

    double calculateTrueRangeMA(std::deque<double>& aTrList, eAtrType aType) {
        switch (aType) {
        case indicators::eAtrType::SMA:
            return std::accumulate(aTrList.begin(), aTrList.end(), 0.0) / aTrList.size();
        case indicators::eAtrType::WMA:
            return calculateTrueRangeWMA(aTrList);
        case indicators::eAtrType::EMA:
            return calculateTrueRangeEMA(aTrList, 2.0 / (aTrList.size() + 1));
        case indicators::eAtrType::RMA:
            return calculateTrueRangeEMA(aTrList, 1.0 / aTrList.size());
        default:
            assert(false && "calculateTrueRangeMA NONE type");
            return 0.0;
        }
    }

    void calculateSuperTrend(candle& aCandle, double aFactor) {
        auto middlePrice = (aCandle.high + aCandle.low) / 2;

        auto upperBand = utils::round(middlePrice + aFactor * aCandle.atr, 2);
        auto lowerBand = utils::round(middlePrice - aFactor * aCandle.atr, 2);

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

    void resetStData() {
        lastUpperBand = 0.0;
        lastLowerBand = 0.0;
        lastTrend = 0.0;
        lastClose = 0.0;
    }
}

void indicators::calculateRangeAtr(std::vector<candle>& aCandles, eAtrType aType, size_t aSize) {
    std::deque<double> trList;
    for (size_t i = 1, size = aCandles.size(); i < size; ++i) {
        auto currentTrueRange = calculateTrueRange(aCandles[i], aCandles[i - 1]);
        trList.push_back(currentTrueRange);
        if (trList.size() > aSize) {
            trList.pop_front();
        }
        if (i >= aSize) {
            aCandles[i].atr = calculateTrueRangeMA(trList, aType);
        }
    }
    lastEma = 0.0;
}

void indicators::calculateSuperTrends(std::vector<candle>& aCandles, double aFactor) {
    for (auto& candle : aCandles) {
        calculateSuperTrend(candle, aFactor);
    }
    resetStData();
}