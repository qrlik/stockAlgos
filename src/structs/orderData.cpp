#include "orderData.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <assert.h>

void orderData::initOrderDataFromJson(orderData& aData, const Json& aJson) {
	for (const auto& [field, value] : aJson.items()) {
		if (field == "price") {
			aData.price = value.get<double>();
		}
		else if (field == "stopLoss") {
			aData.stopLoss = value.get<double>();
		}
		else if (field == "minimumProfit") {
			aData.minimumProfit = value.get<double>();
		}
		else if (field == "margin") {
			aData.margin = value.get<double>();
		}
		else if (field == "notionalValue") {
			aData.notionalValue = value.get<double>();
		}
		else if (field == "quantity") {
			aData.quantity = value.get<double>();
		}
		else if (field == "time") {
			aData.time = value.get<std::string>();
		}
	}
}

orderData::orderData() : state(algorithm::eState::NONE) {}

std::string orderData::toString() const {
	std::ostringstream ostr;
	ostr << std::setw(6) << "p: " << std::setw(12) << std::to_string(price)
		<< std::setw(6) << "s: " << std::setw(12) << std::to_string(stopLoss)
		<< std::setw(6) << "a: " << std::setw(12) << std::to_string(margin);
	return ostr.str();
}

bool orderData::operator==(const orderData& aOther) const {
	bool result = true;
	result &= fullCheck == aOther.fullCheck;
	result &= utils::isEqual(price, aOther.price);
	result &= time == aOther.time;
	if (fullCheck) {
		result &= utils::isEqual(stopLoss, aOther.stopLoss);
		result &= utils::isEqual(minimumProfit, aOther.minimumProfit);
		result &= utils::isEqual(margin, aOther.margin);
		result &= utils::isEqual(notionalValue, aOther.notionalValue);
		result &= utils::isEqual(quantity, aOther.quantity);
	}
	return result;
}

void orderData::reset() {
	time.clear();
	price = 0.0;
	stopLoss = 0.0;
	minimumProfit = 0.0;
	margin = 0.0;
	notionalValue = 0.0;
	quantity = 0.0;
	state = algorithm::eState::NONE;
	fullCheck = false;
}

double orderData::calculateStopLoss(const algorithm::stAlgorithm& aMM) const {
	const auto liqPrice = MARKET_DATA->getLiquidationPrice(price, notionalValue, aMM.getData().getLeverage(), quantity, state == algorithm::eState::LONG);
	auto stopLossSign = (state == algorithm::eState::LONG) ? 1 : -1;
	auto result = liqPrice * (100 + stopLossSign * aMM.getData().getLiquidationOffsetPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

double orderData::calculateMinimumProfit(const algorithm::stAlgorithm& aMM) const {
	auto minProfitSign = (state == algorithm::eState::LONG) ? 1 : -1;
	auto result = price * (100.0 + minProfitSign * aMM.getData().getMinimumProfitPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

bool orderData::openOrder(const algorithm::stAlgorithm& aMM, double aPrice) {
	reset();
	const auto quotePrecision = market::marketData::getInstance()->getQuotePrecision();
	auto allowedCash = utils::floor(aMM.getCash() * aMM.getData().getDealPercent() / 100.0, quotePrecision);
	if (const auto allowedCashBySize = utils::floor(aMM.getData().getOrderSize(), quotePrecision); allowedCashBySize > 0.0) {
		allowedCash = std::min(allowedCash, allowedCashBySize);
	}
	const auto allowedNotionalValue = allowedCash * aMM.getData().getLeverage();
	const auto calcQuantity = utils::floor(allowedNotionalValue / aPrice, MARKET_DATA->getQuantityPrecision());
	const auto calcNotionalValue = utils::round(calcQuantity * aPrice, quotePrecision);
	if (calcQuantity < MARKET_DATA->getQuantityPrecision() || calcNotionalValue < MARKET_DATA->getMinNotionalValue()) {
		utils::logError("orderData::openOrder can't open order");
		return false;
	}

	state = aMM.getState();
	fullCheck = aMM.getFullCheck();
	price = aPrice;
	quantity = calcQuantity;
	notionalValue = calcNotionalValue;
	margin = utils::round(notionalValue / aMM.getData().getLeverage(), quotePrecision);

	time = aMM.getCandle().time;
	minimumProfit = calculateMinimumProfit(aMM);
	stopLoss = calculateStopLoss(aMM);
	return true;
}

double orderData::getProfit() const {
	auto quotePrecision = market::marketData::getInstance()->getQuotePrecision();
	const auto orderCloseSummary = utils::round(quantity * stopLoss, quotePrecision);
	const auto orderCloseTax = utils::round(orderCloseSummary * algorithm::stAlgorithmData::tax, quotePrecision);
	auto profitWithoutTax = (state == algorithm::eState::LONG) ? orderCloseSummary - notionalValue : notionalValue - orderCloseSummary;
	return profitWithoutTax - orderCloseTax;
}