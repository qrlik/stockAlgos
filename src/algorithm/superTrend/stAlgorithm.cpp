#include "stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

stAlgorithm::stAlgorithm(const stAlgorithmData& aData):
	baseClass(aData),
	activationWaiterModule(*this),
	stopLossWaiterModule(*this),
	dynamicStopLossModule(*this),
	trendTouchOpenerModule(*this),
	trendBreakOpenerModule(*this)
{
	addState(getIntState(eCustomState::STOP_LOSS_WAIT), "STOP_LOSS_WAIT");
	addState(getIntState(eCustomState::ACTIVATION_WAIT), "ACTIVATION_WAIT");
}

bool stAlgorithm::operator==(const stAlgorithm& aOther) const {
	auto result = baseClass::operator==(aOther);
	result &= activationWaiterModule == aOther.activationWaiterModule;
	result &= stopLossWaiterModule == aOther.stopLossWaiterModule;
	result &= isTrendUp == aOther.isTrendUp;
	result &= isNewTrend == aOther.isNewTrend;
	if (getData().getFullCheck()) {
		result &= utils::isEqual(lastUpSuperTrend, aOther.lastUpSuperTrend);
		result &= utils::isEqual(lastDownSuperTrend, aOther.lastDownSuperTrend);
	}
	return result;
}

double stAlgorithm::getSuperTrend() const {
	return (isTrendUp) ? lastUpSuperTrend : lastDownSuperTrend;
}

double stAlgorithm::getActualSuperTrend() const {
	if (isNewTrend) {
		return (isTrendUp)
			? std::max(lastDownSuperTrend, lastUpSuperTrend)
			: std::min(lastDownSuperTrend, lastUpSuperTrend);
	}
	return getSuperTrend();
}

bool stAlgorithm::isNewTrendChanged() {
	if (!isNewTrend) {
		return false;
	}
	if ((isTrendUp && getCandle().low <= getSuperTrend()) || (!isTrendUp && getCandle().high >= getSuperTrend())) {
		isNewTrend = false;
		return true;
	}
	return false;
}

void stAlgorithm::preLoop() {
	if (getPrevCandle().trendIsUp) {
		lastUpSuperTrend = getPrevCandle().superTrend;
	}
	else {
		lastDownSuperTrend = getPrevCandle().superTrend;
	}

	if (isTrendUp != getPrevCandle().trendIsUp) {
		isTrendUp = getPrevCandle().trendIsUp;
		isNewTrend = trendBreakOpenerModule.isNewTrendAllowed();
		stopLossWaiterModule.onNewTrend();
		activationWaiterModule.onNewTrend();
	}
}

bool stAlgorithm::loop() {
	const auto curState = getState();
	if (curState == getIntState(eBaseState::NONE)) {
		return checkTrend();
	}
	else if (curState == getIntState(eCustomState::STOP_LOSS_WAIT)) {
		return stopLossWaiterModule.check();
	}
	else if (curState == getIntState(eCustomState::ACTIVATION_WAIT)) {
		return activationWaiterModule.check();
	}
	return updateOrder();
}

bool stAlgorithm::checkTrend() {
	if (isNewTrend) {
		return trendBreakOpenerModule.check();
	}
	return trendTouchOpenerModule.check();
}

bool stAlgorithm::updateOrder() {
	bool needReupdate = false;
	if (getState() == getIntState(eBaseState::LONG) && getCandle().low <= getOrder().getStopLoss()) {
		closeOrder();
	}
	else if (getState() == getIntState(eBaseState::SHORT) && getCandle().high >= getOrder().getStopLoss()) {
		closeOrder();
	}
	else if (getOrder().getTime() != getCandle().time) {
		needReupdate = dynamicStopLossModule.check();
	}
	return needReupdate;
}

void stAlgorithm::onOpenOrder() {
	if (isNewTrend) {
		stats.incrementCounter("breakTrendOrder");
	}
	else {
		stats.incrementCounter("touchTrendOrder");
	}
	isNewTrend = false;
}

void stAlgorithm::onCloseOrder(double aProfit) {
	if (aProfit < 0) {
		stopLossWaiterModule.start();
	}
}

void stAlgorithm::updateOrderStopLoss(double aStopLoss) {
	order.updateStopLoss(aStopLoss);
}

void stAlgorithm::initInternal() {
	isTrendUp = getPrevCandle().trendIsUp;
}

void stAlgorithm::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	if (aName == "activationWaitCounter") {
		activationWaiterModule.activationWaitCounter = aValue.get<int>();
	}
	else if (aName == "stopLossWaitCounter") {
		stopLossWaiterModule.stopLossWaitCounter = aValue.get<int>();
	}
	else if (aName == "lastUpSuperTrend") {
		lastUpSuperTrend = aValue.get<double>();
	}
	else if (aName == "lastDownSuperTrend") {
		lastDownSuperTrend = aValue.get<double>();
	}
	else if (aName == "isTrendUp") {
		isTrendUp = aValue.get<bool>();
	}
	else if (aName == "isNewTrend") {
		isNewTrend = aValue.get<bool>();
	}
	else if (aName == "state") {
		setState(stateFromString(aValue.get<std::string>()));
	}
}

void stAlgorithm::logInternal(std::ofstream& aFile) const {
	aFile << std::to_string(isTrendUp) << std::setw(4) << std::to_string(isNewTrend);
	if (getState() == getIntState(eCustomState::STOP_LOSS_WAIT)) {
		aFile << std::setw(4) << std::to_string(stopLossWaiterModule.getCounter());
	}
	else if (getState() == getIntState(eCustomState::ACTIVATION_WAIT)) {
		aFile << std::setw(4) << std::to_string(activationWaiterModule.getCounter());
	}
}