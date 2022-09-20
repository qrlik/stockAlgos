#include "dynamicStopLoss.h"
#include "stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

dynamicStopLoss::dynamicStopLoss(stAlgorithm& aAlgorithm):
	algorithm(aAlgorithm) {}

bool dynamicStopLoss::checkTrend() {
	const auto& order = static_cast<const stAlgorithm&>(algorithm).getOrder();
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		auto lastUpTrend = algorithm.getLastUpSuperTrend();
		if (utils::isGreaterOrEqual(lastUpTrend, order.getMinimumProfit()) && utils::isGreater(lastUpTrend, order.getStopLoss())) {
			algorithm.updateOrderStopLoss(lastUpTrend);
			return true;
		}
	}
	else {
		auto lastDownTrend = algorithm.getLastDownSuperTrend();
		if (utils::isLessOrEqual(lastDownTrend, order.getMinimumProfit()) && utils::isLess(lastDownTrend, order.getStopLoss())) {
			algorithm.updateOrderStopLoss(lastDownTrend);
			return true;
		}
	}
	return false;
}

bool dynamicStopLoss::checkDynamic() {
	const auto dynamicSLPercent = algorithm.getData().getDynamicSLPercent();
	if (utils::isLess(dynamicSLPercent, 0.0) || utils::isEqual(dynamicSLPercent, 0.0)) {
		utils::logError("dynamicStopLoss::checkDynamic wrong percent");
		return false;
	}
	const auto& order = static_cast<const stAlgorithm&>(algorithm).getOrder();
	const auto& candle = algorithm.getCandle();
	const auto pricePrecision = algorithm.getData().getMarketData().getPricePrecision();
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		auto dynamicStopLoss = utils::round(candle.high * (100.0 - dynamicSLPercent) / 100.0, pricePrecision);
		if (utils::isGreaterOrEqual(dynamicStopLoss, order.getMinimumProfit()) && utils::isGreater(dynamicStopLoss, order.getStopLoss())) {
			algorithm.updateOrderStopLoss(dynamicStopLoss);
			return true;
		}
	}
	else {
		auto dynamicStopLoss = utils::round(candle.low * (100.0 + dynamicSLPercent) / 100.0, pricePrecision);
		if (utils::isLessOrEqual(dynamicStopLoss, order.getMinimumProfit()) && utils::isLess(dynamicStopLoss, order.getStopLoss())) {
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