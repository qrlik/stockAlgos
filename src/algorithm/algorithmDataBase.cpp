#include "algorithmDataBase.h"
#include "utils/utils.h"

using namespace algorithm;

algorithmDataBase::algorithmDataBase(const Json& aValue) {
	for (const auto& [field, value] : aValue.items()) {
		initDataField(field, value);
	}
}

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

void algorithmDataBase::initDataField(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	if (aName == "dealPercent") {
		dealPercent = aValue.get<double>();
		return;
	}
	else if (aName == "orderSize") {
		orderSize = aValue.get<double>();
		return;
	}
	else if (aName == "leverage") {
		leverage = aValue.get<int>();
		return;
	}
	else if (aName == "startCash") {
		startCash = aValue.get<double>();
		return;
	}
	else if (aName == "maxLossPercent") {
		maxLossPercent = aValue.get<double>();
		return;
	}
	else if (aName == "maxLossCash") {
		maxLossCash = aValue.get<double>();
		return;
	}
	else if (aName == "liquidationOffsetPercent") {
		liquidationOffsetPercent = aValue.get<double>();
		return;
	}
	else if (aName == "minimumProfitPercent") {
		minimumProfitPercent = aValue.get<double>();
		return;
	}
	else if (aName == "fullCheck") {
		fullCheck = aValue.get<bool>();
		return;
	}
	initDataFieldInternal(aName, aValue);
}
