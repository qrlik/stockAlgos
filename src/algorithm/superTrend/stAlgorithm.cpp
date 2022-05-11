#include "stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <fstream>

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

	if (!inited) {
		isTrendUp = getPrevCandle().trendIsUp;
		inited = true;
	}
	else if (isTrendUp != getPrevCandle().trendIsUp) {
		isTrendUp = getPrevCandle().trendIsUp;
		isNewTrend = trendBreakOpenerModule.isNewTrendAllowed();
		stopLossWaiterModule.onNewTrend();
		activationWaiterModule.onNewTrend();
	}
}

bool stAlgorithm::loop() {
	const auto state = getState();
	if (state == getIntState(eBaseState::NONE)) {
		return checkTrend();
	}
	else if (state == getIntState(eCustomState::STOP_LOSS_WAIT)) {
		return stopLossWaiterModule.check();
	}
	else if (state == getIntState(eCustomState::ACTIVATION_WAIT)) {
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

void stAlgorithm::openOrder(eOrderState aState, double aPrice) {
	aPrice = utils::round(aPrice, market::marketData::getInstance()->getPricePrecision());
	if (!order.openOrder(*this, aState, aPrice)) {
		setState(getIntState(eBaseState::NONE));
		return;
	}

	setState(getIntState(aState));
	auto taxAmount = utils::round(getOrder().getNotionalValue() * MARKET_DATA->getTaxFactor(), market::marketData::getInstance()->getQuotePrecision());
	cash = cash - getOrder().getMargin() - taxAmount;
	stats.onOpenOrder((aState == eOrderState::LONG), isNewTrend);
	isNewTrend = false;
}

void stAlgorithm::closeOrder() {
	const auto profit = getOrder().getProfit();
	if (profit < 0) {
		stopLossWaiterModule.start();
	}
	else {
		setState(getIntState(eBaseState::NONE));
	}
	cash = cash + getOrder().getMargin() + profit;
	order.reset();
	if (const bool isMaxLossStop = stats.onCloseOrder(cash, profit)) {
		stopCashBreak = true;
	}
}

void stAlgorithm::updateOrderStopLoss(double aStopLoss) {
	order.updateStopLoss(aStopLoss);
}

void stAlgorithm::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	if (aName == "activationWaitCounter") {
		getActivationWaiter().setCounter(aValue.get<int>());
	}
	else if (aName == "stopLossWaitCounter") {
		getStopLossWaiter().setCounter(aValue.get<int>());
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

void stAlgorithm::log() const {
	std::ofstream output("Logs.txt", std::ios::app);
	output << getCandle().time << "\tcash: " << std::setw(12) << std::to_string(cash)
		<< std::setw(18) << stateToString(getState()) << std::setw(4) << std::to_string(isTrendUp) // TO DO fix get int state
		<< std::setw(4) << std::to_string(isNewTrend);
	if (getState() == getIntState(eCustomState::STOP_LOSS_WAIT)) {
		output << std::setw(4) << std::to_string(stopLossWaiterModule.getCounter());
	}
	else if (getState() == getIntState(eCustomState::ACTIVATION_WAIT)) {
		output << std::setw(4) << std::to_string(activationWaiterModule.getCounter());
	}
	else if (!getOrder().getTime().empty()) {
		output << getOrder().toString();
	}
	output << std::endl;
}