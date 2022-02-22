#include "dynamicStopLoss.h"
#include "moneyMaker.h"
#include "../utils/utils.h"

using namespace algorithm;

dynamicStopLoss::dynamicStopLoss(moneyMaker* aMm, double aPercent, bool aTrendMode): 
	mm(aMm),
	dynamicSLPercent(aPercent),
	trendMode(aTrendMode) {}

bool dynamicStopLoss::checkTrend() {
    auto& order = mm->getOrder();
    if (mm->getState() == moneyMaker::eState::LONG) {
        auto lastUpTrend = mm->getLastUpSuperTrend();
        if (lastUpTrend >= order.minimumProfit && lastUpTrend > order.stopLoss) {
            order.stopLoss = lastUpTrend;
            return true;
        }
    }
    else {
        auto lastDownTrend = mm->getLastDownSuperTrend();
        if (lastDownTrend <= order.minimumProfit && lastDownTrend < order.stopLoss) {
            order.stopLoss = lastDownTrend;
            return true;
        }
    }
    return false;
}

bool dynamicStopLoss::checkDynamic() {
    auto& order = mm->getOrder();
    const auto& candle = mm->getCandle();
    assert(dynamicSLPercent > 0.0);
    if (mm->getState() == moneyMaker::eState::LONG) {
        auto dynamicStopLoss = utils::floor(candle.high * (100.0 - dynamicSLPercent) / 100.0, 2);
        if (dynamicStopLoss >= order.minimumProfit && dynamicStopLoss > order.stopLoss) {
            order.stopLoss = dynamicStopLoss;
            return true;
        }
    }
    else {
        auto dynamicStopLoss = utils::ceil(candle.low * (100.0 + dynamicSLPercent) / 100.0, 2);
        if (dynamicStopLoss <= order.minimumProfit && dynamicStopLoss < order.stopLoss) {
            order.stopLoss = dynamicStopLoss;
            return true;
        }
    }
    return false;
}

bool dynamicStopLoss::check() {
	if (trendMode) {
		return checkTrend();
	}
	return checkDynamic();
}