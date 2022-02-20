#include "moneyMaker.h"
#include "../utils/utils.h"

using namespace algorithm;

moneyMaker::moneyMaker(const algorithmData& aData):
	activationWaiterModule(this, aData.activationWaiterRange, aData.activationWaiterResetAllowed, aData.activationWaiterFullCandleCheck),
	stopLossWaiterModule(this, aData.stopLossWaiterRange, aData.stopLossWaiterEnabled, aData.stopLossWaiterResetAllowed, aData.stopLossWaiterFullCandleCheck),
	dynamicStopLossModule(this, aData.dynamicSLPercent, aData.dynamicSLTrendMode),
	trendTouchOpenerModule(this, aData.touchOpenerActivationWaitMode),
	trendBreakOpenerModule(this, aData.breakOpenerEnabled, aData.breakOpenerActivationWaitMode, aData.alwaysUseNewTrend),
	stFactor(aData.stFactor),
	atrSize(aData.atrSize),
	atrType(aData.atrType),
	activationPercent(aData.activationPercent),
	stopLossPercent(aData.stopLossPercent),
	minimumProfitPercent(aData.minimumProfitPercent),
	dealPercent(aData.dealPercent),
	leverage(aData.leverage)
{}

moneyMaker::eState moneyMaker::getState() const {
	return state;
}

void moneyMaker::setState(eState aState) {
	state = aState;
}

bool moneyMaker::getIsTrendUp() const {
	return isTrendUp;
}

const candle& moneyMaker::getCandle() const {
	return curCandle;
}

activationWaiter& moneyMaker::getActivationWaiter() {
	return activationWaiterModule;
}

orderData& moneyMaker::getOrder() {
	return order;
}

double moneyMaker::getLastUpSuperTrend() const {
	return lastUpSuperTrend;
}

double moneyMaker::getLastDownSuperTrend() const {
	return lastDownSuperTrend;
}

double moneyMaker::getSuperTrend() const {
	return (isTrendUp) ? lastUpSuperTrend : lastDownSuperTrend;
}

double moneyMaker::getActualSuperTrend() const {
	if (isNewTrend) {
		return (isTrendUp)
			? std::max(lastDownSuperTrend, lastUpSuperTrend)
			: std::min(lastDownSuperTrend, lastUpSuperTrend);
	}
	return getSuperTrend();
}

double moneyMaker::getTrendActivation(double aSuperTrend) const {
	auto trendActivationSign = (isTrendUp) ? 1 : -1;
	auto result = aSuperTrend * (100 + trendActivationSign * activationPercent) / 100;
	if (fullCheck) {
		return (isTrendUp) ? utils::ceil(result, 2) : utils::floor(result, 2);
	}
	return result;
}

bool moneyMaker::isNewTrendChanged() {
	if (!isNewTrend) {
		return false;
	}
	const auto trendActivation = getTrendActivation(getSuperTrend());
	if ((isTrendUp && curCandle.low <= trendActivation) || (!isTrendUp && curCandle.high >= trendActivation)) {
		isNewTrend = false;
		return true;
	}
	return false;
}

void moneyMaker::openOrder(eState aState, double aPrice) {

}