#include "checkers.h"
#include "algorithm/statistic.h"
#include "utils/utils.h"
#include "market/marketRules.h"
#include <iostream>

using namespace tests;

void tests::checkAlgorithmData(const algorithm::stAlgorithmData& aData) {
	bool result = true;
	result &= aData.getAtrType() != market::eAtrType::NONE;
	result &= aData.getAtrSize() > 0;
	result &= aData.getStFactor() > 0.0;

	result &= aData.getDealPercent() > 0.0;
	result &= aData.getLeverage() > 0 && aData.getLeverage() <= 125;

	result &= aData.getStartCash() > market::marketData::getInstance()->getMinNotionalValue() / aData.getLeverage();
	result &= aData.getStartCash() > aData.getMaxLossCash();

	const auto minLiqPercent = (aData.getOrderSize() > 0.0)
		? MARKET_DATA->getLiquidationPercent(aData.getOrderSize(), aData.getLeverage())
		: MARKET_DATA->getLeverageLiquidationRange(aData.getLeverage()).first;
	result &= aData.getLiquidationOffsetPercent() < minLiqPercent;
	result &= aData.getMinimumProfitPercent() > 2 * algorithm::stAlgorithmData::tax * 100.0;

	result &= (utils::isEqual(aData.getDynamicSLPercent(), -1.0) && aData.getDynamicSLTrendMode()) || aData.getDynamicSLPercent() > 0.0;

	auto waiter = aData.getTouchOpenerActivationWaitMode();
	if (aData.getBreakOpenerEnabled()) {
		waiter &= aData.getBreakOpenerActivationWaitMode();
	}
	result &= (waiter) ? aData.getActivationWaiterRange() >= 0 : aData.getActivationWaiterRange() == -1;
	result &= (aData.getStopLossWaiterEnabled()) ? aData.getStopLossWaiterRange() >= 0 : aData.getStopLossWaiterRange() == -1;

	if (!result) {
		assert("tests::checkAlgorithmData fail" && result);
		utils::logError("tests::checkAlgorithmData fail");
		std::cout << aData.toJson() << '\n';
	}
}


