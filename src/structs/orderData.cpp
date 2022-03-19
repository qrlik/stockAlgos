#include "orderData.h"
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

double orderData::getLiquidationPrice() const {
    const auto& tierData = market::marketData::getInstance()->getTierData(notionalValue);
    const auto sign = (true) ? 1 : -1;
    const auto upper = margin + tierData.maintenanceAmount - sign * notionalValue;
    const auto lower = quantity * tierData.maintenanceMarginRate - sign * quantity;
    return upper / lower; // round precision
}

std::string orderData::toString() const {
	std::ostringstream ostr;
	ostr << std::setw(6) << "p: " << std::setw(12) << std::to_string(price)
		<< std::setw(6) << "s: " << std::setw(12) << std::to_string(stopLoss)
		<< std::setw(6) << "a: " << std::setw(12) << std::to_string(margin);
	return ostr.str();
}

bool orderData::operator==(const orderData& aOther) {
    assert(fullCheck == aOther.fullCheck);
    assert(utils::round(price, 5) == utils::round(aOther.price, 5));
    assert(time == aOther.time);
    if (fullCheck) {
        assert(stopLoss == aOther.stopLoss);
        assert(minimumProfit == aOther.minimumProfit);
        assert(margin == aOther.margin);
        assert(notionalValue == aOther.notionalValue);
    }
    return true;
}