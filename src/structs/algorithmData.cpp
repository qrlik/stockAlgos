#include "algorithmData.h"

const double algorithmData::tax = 0.0004;

algorithmData algorithmData::initAlgorithmDataFromJson(const Json& aData) {
	algorithmData result;
	for (const auto& [field, value] : aData.items()) {
		if (field == "atrType") {
			result.atrType = indicators::atrTypeFromString(value.get<std::string>());
		}
		else if (field == "atrSize") {
			result.atrSize = value.get<int>();
		}
		else if (field == "stFactor") {
			result.stFactor = value.get<double>();
		}
		else if (field == "dealPercent") {
			result.dealPercent = value.get<double>();
		}
		else if (field == "leverage") {
			result.leverage = value.get<int>();
		}
		else if (field == "activationPercent") {
			result.activationPercent = value.get<double>();
		}
		else if (field == "stopLossPercent") {
			result.stopLossPercent = value.get<double>();
		}
		else if (field == "minimumProfitPercent") {
			result.minimumProfitPercent = value.get<double>();
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
			result.fullCheck = value.get<bool>();
		}
	}
	return result;
}