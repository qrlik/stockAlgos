#include "dynamicStopLoss.h"
#include "moneyMaker.h"
#include "../market/marketRules.h"
#include "../utils/utils.h"

using namespace algorithm;

dynamicStopLoss::dynamicStopLoss(moneyMaker* aMm, double aPercent, bool aTrendMode): 
	mm(aMm),
	dynamicSLPercent(aPercent),
	trendMode(aTrendMode) {}

bool dynamicStopLoss::checkTrend() {
    auto& order = mm->getOrder();
    if (mm->getState() == eState::LONG) {
        auto lastUpTrend = mm->getLastUpSuperTrend();
        if (lastUpTrend >= order.getMinimumProfit() && lastUpTrend > order.getStopLoss()) {
            order.updateStopLoss(lastUpTrend);
            return true;
        }
    }
    else {
        auto lastDownTrend = mm->getLastDownSuperTrend();
        if (lastDownTrend <= order.getMinimumProfit() && lastDownTrend < order.getStopLoss()) {
            order.updateStopLoss(lastDownTrend);
            return true;
        }
    }
    return false;
}

bool dynamicStopLoss::checkDynamic() {
    auto& order = mm->getOrder();
    const auto& candle = mm->getCandle();
    assert(dynamicSLPercent > 0.0);
    const auto pricePrecision = MARKET_DATA->getPricePrecision();
    if (mm->getState() == eState::LONG) {
        auto dynamicStopLoss = utils::round(candle.high * (100.0 - dynamicSLPercent) / 100.0, pricePrecision);
        if (dynamicStopLoss >= order.getMinimumProfit() && dynamicStopLoss > order.getStopLoss()) {
            order.updateStopLoss(dynamicStopLoss);
            return true;
        }
    }
    else {
        auto dynamicStopLoss = utils::round(candle.low * (100.0 + dynamicSLPercent) / 100.0, pricePrecision);
        if (dynamicStopLoss <= order.getMinimumProfit() && dynamicStopLoss < order.getStopLoss()) {
            order.updateStopLoss(dynamicStopLoss);
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