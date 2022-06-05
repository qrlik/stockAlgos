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
	result &= utils::isEqual(stopLossPercent, aOther.stopLossPercent);
	result &= utils::isEqual(liquidationOffsetPercent, aOther.liquidationOffsetPercent);
	result &= utils::isEqual(minimumProfitPercent, aOther.minimumProfitPercent);
	result &= leverage == aOther.leverage;
	result &= fullCheck == aOther.fullCheck;
	result &= fullCheckCustom == aOther.fullCheckCustom;
	return result;
}

bool algorithmDataBase::isValid() const {
	auto result = true;

	result &= indicatorsData.isValid();
	result &= utils::isGreater(dealPercent, 0.0) && utils::isLess(dealPercent, 100.0);
	result &= leverage > 0 && leverage <= MARKET_DATA->getMaxLeverage();

	result &= utils::isGreater(startCash, MARKET_DATA->getMinNotionalValue() / leverage);
	result &= utils::isGreater(startCash, maxLossCash);
	result &= utils::isLess(orderSize, startCash);
	result &= utils::isGreater(maxLossPercent, 0.0) && utils::isLess(maxLossPercent, 100.0);

	const auto minLiqPercent = (utils::isGreater(orderSize, 0.0))
		? MARKET_DATA->getLiquidationPercent(orderSize, leverage)
		: MARKET_DATA->getLeverageLiquidationRange(leverage).first;
	result &= utils::isGreater(stopLossPercent, 0.0) || utils::isGreater(liquidationOffsetPercent, 0.0);
	result &= utils::isLess(stopLossPercent, minLiqPercent);
	result &= utils::isLess(liquidationOffsetPercent, minLiqPercent);

	result &= utils::isGreater(minimumProfitPercent, 2 * MARKET_DATA->getTaxFactor() * 100.0);

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
	else if (aName == "stopLossPercent") {
		stopLossPercent = aValue.get<double>();
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
	else if (aName == "fullCheckCustom") {
		fullCheckCustom = aValue.get<bool>();
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
	else if (aName == "stopLossPercent") {
		return utils::isEqual(stopLossPercent, aValue.get<double>());
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
	else if (aName == "fullCheckCustom") {
		return fullCheckCustom == aValue.get<bool>();
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
	aData["stopLossPercent"] = stopLossPercent;
	aData["liquidationOffsetPercent"] = liquidationOffsetPercent;
	aData["minimumProfitPercent"] = minimumProfitPercent;
	indicatorsData.addJsonData(aData);
	addJsonDataInternal(aData);
}