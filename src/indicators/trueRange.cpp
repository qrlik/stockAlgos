#include "trueRange.h"
#include <deque>

using namespace indicators;

namespace {
    double lastEma = 0.0;

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
        default:
            return calculateTrueRangeEMA(aTrList, 1.0 / aTrList.size());
        }
    }
}

void indicators::calculateRangeAtr(std::vector<candle>& aCandles, eAtrType aType, size_t aSize) {
    std::deque<double> trList;
    size_t index = 1;
    while (index < aCandles.size()) {
        auto currentTrueRange = calculateTrueRange(aCandles[index], aCandles[index - 1]);
        trList.push_back(currentTrueRange);
        if (trList.size() > aSize) {
            trList.pop_front();
        }
        if (index >= aSize) {
            aCandles[index].atr = calculateTrueRangeMA(trList, aType);
        }
        index += 1;
    }
    lastEma = 0.0;
}