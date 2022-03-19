#include "moneyMaker.h"
#include "../market/marketRules.h"
#include "../utils/utils.h"
#include <iostream>
#include <fstream>

using namespace algorithm;

std::string moneyMaker::stateToString(eState aState) {
	switch (aState) {
		case algorithm::moneyMaker::eState::LONG:
			return "LONG";
		case algorithm::moneyMaker::eState::SHORT:
			return "SHORT";
		case algorithm::moneyMaker::eState::STOP_LOSS_WAIT:
			return "STOP_LOSS_WAIT";
		case algorithm::moneyMaker::eState::ACTIVATION_WAIT:
			return "ACTIVATION_WAIT";
		default:
			return "NONE";
	}
}

moneyMaker::eState moneyMaker::stateFromString(const std::string& aStr) {
	if (aStr == "LONG") {
		return moneyMaker::eState::LONG;
	}
	else if (aStr == "SHORT") {
		return moneyMaker::eState::SHORT;
	}
	else if (aStr == "STOP_LOSS_WAIT") {
		return moneyMaker::eState::STOP_LOSS_WAIT;
	}
	else if (aStr == "ACTIVATION_WAIT") {
		return moneyMaker::eState::ACTIVATION_WAIT;
	}
	return moneyMaker::eState::NONE;
}

moneyMaker::moneyMaker(const algorithmData& aData, double aCash):
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
	startCash(aCash),
	cash(aCash),
	stopCash(aCash * 0.4) {}

bool moneyMaker::operator==(const moneyMaker& aOther) {
	assert(activationWaiterModule == aOther.activationWaiterModule);
	assert(stopLossWaiterModule == aOther.stopLossWaiterModule);
	assert(state == aOther.state);
	assert(order == aOther.order);
	assert(fullCheck == aOther.fullCheck);
	assert(isTrendUp == aOther.isTrendUp);
	assert(isNewTrend == aOther.isNewTrend);
	if (fullCheck) {
		assert(cash == aOther.cash);
		assert(lastUpSuperTrend == aOther.lastUpSuperTrend);
		assert(lastDownSuperTrend == aOther.lastDownSuperTrend);
	}
	return true;
}

moneyMaker::eState moneyMaker::getState() const {
	return state;
}

void moneyMaker::setState(eState aState) {
	state = aState;
}

void moneyMaker::setWithLogs(bool aState) {
	withLogs = aState;
}

void moneyMaker::setTest(bool aState) {
	isTest = aState;
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

stopLossWaiter& moneyMaker::getStopLossWaiter() {
	return stopLossWaiterModule;
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

double moneyMaker::getStopLossPrice() const { // move inside order
	const auto liqPrice = order.getLiquidationPrice();
	auto stopLossSign = (state == eState::LONG) ? 1 : -1;
	auto result = liqPrice * (100 + stopLossSign * liquidationOffsetPercent) / 100.0;
	return result;
}

double moneyMaker::getMinimumProfitPrice() const { // move inside order
	auto minProfitSign = (state == eState::LONG) ? 1 : -1;
	auto result = order.price * (100.0 + minProfitSign * minimumProfitPercent) / 100.0;
	if (fullCheck) {
		return (state == eState::LONG) ? utils::ceil(result, 2) : utils::floor(result, 2);
	}
	return result;
}

double moneyMaker::getFullCash() const {
	auto curCash = cash;
	if (!order.time.empty()) {
		curCash += order.margin;
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
		case algorithm::moneyMaker::eState::NONE:
			return checkTrend();
		case algorithm::moneyMaker::eState::STOP_LOSS_WAIT:
			return stopLossWaiterModule.check();
		case algorithm::moneyMaker::eState::ACTIVATION_WAIT:
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
	if (state == eState::LONG && curCandle.low <= order.stopLoss) {
		closeOrder();
	}
	else if (state == eState::SHORT && curCandle.high >= order.stopLoss) {
		closeOrder();
	}
	else if (order.time != curCandle.time) {
		needReupdate = dynamicStopLossModule.check();
	}
	return needReupdate;
}

void moneyMaker::openOrder(eState aState, double aPrice) {
	state = aState;
	order = orderData{}; // move inside order
	order.fullCheck = fullCheck;
	order.price = aPrice;
	order.minimumProfit = getMinimumProfitPrice();
	order.margin = cash * dealPercent / 100.0;
	order.notionalValue = order.margin * leverage;
	order.quantity = order.notionalValue / order.price; // add 0.001 BTC here check
	if (fullCheck) {
		order.margin = utils::floor(order.margin, 3);
	}
	order.stopLoss = getStopLossPrice();
	order.time = curCandle.time;

	auto taxAmount = order.notionalValue * algorithmData::tax;
	cash = cash - order.margin - taxAmount;
	if (fullCheck) {
		cash = utils::floor(cash, 2);
	}
	stats.onOpenOrder(isNewTrend);
	isNewTrend = false;
}

void moneyMaker::closeOrder() {
	auto orderCloseSummary = order.notionalValue / order.price * order.stopLoss; // move inside order
	auto orderCloseTax = orderCloseSummary * algorithmData::tax;
	auto profitWithoutTax = (state == eState::LONG) ? orderCloseSummary - order.notionalValue : order.notionalValue - orderCloseSummary;
	auto profit = profitWithoutTax - orderCloseTax;
	if (profit < 0) {
		stopLossWaiterModule.start();
	}
	else {
		state = eState::NONE;
	}
	cash = cash + order.margin + profit;
	if (fullCheck) {
		cash = utils::floor(cash, 2);
	}
	order = orderData{};
	order.fullCheck = fullCheck;
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
	else if (!order.time.empty()) {
		output << order.toString();
	}
	output << std::endl;
}