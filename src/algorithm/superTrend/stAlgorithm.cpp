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
	atrType(aData.getAtrType()),
	liquidationOffsetPercent(aData.getLiquidationOffsetPercent()),
	minimumProfitPercent(aData.getMinimumProfitPercent()),
	dealPercent(aData.getDealPercent()),
	orderSize(aData.getOrderSize()),
	leverage(aData.getLeverage()),
	fullCheck(aData.getFullCheck()),
	startCash(aData.getStartCash()),
	cash(aData.getStartCash()),
	stats(aData.getStartCash(), aData.getMaxLossPercent(), aData.getMaxLossCash()) {}

bool stAlgorithm::operator==(const stAlgorithm& aOther) {
	// add data == data;
	assert(activationWaiterModule == aOther.activationWaiterModule);
	assert(stopLossWaiterModule == aOther.stopLossWaiterModule);
	assert(state == aOther.state);
	assert(order == aOther.order);
	assert(fullCheck == aOther.fullCheck);
	assert(isTrendUp == aOther.isTrendUp);
	assert(isNewTrend == aOther.isNewTrend);
	if (fullCheck) {
		assert(stats == aOther.stats);
		assert(utils::isEqual(cash, aOther.cash, market::marketData::getInstance()->getQuotePrecision()));
		assert(utils::isEqual(lastUpSuperTrend, aOther.lastUpSuperTrend));
		assert(utils::isEqual(lastDownSuperTrend, aOther.lastDownSuperTrend));
	}
	return true;
}

void stAlgorithm::setState(eState aState) {
	state = aState;
}

void stAlgorithm::setWithLogs(bool aState) {
	withLogs = aState;
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

double stAlgorithm::getFullCash() const {
	auto curCash = cash;
	if (!order.getTime().empty()) {
		curCash += order.getMargin() + order.getProfit();
	}
	return curCash;
}

bool stAlgorithm::isNewTrendChanged() {
	if (!isNewTrend) {
		return false;
	}
	if ((isTrendUp && curCandle.low <= getSuperTrend()) || (!isTrendUp && curCandle.high >= getSuperTrend())) {
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
	if (stopCashBreak) {
		return false;
	}
	if (!updateCandles(aCandle)) {
		return true;
	}
	updateTrends();
	while (update()) {}
	if (withLogs) {
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

bool stAlgorithm::updateCandles(const candle& aCandle) {
	if (curCandle.time.empty()) {
		curCandle = aCandle;
		return false;
	}
	prevCandle = std::move(curCandle);
	curCandle = aCandle;
	return true;
}

void stAlgorithm::updateTrends() {
	if (prevCandle.trendIsUp) {
		lastUpSuperTrend = prevCandle.superTrend;
	}
	else {
		lastDownSuperTrend = prevCandle.superTrend;
	}

	if (!inited) {
		isTrendUp = prevCandle.trendIsUp;
		inited = true;
	}
	else if (isTrendUp != prevCandle.trendIsUp) {
		isTrendUp = prevCandle.trendIsUp;
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
	if (state == eState::LONG && curCandle.low <= order.getStopLoss()) {
		closeOrder();
	}
	else if (state == eState::SHORT && curCandle.high >= order.getStopLoss()) {
		closeOrder();
	}
	else if (order.getTime() != curCandle.time) {
		needReupdate = dynamicStopLossModule.check();
	}
	return needReupdate;
}

void stAlgorithm::openOrder(eState aState, double aPrice) {
	aPrice = utils::round(aPrice, market::marketData::getInstance()->getPricePrecision());
	state = aState;
	if (!order.openOrder(*this, aPrice)) {
		state = eState::NONE;
		return;
	}

	auto taxAmount = utils::round(order.getNotionalValue() * stAlgorithmData::tax, market::marketData::getInstance()->getQuotePrecision());
	cash = cash - order.getMargin() - taxAmount;
	stats.onOpenOrder((state == eState::LONG), isNewTrend);
	isNewTrend = false;
}

void stAlgorithm::closeOrder() {
	const auto profit = order.getProfit();
	if (profit < 0) {
		stopLossWaiterModule.start();
	}
	else {
		state = eState::NONE;
	}
	cash = cash + order.getMargin() + profit;
	order.reset();
	order.setFullCheck(fullCheck);
	if (const bool isMaxLossStop = stats.onCloseOrder(cash, profit)) {
		stopCashBreak = true;
	}
}

void stAlgorithm::log() {
	std::ofstream output("Logs.txt", std::ios::app);
	output << curCandle.time << "\tcash: " << std::setw(12) << std::to_string(cash)
		<< std::setw(18) << stateToString(state) << std::setw(4) << std::to_string(isTrendUp)
		<< std::setw(4) << std::to_string(isNewTrend);
	if (state == eState::STOP_LOSS_WAIT) {
		output << std::setw(4) << std::to_string(stopLossWaiterModule.getCounter());
	}
	else if (state == eState::ACTIVATION_WAIT) {
		output << std::setw(4) << std::to_string(activationWaiterModule.getCounter());
	}
	else if (!order.getTime().empty()) {
		output << order.toString();
	}
	output << std::endl;
}