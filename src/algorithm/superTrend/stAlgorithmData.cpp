#include "stAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;

stAlgorithmData::stAlgorithmData(const Json& aValue) {
	initFromJson(aValue);
}

bool stAlgorithmData::operator==(const stAlgorithmData& aOther) const {
	bool result = baseClass::operator==(aOther);
	result &= atrType == aOther.atrType;
	result &= atrSize == aOther.atrSize;
	result &= utils::isEqual(stFactor, aOther.stFactor);
	result &= dynamicSLTrendMode == aOther.dynamicSLTrendMode;
	result &= utils::isEqual(dynamicSLPercent, aOther.dynamicSLPercent);
	result &= touchOpenerActivationWaitMode == aOther.touchOpenerActivationWaitMode;
	result &= breakOpenerEnabled == aOther.breakOpenerEnabled;
	result &= breakOpenerActivationWaitMode == aOther.breakOpenerActivationWaitMode;
	result &= alwaysUseNewTrend == aOther.alwaysUseNewTrend;
	result &= activationWaiterResetAllowed == aOther.activationWaiterResetAllowed;
	result &= activationWaiterRange == aOther.activationWaiterRange;
	result &= activationWaiterFullCandleCheck == aOther.activationWaiterFullCandleCheck;
	result &= stopLossWaiterEnabled == aOther.stopLossWaiterEnabled;
	result &= stopLossWaiterResetAllowed == aOther.stopLossWaiterResetAllowed;
	result &= stopLossWaiterRange == aOther.stopLossWaiterRange;
	result &= stopLossWaiterFullCandleCheck == aOther.stopLossWaiterFullCandleCheck;
	return result;
}

bool stAlgorithmData::isValidInternal() const {
	auto result = true;
	result &= atrType != market::eAtrType::NONE;
	result &= atrSize > 1;
	result &= stFactor >= 1.0;

	result &= (utils::isEqual(dynamicSLPercent, -1.0) && dynamicSLTrendMode) || dynamicSLPercent > 0.0;

	auto waiter = touchOpenerActivationWaitMode;
	if (breakOpenerEnabled) {
		waiter &= breakOpenerActivationWaitMode;
	}
	else {
		result &= !breakOpenerActivationWaitMode;
		result &= !alwaysUseNewTrend;
	}
	if (!waiter) {
		result &= !activationWaiterResetAllowed;
		result &= !activationWaiterFullCandleCheck;
	}
	result &= (waiter) ? activationWaiterRange >= 0 : activationWaiterRange == -1;
	if (!stopLossWaiterEnabled) {
		result &= !stopLossWaiterResetAllowed;
		result &= !stopLossWaiterFullCandleCheck;
	}
	result &= (stopLossWaiterEnabled) ? stopLossWaiterRange >= 0 : stopLossWaiterRange == -1;
	return result;
}

Json stAlgorithmData::toJson() const {
	Json result;
	result["atrType"] = market::atrTypeToString(atrType);
	result["atrSize"] = atrSize;
	result["stFactor"] = stFactor;

	result["dealPercent"] = getDealPercent();
	result["leverage"] = getLeverage();

	result["startCash"] = getStartCash();
	result["maxLossPercent"] = getMaxLossPercent();
	result["maxLossCash"] = getMaxLossCash();

	result["liquidationOffsetPercent"] = getLiquidationOffsetPercent();
	result["minimumProfitPercent"] = getMinimumProfitPercent();

	result["dynamicSLPercent"] = dynamicSLPercent;
	result["dynamicSLTrendMode"] = dynamicSLTrendMode;

	result["touchOpenerActivationWaitMode"] = touchOpenerActivationWaitMode;

	result["breakOpenerEnabled"] = breakOpenerEnabled;
	result["breakOpenerActivationWaitMode"] = breakOpenerActivationWaitMode;
	result["alwaysUseNewTrend"] = alwaysUseNewTrend;

	result["activationWaiterResetAllowed"] = activationWaiterResetAllowed;
	result["activationWaiterRange"] = activationWaiterRange;
	result["activationWaiterFullCandleCheck"] = activationWaiterFullCandleCheck;

	result["stopLossWaiterEnabled"] = stopLossWaiterEnabled;
	result["stopLossWaiterResetAllowed"] = stopLossWaiterResetAllowed;
	result["stopLossWaiterRange"] = stopLossWaiterRange;
	result["stopLossWaiterFullCandleCheck"] = stopLossWaiterFullCandleCheck;

	return result;
}

bool stAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return false;
	}
	if (aName == "atrType") {
		atrType = market::atrTypeFromString(aValue.get<std::string>());
		return true;
	}
	else if (aName == "atrSize") {
		atrSize = aValue.get<int>();
		return true;
	}
	else if (aName == "stFactor") {
		stFactor = aValue.get<double>();
		return true;
	}
	else if (aName == "dynamicSLPercent") {
		dynamicSLPercent = aValue.get<double>();
		return true;
	}
	else if (aName == "dynamicSLTrendMode") {
		dynamicSLTrendMode = aValue.get<bool>();
		return true;
	}
	else if (aName == "touchOpenerActivationWaitMode") {
		touchOpenerActivationWaitMode = aValue.get<bool>();
		return true;
	}
	else if (aName == "breakOpenerEnabled") {
		breakOpenerEnabled = aValue.get<bool>();
		return true;
	}
	else if (aName == "breakOpenerActivationWaitMode") {
		breakOpenerActivationWaitMode = aValue.get<bool>();
		return true;
	}
	else if (aName == "alwaysUseNewTrend") {
		alwaysUseNewTrend = aValue.get<bool>();
		return true;
	}
	else if (aName == "activationWaiterResetAllowed") {
		activationWaiterResetAllowed = aValue.get<bool>();
		return true;
	}
	else if (aName == "activationWaiterRange") {
		activationWaiterRange = aValue.get<int>();
		return true;
	}
	else if (aName == "activationWaiterFullCandleCheck") {
		activationWaiterFullCandleCheck = aValue.get<bool>();
		return true;
	}
	else if (aName == "stopLossWaiterEnabled") {
		stopLossWaiterEnabled = aValue.get<bool>();
		return true;
	}
	else if (aName == "stopLossWaiterResetAllowed") {
		stopLossWaiterResetAllowed = aValue.get<bool>();
		return true;
	}
	else if (aName == "stopLossWaiterRange") {
		stopLossWaiterRange = aValue.get<int>();
		return true;
	}
	else if (aName == "stopLossWaiterFullCandleCheck") {
		stopLossWaiterFullCandleCheck = aValue.get<bool>();
		return true;
	}
	return false;
}

bool stAlgorithmData::checkCriteriaInternal(const std::string& aName, const Json& aValue) const {
	if (aValue.is_null()) {
		return false;
	}
	if (aName == "atrType") {
		return atrType == market::atrTypeFromString(aValue.get<std::string>());
	}
	else if (aName == "atrSize") {
		return atrSize == aValue.get<int>();
	}
	else if (aName == "stFactor") {
		return utils::isEqual(stFactor, aValue.get<double>());
	}
	else if (aName == "dynamicSLPercent") {
		return utils::isEqual(dynamicSLPercent, aValue.get<double>());
	}
	else if (aName == "dynamicSLTrendMode") {
		return dynamicSLTrendMode == aValue.get<bool>();
	}
	else if (aName == "touchOpenerActivationWaitMode") {
		return touchOpenerActivationWaitMode == aValue.get<bool>();
	}
	else if (aName == "breakOpenerEnabled") {
		return breakOpenerEnabled == aValue.get<bool>();
	}
	else if (aName == "breakOpenerActivationWaitMode") {
		return breakOpenerActivationWaitMode == aValue.get<bool>();
	}
	else if (aName == "alwaysUseNewTrend") {
		return alwaysUseNewTrend == aValue.get<bool>();
	}
	else if (aName == "activationWaiterResetAllowed") {
		return activationWaiterResetAllowed == aValue.get<bool>();
	}
	else if (aName == "activationWaiterRange") {
		return activationWaiterRange == aValue.get<int>();
	}
	else if (aName == "activationWaiterFullCandleCheck") {
		return activationWaiterFullCandleCheck == aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterEnabled") {
		return stopLossWaiterEnabled == aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterResetAllowed") {
		return stopLossWaiterResetAllowed == aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterRange") {
		return stopLossWaiterRange == aValue.get<int>();
	}
	else if (aName == "stopLossWaiterFullCandleCheck") {
		return stopLossWaiterFullCandleCheck == aValue.get<bool>();
	}
	return false;
}