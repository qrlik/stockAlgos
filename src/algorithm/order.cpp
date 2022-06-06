#include "order.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

order::order() : state(eOrderState::NONE) {}

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

double order::calculateStopLoss(const algorithm::algorithmDataBase& aData) const {
	if (auto stopLossPercent = aData.getStopLossPercent(); utils::isGreater(stopLossPercent, 0.0)) {
		auto stopLossSign = (state == eOrderState::LONG) ? -1 : 1;
		auto result = price * (100 + stopLossSign * stopLossPercent) / 100.0;
		return utils::round(result, MARKET_DATA->getPricePrecision());
	}
	const auto liqPrice = MARKET_DATA->getLiquidationPrice(price, notionalValue, aData.getLeverage(), quantity, state == eOrderState::LONG);
	auto stopLossSign = (state == eOrderState::LONG) ? 1 : -1;
	auto result = liqPrice * (100 + stopLossSign * aData.getLiquidationOffsetPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

double order::calculateMinimumProfit(const algorithm::algorithmDataBase& aData) const {
	auto minProfitSign = (state == eOrderState::LONG) ? 1 : -1;
	auto result = price * (100.0 + minProfitSign * aData.getMinimumProfitPercent()) / 100.0;
	return utils::round(result, MARKET_DATA->getPricePrecision());
}

bool order::openOrder(const algorithm::algorithmDataBase& aData, eOrderState aState, double aPrice, double aCash, const std::string& aTime) {
	reset();
	const auto quotePrecision = MARKET_DATA->getQuotePrecision();
	auto allowedCash = utils::floor(aCash * aData.getDealPercent() / 100.0, quotePrecision);
	if (const auto allowedCashBySize = utils::floor(aData.getOrderSize(), quotePrecision); utils::isGreater(allowedCashBySize, 0.0)) {
		allowedCash = utils::minFloat(allowedCash, allowedCashBySize);
	}
	const auto allowedNotionalValue = utils::minFloat(allowedCash * aData.getLeverage(), MARKET_DATA->getLeverageMaxPosition(aData.getLeverage()));
	const auto calcQuantity = utils::floor(allowedNotionalValue / aPrice, MARKET_DATA->getQuantityPrecision());
	const auto calcNotionalValue = utils::round(calcQuantity * aPrice, quotePrecision);
	if (utils::isLess(calcQuantity, MARKET_DATA->getQuantityPrecision()) || utils::isLess(calcNotionalValue, MARKET_DATA->getMinNotionalValue())) {
		utils::logError("orderData::openOrder can't open order");
		return false;
	}

	state = aState;
	fullCheck = aData.getFullCheck();
	price = aPrice;
	quantity = calcQuantity;
	notionalValue = calcNotionalValue;
	margin = utils::round(notionalValue / aData.getLeverage(), quotePrecision);

	time = aTime;
	minimumProfit = calculateMinimumProfit(aData);
	initStopLoss = calculateStopLoss(aData);
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
	auto quotePrecision = MARKET_DATA->getQuotePrecision();
	const auto orderCloseSummary = utils::round(quantity * stopLoss, quotePrecision);
	const auto orderCloseTax = utils::round(orderCloseSummary * MARKET_DATA->getTaxFactor(), quotePrecision);
	auto profitWithoutTax = (state == eOrderState::LONG) ? orderCloseSummary - notionalValue : notionalValue - orderCloseSummary;
	return profitWithoutTax - orderCloseTax;
}

void order::initFromJson(const algorithm::algorithmDataBase& aAlgData, const Json& aJson) {
	fullCheck = aAlgData.getFullCheck();
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
