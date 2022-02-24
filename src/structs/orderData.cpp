#include "orderData.h"
#include "../utils/utils.h"
#include <sstream>
#include <iomanip>
#include <assert.h>

void orderData::initOrderDataFromJson(orderData& aData, Json aJson) {
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
            aData.amount = value.get<double>();
        }
        else if (field == "time") {
            aData.time = value.get<std::string>();
        }
    }
}

std::string orderData::toString() const {
	std::ostringstream ostr;
	ostr << std::setw(6) << "p: " << std::setw(12) << std::to_string(price)
		<< std::setw(6) << "s: " << std::setw(12) << std::to_string(stopLoss)
		<< std::setw(6) << "a: " << std::setw(12) << std::to_string(amount);
	return ostr.str();
}

bool orderData::operator==(const orderData& aOther) {
    assert(fullCheck == aOther.fullCheck);
    assert(price == aOther.price);
    assert(time == aOther.time);
    if (fullCheck) {
        assert(stopLoss == aOther.stopLoss);
        assert(minimumProfit == aOther.minimumProfit);
        assert(amount == aOther.amount);
    }
    return true;
}