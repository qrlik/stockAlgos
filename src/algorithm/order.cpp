#include "order.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

namespace {
	double minLiquidationPercent = std::numeric_limits<double>::max();
}

double algorithm::getMinLiquidationPercent() {
	return minLiquidationPercent;
}

order::order(const algorithm::algorithmDataBase& data) : mData(data) {}

std::string order::toString() const {
	std::ostringstream ostr;
	ostr << std::setw(6) << "p: " << std::setw(12) << std::to_string(price)
		<< std::setw(6) << "s: " << std::setw(12) << std::to_string(stopLoss)
		<< std::setw(6) << "a: " << std::setw(12) << std::to_string(margin);
	return ostr.str();
}

bool order::operator==(const order& aOther) const {
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

void order::reset() {
	time.clear();
	price = 0.0;
	initStopLoss = 0.0;
	stopLoss = 0.0;
	minimumProfit = 0.0;
	margin = 0.0;
	notionalValue = 0.0;
	quantity = 0.0;
	state = eOrderState::NONE;
}

void order::updateStopLoss(double aStopLoss) {
	if (state == eOrderState::LONG) {
		stopLoss = utils::maxFloat(initStopLoss, aStopLoss);
	}
	else if (state == eOrderState::SHORT) {
		stopLoss = utils::minFloat(initStopLoss, aStopLoss);
	}
}

bool order::checkPriceNearLiquidation(double liqPrice, double precision) const {
	if (utils::isLess(std::abs(liqPrice - price), precision * 1.5)) { // one tick between price and liquidation
		auto errorData = mData.getBaseErrorData();
		errorData["liqPrice"] = liqPrice;
		errorData["price"] = price;
		errorData["precision"] = precision;
		utils::logError("order::calculateStopLoss wrong precision", std::move(errorData));
		return false;
	}
	return true;
}

bool order::checkStopLossBeyondLiquidation(double stopPrice, double liqPrice) const {
	if ((state == eOrderState::LONG && utils::isLessOrEqual(stopPrice, liqPrice))
		|| (state == eOrderState::SHORT && utils::isGreaterOrEqual(stopPrice, liqPrice))) {
		auto errorData = mData.getBaseErrorData();
		errorData["stopLoss"] = stopPrice;
		errorData["liqPrice"] = liqPrice;
		errorData["state"] = state;
		utils::logError("order::calculateStopLoss wrong stopLoss");
		return false;
	}
	return true;
}

bool order::checkStopLossBeyondPrice(double stopPrice) const {
	if ((state == eOrderState::LONG && utils::isGreaterOrEqual(stopPrice, price))
		|| (state == eOrderState::SHORT && utils::isLessOrEqual(stopPrice, price))) {
		auto errorData = mData.getBaseErrorData();
		errorData["checkedPrice"] = price;
		errorData["stopLoss"] = stopPrice;
		errorData["state"] = state;
		utils::logError("order::calculateStopLoss wrong stopLoss");
		return false;
	}
	return true;
}

double order::calculateStopLossByType(double liqPrice, double precision) const {
	double result = -1.0;

	if (utils::isGreater(mData.getStopLossPercent(), 0.0)) {
		const auto stopLossSign = (state == eOrderState::LONG) ? -1 : 1;
		result = price * (100 + stopLossSign * mData.getStopLossPercent()) / 100.0;
	}
	else if (utils::isGreater(mData.getLiquidationOffsetPercent(), 0.0)) {
		auto stopLossSign = (state == eOrderState::LONG) ? 1 : -1;
		result = liqPrice * (100 + stopLossSign * mData.getLiquidationOffsetPercent()) / 100.0;
	}

	if (state == eOrderState::LONG) {
		result = utils::ceil(result, precision);
	}
	else {
		result = utils::floor(result, precision);
	}

	if (utils::isGreater(mData.getStopLossPercent(), 0.0)) {
		if (!checkStopLossBeyondLiquidation(result, liqPrice)) {
			return -1.0;
		}
	}
	else if (utils::isGreater(mData.getLiquidationOffsetPercent(), 0.0)) {
		if (!checkStopLossBeyondPrice(result)) {
			return -1.0;
		}
	}

	return result;
}

double order::calculateStopLoss() const {
	const auto precision = mData.getMarketData().getPricePrecision(price);
	const auto liqPrice = mData.getMarketData().getLiquidationPrice(price, notionalValue, mData.getLeverage(), quantity, state == eOrderState::LONG);
	const auto actualLiqPercent = (state == eOrderState::LONG) ? (100.0 - liqPrice / price * 100.0) : (liqPrice / price * 100.0 - 100.0);
	minLiquidationPercent = utils::minFloat(minLiquidationPercent, actualLiqPercent);

	if (!checkPriceNearLiquidation(liqPrice, precision)) {
		return -1.0;
	}

	return calculateStopLossByType(liqPrice, precision);
}

double order::calculateMinimumProfit() const {
	auto minProfitSign = (state == eOrderState::LONG) ? 1 : -1;
	auto result = price * (100.0 + minProfitSign * mData.getMinimumProfitPercent()) / 100.0;
	return utils::round(result, mData.getMarketData().getPricePrecision(price));
}

bool order::openOrder(eOrderState aState, double aPrice, double aCash, const std::string& aTime) {
	reset();
	const auto& marketData = mData.getMarketData();
	const auto quotePrecision = marketData.getQuotePrecision();
	auto allowedCash = utils::floor(aCash * mData.getDealPercent() / 100.0, quotePrecision);
	if (const auto allowedCashBySize = utils::floor(mData.getOrderSize(), quotePrecision); utils::isGreater(allowedCashBySize, 0.0)) {
		allowedCash = utils::minFloat(allowedCash, allowedCashBySize);
	}

	const auto allowedNotionalValue = utils::minFloat(allowedCash * mData.getLeverage(), marketData.getLeverageMaxPosition(mData.getLeverage()));
	const auto calcQuantity = [allowedNotionalValue, aPrice, &marketData]() {
		const auto minQuantity = marketData.getQuantityPrecision();
		auto result = utils::floor(allowedNotionalValue / aPrice, minQuantity);
		return (utils::isGreaterOrEqual(result, minQuantity)) ? result : minQuantity;
	}(); 
	const auto calcNotionalValue = utils::round(calcQuantity * aPrice, quotePrecision);
	if (utils::isLess(calcQuantity, marketData.getQuantityPrecision()) || utils::isLess(calcNotionalValue, marketData.getMinNotionalValue())) {
		utils::logError("orderData::openOrder can't open order");
		return false;
	}

	state = aState;
	fullCheck = mData.getFullCheck();
	price = aPrice;
	quantity = calcQuantity;
	notionalValue = calcNotionalValue;
	margin = utils::round(notionalValue / mData.getLeverage(), quotePrecision);

	time = aTime;
	minimumProfit = calculateMinimumProfit();

	initStopLoss = calculateStopLoss();
	if (utils::isLessOrEqual(initStopLoss, 0.0)) {
		reset();
		return false;
	}
	stopLoss = initStopLoss;
	return true;
}

double order::closeOrder() {
	auto profit = std::numeric_limits<double>::min();
	if (state == eOrderState::LONG && utils::isLess(stopLoss, initStopLoss) ||
		state == eOrderState::SHORT && utils::isGreater(stopLoss, initStopLoss)) {
		utils::logError("order::closeOrder wrong stopLoss");
		return profit;
	}
	profit = getProfit();
	reset();
	return profit;
}

double order::getProfit() const {
	auto quotePrecision = mData.getMarketData().getQuotePrecision();
	const auto orderCloseSummary = utils::round(quantity * stopLoss, quotePrecision);
	const auto orderCloseTax = utils::round(orderCloseSummary * MARKET_SYSTEM->getTaxFactor(), quotePrecision);
	auto profitWithoutTax = (state == eOrderState::LONG) ? orderCloseSummary - notionalValue : notionalValue - orderCloseSummary;
	return profitWithoutTax - orderCloseTax;
}

void order::initFromJson(const Json& aJson) {
	fullCheck = mData.getFullCheck();
	for (const auto& [field, value] : aJson.items()) {
		if (value.is_null()) {
			continue;
		}
		if (field == "price") {
			price = value.get<double>();
		}
		else if (field == "stopLoss") {
			initStopLoss = value.get<double>();
			stopLoss = value.get<double>();
		}
		else if (field == "minimumProfit") {
			minimumProfit = value.get<double>();
		}
		else if (field == "margin") {
			margin = value.get<double>();
		}
		else if (field == "notionalValue") {
			notionalValue = value.get<double>();
		}
		else if (field == "quantity") {
			quantity = value.get<double>();
		}
		else if (field == "time") {
			time = value.get<std::string>();
		}
	}
}
