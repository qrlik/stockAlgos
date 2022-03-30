#include "moneyMaker.h"
#include "../market/marketRules.h"
#include "../utils/utils.h"
#include <iostream>
#include <fstream>

using namespace algorithm;

std::string moneyMaker::stateToString(eState aState) {
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

eState moneyMaker::stateFromString(const std::string& aStr) {
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

moneyMaker::moneyMaker(const algorithmData& aData):
	activationWaiterModule(this, aData.activationWaiterRange, aData.activationWaiterResetAllowed, aData.activationWaiterFullCandleCheck),
	stopLossWaiterModule(this, aData.stopLossWaiterRange, aData.stopLossWaiterEnabled, aData.stopLossWaiterResetAllowed, aData.stopLossWaiterFullCandleCheck),
	dynamicStopLossModule(this, aData.dynamicSLPercent, aData.dynamicSLTrendMode),
	trendTouchOpenerModule(this, aData.touchOpenerActivationWaitMode),
	trendBreakOpenerModule(this, aData.breakOpenerEnabled, aData.breakOpenerActivationWaitMode, aData.alwaysUseNewTrend),
	stFactor(aData.stFactor),
	atrSize(aData.atrSize),
	atrType(aData.atrType),
	liquidationOffsetPercent(aData.liquidationOffsetPercent),
	minimumProfitPercent(aData.minimumProfitPercent),
	dealPercent(aData.dealPercent),
	leverage(aData.leverage),
	fullCheck(aData.fullCheck),
	startCash(aData.startCash),
	cash(aData.startCash),
	stopCash(aData.stopCash),
	stats(aData.startCash) {}

bool moneyMaker::operator==(const moneyMaker& aOther) {
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

void moneyMaker::setState(eState aState) {
	state = aState;
}

void moneyMaker::setWithLogs(bool aState) {
	withLogs = aState;
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

double moneyMaker::getFullCash() const {
	auto curCash = cash;
	if (!order.getTime().empty()) {
		curCash += order.getMargin() + order.getProfit();
	}
	return curCash;
}

bool moneyMaker::isNewTrendChanged() {
	if (!isNewTrend) {
		return false;
	}
	if ((isTrendUp && curCandle.low <= getSuperTrend()) || (!isTrendUp && curCandle.high >= getSuperTrend())) {
		isNewTrend = false;
		return true;
	}
	return false;
}

void moneyMaker::calculate(const std::vector<candle>& aCandles) {
	for (const auto& candle : aCandles) {
		if (!doAction(candle)) {
			break;
		}
	}
}

bool moneyMaker::doAction(const candle& aCandle) {
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

bool moneyMaker::update() {
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

bool moneyMaker::updateCandles(const candle& aCandle) {
	if (curCandle.time.empty()) {
		curCandle = aCandle;
		return false;
	}
	prevCandle = std::move(curCandle);
	curCandle = aCandle;
	return true;
}

void moneyMaker::updateTrends() {
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

bool moneyMaker::checkTrend() {
	if (isNewTrend) {
		return trendBreakOpenerModule.check();
	}
	return trendTouchOpenerModule.check();
}

bool moneyMaker::updateOrder() {
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

void moneyMaker::openOrder(eState aState, double aPrice) {
	state = aState;
	if (!order.openOrder(*this, aPrice)) {
		state = eState::NONE;
		return;
	}

	auto taxAmount = utils::round(order.getNotionalValue() * algorithmData::tax, market::marketData::getInstance()->getQuotePrecision());
	cash = cash - order.getMargin() - taxAmount;
	stats.onOpenOrder((state == eState::LONG), isNewTrend);
	isNewTrend = false;
}

void moneyMaker::closeOrder() {
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
	stats.onCloseOrder(cash, profit);
	if (cash <= stopCash) {
		stopCashBreak = true;
	}
}

void moneyMaker::log() {
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