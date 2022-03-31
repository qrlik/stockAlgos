#include "orderData.h"
#include "../algorithm/moneyMaker.h"
#include "../market/marketRules.h"
#include "../utils/utils.h"
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

bool orderData::operator==(const orderData& aOther) {
	assert(fullCheck == aOther.fullCheck);
	assert(utils::isEqual(price, aOther.price));
	assert(time == aOther.time);
	if (fullCheck) {
		assert(utils::isEqual(stopLoss, aOther.stopLoss));
		assert(utils::isEqual(minimumProfit, aOther.minimumProfit));
		assert(utils::isEqual(margin, aOther.margin));
		assert(utils::isEqual(notionalValue, aOther.notionalValue));
		assert(utils::isEqual(quantity, aOther.quantity));
	}
	return true;
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

double orderData::calculateStopLoss(const algorithm::moneyMaker& aMM) const {
	const auto liqPrice = market::marketData::getLiquidationPrice(price, notionalValue, aMM.getLeverage(), quantity, state == algorithm::eState::LONG);
	auto stopLossSign = (state == algorithm::eState::LONG) ? 1 : -1;
	auto result = liqPrice * (100 + stopLossSign * aMM.getLiquidationOffsetPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

double orderData::calculateMinimumProfit(const algorithm::moneyMaker& aMM) const {
	auto minProfitSign = (state == algorithm::eState::LONG) ? 1 : -1;
	auto result = price * (100.0 + minProfitSign * aMM.getMinimumProfitPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

bool orderData::openOrder(const algorithm::moneyMaker& aMM, double aPrice) {
	reset();
	const auto quotePrecision = market::marketData::getInstance()->getQuotePrecision();
	const auto allowedCash = utils::floor(aMM.getCash() * aMM.getDealPercent() / 100.0, quotePrecision);
	const auto allowedNotionalValue = allowedCash * aMM.getLeverage();
	const auto calcQuantity = utils::floor(allowedNotionalValue / aPrice, MARKET_DATA->getQuantityPrecision());
	const auto calcNotionalValue = utils::round(calcQuantity * aPrice, quotePrecision);
	if (calcQuantity < MARKET_DATA->getQuantityPrecision() || calcNotionalValue < MARKET_DATA->getMinNotionalValue()) {
		std::cout << "[WARNING] orderData::openOrder can't open order\n";
		return false;
	}

	state = aMM.getState();
	fullCheck = aMM.getFullCheck();
	price = aPrice;
	quantity = calcQuantity;
	notionalValue = calcNotionalValue;
	margin = utils::round(notionalValue / aMM.getLeverage(), quotePrecision);

	time = aMM.getCandle().time;
	minimumProfit = calculateMinimumProfit(aMM);
	stopLoss = calculateStopLoss(aMM);
	return true;
}

double orderData::getProfit() const {
	auto quotePrecision = market::marketData::getInstance()->getQuotePrecision();
	const auto orderCloseSummary = utils::round(quantity * stopLoss, quotePrecision);
	const auto orderCloseTax = utils::round(orderCloseSummary * algorithmData::tax, quotePrecision);
	auto profitWithoutTax = (state == algorithm::eState::LONG) ? orderCloseSummary - notionalValue : notionalValue - orderCloseSummary;
	return profitWithoutTax - orderCloseTax;
}