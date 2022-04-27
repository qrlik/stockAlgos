#include "stAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;

const double stAlgorithmData::tax = 0.0004;

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

void stAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aValue.is_null()) {
		return;
	}
	if (aName == "atrType") {
		atrType = market::atrTypeFromString(aValue.get<std::string>());
	}
	else if (aName == "atrSize") {
		atrSize = aValue.get<int>();
	}
	else if (aName == "stFactor") {
		stFactor = aValue.get<double>();
	}
	else if (aName == "dynamicSLPercent") {
		dynamicSLPercent = aValue.get<double>();
	}
	else if (aName == "dynamicSLTrendMode") {
		dynamicSLTrendMode = aValue.get<bool>();
	}
	else if (aName == "touchOpenerActivationWaitMode") {
		touchOpenerActivationWaitMode = aValue.get<bool>();
	}
	else if (aName == "breakOpenerEnabled") {
		breakOpenerEnabled = aValue.get<bool>();
	}
	else if (aName == "breakOpenerActivationWaitMode") {
		breakOpenerActivationWaitMode = aValue.get<bool>();
	}
	else if (aName == "alwaysUseNewTrend") {
		alwaysUseNewTrend = aValue.get<bool>();
	}
	else if (aName == "activationWaiterResetAllowed") {
		activationWaiterResetAllowed = aValue.get<bool>();
	}
	else if (aName == "activationWaiterRange") {
		activationWaiterRange = aValue.get<int>();
	}
	else if (aName == "activationWaiterFullCandleCheck") {
		activationWaiterFullCandleCheck = aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterEnabled") {
		stopLossWaiterEnabled = aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterResetAllowed") {
		stopLossWaiterResetAllowed = aValue.get<bool>();
	}
	else if (aName == "stopLossWaiterRange") {
		stopLossWaiterRange = aValue.get<int>();
	}
	else if (aName == "stopLossWaiterFullCandleCheck") {
		stopLossWaiterFullCandleCheck = aValue.get<bool>();
	}
}