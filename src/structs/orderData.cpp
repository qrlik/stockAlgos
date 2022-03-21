#include "orderData.h"
#include "../algorithm/moneyMaker.h"
#include "../market/marketRules.h"
#include "../utils/utils.h"
#include <sstream>
#include <iomanip>
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
        else if (field == "amount") {
            aData.margin = value.get<double>();
        }
        else if (field == "time") {
            aData.time = value.get<std::string>();
        }
        // check here later
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
    assert(price == aOther.price);
    assert(time == aOther.time);
    if (fullCheck) {
        assert(stopLoss == aOther.stopLoss);
        assert(minimumProfit == aOther.minimumProfit);
        assert(margin == aOther.margin);
        assert(notionalValue == aOther.notionalValue);
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

double orderData::calculateLiquidationPrice() const {
    const auto& tierData = MARKET_DATA->getTierData(notionalValue);
    const auto sign = (state == algorithm::eState::LONG) ? 1 : -1;
    const auto upper = margin + tierData.maintenanceAmount - sign * notionalValue;
    const auto lower = quantity * tierData.maintenanceMarginRate - sign * quantity;
    if (state == algorithm::eState::LONG) {
        return utils::ceil(upper / lower, MARKET_DATA->getPricePrecision());
    }
    return utils::floor(upper / lower, MARKET_DATA->getPricePrecision());
}

double orderData::calculateStopLoss(const algorithm::moneyMaker& aMM) const {
    const auto liqPrice = calculateLiquidationPrice();
    auto stopLossSign = (state == algorithm::eState::LONG) ? 1 : -1;
    auto result = liqPrice * (100 + stopLossSign * aMM.getLiquidationOffsetPercent()) / 100.0;
    return utils::round(result, MARKET_DATA->getPricePrecision());
}

double orderData::calculateMinimumProfit(const algorithm::moneyMaker& aMM) const {
    auto minProfitSign = (state == algorithm::eState::LONG) ? 1 : -1;
    auto result = price * (100.0 + minProfitSign * aMM.getMinimumProfitPercent()) / 100.0;
    return utils::round(result, MARKET_DATA->getPricePrecision());
}

void orderData::openOrder(const algorithm::moneyMaker& aMM, double aPrice) {
    reset();
    state = aMM.getState();
    fullCheck = aMM.getFullCheck();
    price = aPrice;

    const auto allowedCash = aMM.getCash() * aMM.getDealPercent() / 100.0;
    const auto allowedNotionalValue = allowedCash * aMM.getLeverage();
    quantity = utils::floor(allowedNotionalValue / aPrice, MARKET_DATA->getTradePrecision());
    notionalValue = quantity * aPrice;
    margin = notionalValue / aMM.getLeverage();

    minimumProfit = calculateMinimumProfit(aMM);
    stopLoss = calculateStopLoss(aMM);
    time = aMM.getCandle().time;
}

double orderData::getProfit() const {
    const auto orderCloseSummary = quantity * stopLoss;
    const auto orderCloseTax = orderCloseSummary * algorithmData::tax;
    auto profitWithoutTax = (state == algorithm::eState::LONG) ? orderCloseSummary - notionalValue : notionalValue - orderCloseSummary;
    return profitWithoutTax - orderCloseTax;
}