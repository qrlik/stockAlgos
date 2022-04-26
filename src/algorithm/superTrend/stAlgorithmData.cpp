#include "stAlgorithmData.h"

using namespace algorithm;

const double stAlgorithmData::tax = 0.0004;

stAlgorithmData stAlgorithmData::initAlgorithmDataFromJson(const Json& aData) {
	stAlgorithmData result;
	for (const auto& [field, value] : aData.items()) {
		if (field == "atrType") {
			result.atrType = market::atrTypeFromString(value.get<std::string>());
		}
		else if (field == "atrSize") {
			result.atrSize = value.get<int>();
		}
		else if (field == "stFactor") {
			result.stFactor = value.get<double>();
		}
		else if (field == "dealPercent") {
			result.setDealPercent(value.get<double>());
		}
		else if (field == "leverage") {
			result.setLeverage(value.get<int>());
		}
		else if (field == "startCash") {
			result.setStartCash(value.get<double>());
		}
		else if (field == "maxLossPercent") {
			result.setMaxLossPercent(value.get<double>());
		}
		else if (field == "maxLossCash") {
			result.setMaxLossCash(value.get<double>());
		}
		else if (field == "liquidationOffsetPercent") {
			result.setLiquidationOffsetPercent(value.get<double>());
		}
		else if (field == "minimumProfitPercent") {
			result.setMinimumProfitPercent(value.get<double>());
		}
		else if (field == "dynamicSLPercent" && !value.is_null()) {
			result.dynamicSLPercent = value.get<double>();
		}
		else if (field == "dynamicSLTrendMode" && !value.is_null()) {
			result.dynamicSLTrendMode = value.get<bool>();
		}
		else if (field == "touchOpenerActivationWaitMode") {
			result.touchOpenerActivationWaitMode = value.get<bool>();
		}
		else if (field == "breakOpenerEnabled") {
			result.breakOpenerEnabled = value.get<bool>();
		}
		else if (field == "breakOpenerActivationWaitMode" && !value.is_null()) {
			result.breakOpenerActivationWaitMode = value.get<bool>();
		}
		else if (field == "alwaysUseNewTrend" && !value.is_null()) {
			result.alwaysUseNewTrend = value.get<bool>();
		}
		else if (field == "activationWaiterResetAllowed" && !value.is_null()) {
			result.activationWaiterResetAllowed = value.get<bool>();
		}
		else if (field == "activationWaiterRange" && !value.is_null()) {
			result.activationWaiterRange = value.get<int>();
		}
		else if (field == "activationWaiterFullCandleCheck" && !value.is_null()) {
			result.activationWaiterFullCandleCheck = value.get<bool>();
		}
		else if (field == "stopLossWaiterEnabled") {
			result.stopLossWaiterEnabled = value.get<bool>();
		}
		else if (field == "stopLossWaiterResetAllowed" && !value.is_null()) {
			result.stopLossWaiterResetAllowed = value.get<bool>();
		}
		else if (field == "stopLossWaiterRange" && !value.is_null()) {
			result.stopLossWaiterRange = value.get<int>();
		}
		else if (field == "stopLossWaiterFullCandleCheck" && !value.is_null()) {
			result.stopLossWaiterFullCandleCheck = value.get<bool>();
		}
		else if (field == "fullCheck") {
			result.setFullCheck(value.get<bool>());
		}
	}
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

}