#include "stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <iostream>
#include <fstream>

using namespace algorithm;

std::string stAlgorithm::stateToString(eState aState) {
	switch (aState) {
		case algorithm::eState::LONG:
			return "LONG";
		case algorithm::eState::SHORT:
			return "SHORT";
		case algorithm::eState::STOP_LOSS_WAIT:
			return "STOP_LOSS_WAIT";
		case algorithm::eState::ACTIVATION_WAIT:
			return "ACTIVATION_WAIT";
		default:
			return "NONE";
	}
}

eState stAlgorithm::stateFromString(const std::string& aStr) {
	if (aStr == "LONG") {
		return eState::LONG;
	}
	else if (aStr == "SHORT") {
		return eState::SHORT;
	}
	else if (aStr == "STOP_LOSS_WAIT") {
		return eState::STOP_LOSS_WAIT;
	}
	else if (aStr == "ACTIVATION_WAIT") {
		return eState::ACTIVATION_WAIT;
	}
	return eState::NONE;
}

stAlgorithm::stAlgorithm(const stAlgorithmData& aData):
	baseClass(aData),
	activationWaiterModule(*this),
	stopLossWaiterModule(*this),
	dynamicStopLossModule(*this),
	trendTouchOpenerModule(*this),
	trendBreakOpenerModule(*this),
	stFactor(aData.getStFactor()), // TO DO delete all below
	atrSize(aData.getAtrSize()),
	atrType(aData.getAtrType()) {}

bool stAlgorithm::operator==(const stAlgorithm& aOther) const {
	auto result = baseClass::operator==(aOther);
	result &= activationWaiterModule == aOther.activationWaiterModule;
	result &= stopLossWaiterModule == aOther.stopLossWaiterModule;
	result &= state == aOther.state;
	result &= isTrendUp == aOther.isTrendUp;
	result &= isNewTrend == aOther.isNewTrend;
	if (getFullCheck()) {
		result &= utils::isEqual(lastUpSuperTrend, aOther.lastUpSuperTrend);
		result &= utils::isEqual(lastDownSuperTrend, aOther.lastDownSuperTrend);
	}
	return result;
}

void stAlgorithm::setState(eState aState) {
	state = aState;
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

bool stAlgorithm::calculate(const std::vector<candle>& aCandles) {
	for (const auto& candle : aCandles) {
		if (!doAction(candle)) {
			return false;
		}
	}
	return true;
}

bool stAlgorithm::doAction(const candle& aCandle) {
	if (getStopCashBreak()) {
		return false;
	}
	if (!updateCandles(aCandle)) {
		return true;
	}
	updateTrends();
	while (update()) {}
	if (getWithLogs()) {
		log();
	}
	return true;
}

bool stAlgorithm::update() {
	switch (state) {
		case algorithm::eState::NONE:
			return checkTrend();
		case algorithm::eState::STOP_LOSS_WAIT:
			return stopLossWaiterModule.check();
		case algorithm::eState::ACTIVATION_WAIT:
			return activationWaiterModule.check();
		default:
			return updateOrder();
	}
}

void stAlgorithm::updateTrends() {
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

bool stAlgorithm::checkTrend() {
	if (isNewTrend) {
		return trendBreakOpenerModule.check();
	}
	return trendTouchOpenerModule.check();
}

bool stAlgorithm::updateOrder() {
	bool needReupdate = false;
	if (state == eState::LONG && getCandle().low <= getOrder().getStopLoss()) {
		closeOrder();
	}
	else if (state == eState::SHORT && getCandle().high >= getOrder().getStopLoss()) {
		closeOrder();
	}
	else if (getOrder().getTime() != getCandle().time) {
		needReupdate = dynamicStopLossModule.check();
	}
	return needReupdate;
}

void stAlgorithm::openOrder(eState aState, double aPrice) {
	aPrice = utils::round(aPrice, market::marketData::getInstance()->getPricePrecision());
	state = aState;
	if (!getOrder().openOrder(*this, aPrice)) {
		state = eState::NONE;
		return;
	}

	auto taxAmount = utils::round(getOrder().getNotionalValue() * stAlgorithmData::tax, market::marketData::getInstance()->getQuotePrecision());
	cash = cash - getOrder().getMargin() - taxAmount;
	stats.onOpenOrder((state == eState::LONG), isNewTrend);
	isNewTrend = false;
}

void stAlgorithm::closeOrder() {
	const auto profit = getOrder().getProfit();
	if (profit < 0) {
		stopLossWaiterModule.start();
	}
	else {
		state = eState::NONE;
	}
	cash = cash + getOrder().getMargin() + profit;
	getOrder().reset();
	getOrder().setFullCheck(getFullCheck());
	if (const bool isMaxLossStop = stats.onCloseOrder(cash, profit)) {
		stopCashBreak = true;
	}
}

void stAlgorithm::log() const {
	std::ofstream output("Logs.txt", std::ios::app);
	output << getCandle().time << "\tcash: " << std::setw(12) << std::to_string(cash)
		<< std::setw(18) << stateToString(state) << std::setw(4) << std::to_string(isTrendUp)
		<< std::setw(4) << std::to_string(isNewTrend);
	if (state == eState::STOP_LOSS_WAIT) {
		output << std::setw(4) << std::to_string(stopLossWaiterModule.getCounter());
	}
	else if (state == eState::ACTIVATION_WAIT) {
		output << std::setw(4) << std::to_string(activationWaiterModule.getCounter());
	}
	else if (!getOrder().getTime().empty()) {
		output << getOrder().toString();
	}
	output << std::endl;
}