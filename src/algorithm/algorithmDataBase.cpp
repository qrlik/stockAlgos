#include "algorithmDataBase.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

bool algorithmDataBase::operator==(const algorithmDataBase& aOther) const {
	bool result = true;
	result &= indicatorsData == aOther.indicatorsData;
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

bool algorithmDataBase::isValid() const {
	auto result = true;

	result &= indicatorsData.isValid();
	result &= dealPercent > 0.0 && dealPercent < 100.0;
	result &= leverage > 0 && leverage <= 125;

	result &= startCash > MARKET_DATA->getMinNotionalValue() / leverage;
	result &= startCash > maxLossCash;
	result &= orderSize < startCash;
	result &= maxLossPercent > 0.0 && maxLossPercent < 100.0;

	const auto minLiqPercent = (orderSize > 0.0)
		? MARKET_DATA->getLiquidationPercent(orderSize, leverage)
		: MARKET_DATA->getLeverageLiquidationRange(leverage).first;
	result &= liquidationOffsetPercent > 0.0 && liquidationOffsetPercent < minLiqPercent;
	result &= minimumProfitPercent > 2 * MARKET_DATA->getTaxFactor() * 100.0;

	result &= isValidInternal();
	return result;
}

bool algorithmDataBase::initFromJson(const Json& aValue) {
	if (aValue.is_null() || aValue.empty()) {
		return false;
	}
	auto result = true;
	for (const auto& [key, value] : aValue.items()) {
		result &= initDataField(key, value);
	}
	return result;
}

bool algorithmDataBase::initDataField(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return false;
	}
	if (aName == "dealPercent") {
		dealPercent = aValue.get<double>();
		return true;
	}
	else if (aName == "orderSize") {
		orderSize = aValue.get<double>();
		return true;
	}
	else if (aName == "leverage") {
		leverage = aValue.get<int>();
		return true;
	}
	else if (aName == "startCash") {
		startCash = aValue.get<double>();
		return true;
	}
	else if (aName == "maxLossPercent") {
		maxLossPercent = aValue.get<double>();
		return true;
	}
	else if (aName == "maxLossCash") {
		maxLossCash = aValue.get<double>();
		return true;
	}
	else if (aName == "liquidationOffsetPercent") {
		liquidationOffsetPercent = aValue.get<double>();
		return true;
	}
	else if (aName == "minimumProfitPercent") {
		minimumProfitPercent = aValue.get<double>();
		return true;
	}
	else if (aName == "fullCheck") {
		fullCheck = aValue.get<bool>();
		return true;
	}
	else if (indicatorsData.initDataField(aName, aValue)) {
		return true;
	}
	return initDataFieldInternal(aName, aValue);
}

bool algorithmDataBase::checkCriteria(const std::string& aName, const Json& aValue) const {
	if (aValue.is_null()) {
		return false;
	}
	if (aName == "dealPercent") {
		return utils::isEqual(dealPercent, aValue.get<double>());
	}
	else if (aName == "orderSize") {
		return utils::isEqual(orderSize, aValue.get<double>());
	}
	else if (aName == "leverage") {
		return leverage == aValue.get<int>();
	}
	else if (aName == "startCash") {
		return utils::isEqual(startCash, aValue.get<double>());
	}
	else if (aName == "maxLossPercent") {
		return utils::isEqual(maxLossPercent, aValue.get<double>());
	}
	else if (aName == "maxLossCash") {
		return utils::isEqual(maxLossCash, aValue.get<double>());
	}
	else if (aName == "liquidationOffsetPercent") {
		return utils::isEqual(liquidationOffsetPercent, aValue.get<double>());
	}
	else if (aName == "minimumProfitPercent") {
		return utils::isEqual(minimumProfitPercent, aValue.get<double>());
	}
	else if (aName == "fullCheck") {
		return fullCheck == aValue.get<bool>();
	}
	else if (indicatorsData.checkCriteria(aName, aValue)) {
		return true;
	}
	return checkCriteriaInternal(aName, aValue);
}

void algorithmDataBase::addJsonData(Json& aData) const {
	aData["dealPercent"] = dealPercent;
	aData["orderSize"] = orderSize;
	aData["leverage"] = leverage;
	aData["startCash"] = startCash;
	aData["maxLossPercent"] = maxLossPercent;
	aData["maxLossCash"] = maxLossCash;
	aData["liquidationOffsetPercent"] = liquidationOffsetPercent;
	aData["minimumProfitPercent"] = minimumProfitPercent;
	indicatorsData.addJsonData(aData);
	addJsonDataInternal(aData);
}