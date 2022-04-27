#include "dynamicStopLoss.h"
#include "stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

dynamicStopLoss::dynamicStopLoss(stAlgorithm& aAlgorithm):
	algorithm(aAlgorithm) {}

bool dynamicStopLoss::checkTrend() {
	const auto& order = algorithm.getOrder();
	if (algorithm.getState() == eState::LONG) {
		auto lastUpTrend = algorithm.getLastUpSuperTrend();
		if (lastUpTrend >= order.getMinimumProfit() && lastUpTrend > order.getStopLoss()) {
			algorithm.updateOrderStopLoss(lastUpTrend);
			return true;
		}
	}
	else {
		auto lastDownTrend = algorithm.getLastDownSuperTrend();
		if (lastDownTrend <= order.getMinimumProfit() && lastDownTrend < order.getStopLoss()) {
			algorithm.updateOrderStopLoss(lastDownTrend);
			return true;
		}
	}
	return false;
}

bool dynamicStopLoss::checkDynamic() {
	const auto dynamicSLPercent = algorithm.getData().getDynamicSLPercent();
	if (dynamicSLPercent < 0.0 || utils::isEqual(dynamicSLPercent, 0.0)) {
		utils::logError("dynamicStopLoss::checkDynamic wrong percent");
		return false;
	}
	auto& order = algorithm.getOrder();
	const auto& candle = algorithm.getCandle();
	const auto pricePrecision = MARKET_DATA->getPricePrecision();
	if (algorithm.getState() == eState::LONG) {
		auto dynamicStopLoss = utils::round(candle.high * (100.0 - dynamicSLPercent) / 100.0, pricePrecision);
		if (dynamicStopLoss >= order.getMinimumProfit() && dynamicStopLoss > order.getStopLoss()) {
			algorithm.updateOrderStopLoss(dynamicStopLoss);
			return true;
		}
	}
	else {
		auto dynamicStopLoss = utils::round(candle.low * (100.0 + dynamicSLPercent) / 100.0, pricePrecision);
		if (dynamicStopLoss <= order.getMinimumProfit() && dynamicStopLoss < order.getStopLoss()) {
			algorithm.updateOrderStopLoss(dynamicStopLoss);
			return true;
		}
	}
	return false;
}

bool dynamicStopLoss::check() {
	if (algorithm.getData().getDynamicSLTrendMode()) {
		return checkTrend();
	}
	return checkDynamic();
}