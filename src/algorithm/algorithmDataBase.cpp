#include "algorithmDataBase.h"
#include "utils/utils.h"

using namespace algorithm;

bool algorithmDataBase::operator==(const algorithmDataBase& aOther) const {
	bool result = true;
	result &= utils::isEqual(dealPercent, aOther.dealPercent);
	result &= utils::isEqual(orderSize, aOther.orderSize);
	result &= utils::isEqual(startCash, aOther.startCash);
	result &= utils::isEqual(maxLossPercent, aOther.maxLossPercent);
	result &= utils::isEqual(maxLossCash, aOther.maxLossCash);
	result &= utils::isEqual(liquidationOffsetPercent, aOther.liquidationOffsetPercent);
	result &= utils::isEqual(minimumProfitPercent, aOther.minimumProfitPercent);
	result &= leverage == aOther.leverage;
	result &= fullCheck == aOther.fullCheck;
	return result;
}

void algorithmDataBase::initFromJson(const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	for (const auto& [key, value] : aValue.items()) {
		if (value.is_null()) {
			continue;
		}
		if (key == "dealPercent") {
			dealPercent = value.get<double>();
		}
		else if (key == "orderSize") {
			orderSize = value.get<double>();
		}
		else if (key == "leverage") {
			leverage = value.get<int>();
		}
		else if (key == "startCash") {
			startCash = value.get<double>();
		}
		else if (key == "maxLossPercent") {
			maxLossPercent = value.get<double>();
		}
		else if (key == "maxLossCash") {
			maxLossCash = value.get<double>();
		}
		else if (key == "liquidationOffsetPercent") {
			liquidationOffsetPercent = value.get<double>();
		}
		else if (key == "minimumProfitPercent") {
			minimumProfitPercent = value.get<double>();
		}
		else if (key == "fullCheck") {
			fullCheck = value.get<bool>();
		}
		else {
			initDataFieldInternal(key, value);
		}
	}
}
