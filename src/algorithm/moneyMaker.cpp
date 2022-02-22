#include "moneyMaker.h"
#include "../utils/utils.h"
#include <iostream>

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

moneyMaker::moneyMaker(const algorithmData& aData, double aCash):
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
	leverage(aData.leverage),
	startCash(aCash),
	cash(aCash),
	stopCash(aCash * 0.1) {}

moneyMaker::eState moneyMaker::getState() const {
	return state;
}

void moneyMaker::setState(eState aState) {
	state = aState;
}

void moneyMaker::setWithLogs(bool aState) {
	withLogs = aState;
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

double moneyMaker::getStopLossPrice() const {
	auto stopLossSign = (state == eState::LONG) ? -1 : 1;
	auto result = order.price * (100.0 + stopLossSign * stopLossPercent) / 100.0;
	if (fullCheck) {
		return (state == eState::LONG) ? utils::floor(result, 2) : utils::ceil(result, 2);
	}
	return result;
}

double moneyMaker::getMinimumProfitPrice() const {
	auto minProfitSign = (state == eState::LONG) ? 1 : -1;
	auto result = order.price * (100.0 + minProfitSign * minimumProfitPercent) / 100.0;
	if (fullCheck) {
		return (state == eState::LONG) ? utils::ceil(result, 2) : utils::floor(result, 2);
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
	order = orderData{};
	order.fullCheck = fullCheck;
	order.price = aPrice;
	order.stopLoss = getStopLossPrice();
	order.minimumProfit = getMinimumProfitPrice();
	order.amount = cash * dealPercent / 100.0;
	if (fullCheck) {
		order.amount = utils::floor(order.amount, 3);
	}
	order.time = curCandle.time;

	auto taxAmount = order.amount * leverage * algorithmData::tax;
	cash = cash - order.amount - taxAmount;
	if (fullCheck) {
		cash = utils::floor(cash, 2);
	}
	stats.onOpenOrder(isNewTrend);
	isNewTrend = false;
}

void moneyMaker::closeOrder() {
	auto orderOpenSummary = order.amount * leverage;
	auto orderCloseSummary = orderOpenSummary / order.price * order.stopLoss;
	auto orderCloseTax = orderCloseSummary * algorithmData::tax;
	auto profitWithoutTax = (state == eState::LONG) ? orderCloseSummary - orderOpenSummary : orderOpenSummary - orderCloseSummary;
	auto profit = profitWithoutTax - orderCloseTax;
	if (profit < 0) {
		stopLossWaiterModule.start();
	}
	else {
		state = eState::NONE;
	}
	cash = cash + order.amount + profit;
	if (fullCheck) {
		cash = utils::floor(cash, 2);
	}
	order = orderData{};
	stats.onCloseOrder(cash, profit);
	if (cash <= stopCash) {
		stopCashBreak = true;
	}
}

void moneyMaker::log() {
	std::cout << curCandle.time << "\tcash: " << std::setw(12) << std::to_string(cash)
		<< std::setw(8) << stateToString(state) << std::setw(4) << std::to_string(isTrendUp)
		<< std::setw(4) << std::to_string(isNewTrend);
	if (state == eState::STOP_LOSS_WAIT) {
		std::cout << std::setw(4) << std::to_string(stopLossWaiterModule.getCounter());
	}
	else if (state == eState::ACTIVATION_WAIT) {
		std::cout << std::setw(4) << std::to_string(activationWaiterModule.getCounter());
	}
	else if (!order.time.empty()) {
		std::cout << order.toString();
	}
	std::cout << std::endl;
}