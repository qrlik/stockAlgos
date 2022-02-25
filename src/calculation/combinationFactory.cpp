#include "combinationFactory.h"

using namespace calculation;

namespace {
	template<typename T>
	std::vector<T> iotaWithStep(T begin, T end, T step) {
		auto count = static_cast<int>(std::ceil((end - begin) / step));
		std::vector<T> result;
		result.reserve(count);
		for (; begin < end; begin += step) {
			result.push_back(begin);
		}
		return result;
	}
}

combinationFactory::combinationFactory() {
	iterateCombination();
}

size_t combinationFactory::getCombinationsAmount() const {
	return combinations;
}

void combinationFactory::iterateCombination() {
	combinations = 0;
	data = algorithmData{};
	generateSuperTrend();
}

void combinationFactory::iterateCombination(iterateCallback aCallback) {
	combinations = 0;
	callback = std::move(aCallback);
	data = algorithmData{};
	generateSuperTrend();
	callback = nullptr;
}

void combinationFactory::generateSuperTrend() {
	for (auto atrType : { indicators::eAtrType::RMA, indicators::eAtrType::EMA, indicators::eAtrType::WMA, indicators::eAtrType::SMA }) {
		data.atrType = atrType;
		for (auto atrSize : iotaWithStep(minAtrSize, maxAtrSize + stepInt, stepInt)) {
			data.atrSize = atrSize;
			for (auto stFactor : iotaWithStep(minStFactor, maxStFactor + stepFloat, stepFloat)) {
				data.stFactor = stFactor;
				generateDeal();
			}
		}
	}
}

void combinationFactory::generateDeal() {
	for (auto dealPercent : iotaWithStep(minDealPercent, maxDealPercent + stepFloat, stepFloat)) {
		data.dealPercent = dealPercent;
		for (auto leverage : iotaWithStep(minLeverage, maxLeverage + stepInt, stepInt)) {
			data.leverage = leverage;
			generatePercent();
		}
	}
}

void combinationFactory::generatePercent() {
	const auto liquidationPercent = 100 / data.leverage;
	for (auto activationPercent : iotaWithStep(minActivationPercent, liquidationPercent + stepFloat, stepFloat)) {
		data.activationPercent = activationPercent;
		for (auto stopLossPercent : iotaWithStep(std::max(activationPercent, minStopLossPercent), liquidationPercent + stepFloat, stepFloat)) {
			data.stopLossPercent = stopLossPercent;
			for (auto minimumProfitPercent : iotaWithStep(minMinProfitPercent, maxMinProfitPercent + stepFloat, stepFloat)) {
				data.minimumProfitPercent = minimumProfitPercent;
				generateDynamicSL();
			}
		}
	}
}

void combinationFactory::generateDynamicSL() {
	for (auto dynamicSLTrendMode : { true, false }) {
		data.dynamicSLTrendMode = dynamicSLTrendMode;
		if (!dynamicSLTrendMode) {
			const auto liquidationPercent = 100 / data.leverage;
			for (auto dynamicSLPercent : iotaWithStep(minDynamicSLPercent, liquidationPercent + stepFloat, stepFloat)) {
				data.dynamicSLPercent = dynamicSLPercent;
				generateOpener();
			}
		}
		else {
			data.dynamicSLPercent = -1.0;
			generateOpener();
		}
	}
}

void combinationFactory::generateOpener() {
	for (auto touchOpenerActivationWaitMode : { true, false }) {
		data.touchOpenerActivationWaitMode = touchOpenerActivationWaitMode;
		for (auto breakOpenerEnabled : { true, false }) {
			data.breakOpenerEnabled = breakOpenerEnabled;
			if (breakOpenerEnabled) {
				for (auto breakOpenerActivationWaitMode : { true, false }) {
					data.breakOpenerActivationWaitMode = breakOpenerActivationWaitMode;
					for (auto alwaysUseNewTrend : { true, false }) {
						data.alwaysUseNewTrend = alwaysUseNewTrend;
						generateActivation();
					}
				}
			}
			else {
				data.breakOpenerActivationWaitMode = false;
				data.alwaysUseNewTrend = false;
				generateActivation();
			}
		}
	}
}

void combinationFactory::generateActivation() {
	if (data.breakOpenerActivationWaitMode || data.touchOpenerActivationWaitMode) {
		for (auto activationWaiterResetAllowed : { true, false }) {
			data.activationWaiterResetAllowed = activationWaiterResetAllowed;
			for (auto activationWaiterRange : iotaWithStep(minTrendActivationWaitRange, maxTrendActivationWaitRange + 1, 1)) {
				data.activationWaiterRange = activationWaiterRange;
				for (auto activationWaiterFullCandleCheck : { true, false }) {
					data.activationWaiterFullCandleCheck = activationWaiterFullCandleCheck;
					generateStop();
				}
			}
		}
	}
	else {
		data.activationWaiterResetAllowed = false;
		data.activationWaiterRange = -1;
		data.activationWaiterFullCandleCheck = false;
		generateStop();
	}
}

void combinationFactory::generateStop() {
	for (auto stopLossWaiterEnabled : { true, false }) {
		data.stopLossWaiterEnabled = stopLossWaiterEnabled;
		if (stopLossWaiterEnabled) {
			for (auto stopLossWaiterResetAllowed : { true, false }) {
				data.stopLossWaiterResetAllowed = stopLossWaiterResetAllowed;
				for (auto stopLossWaiterRange : iotaWithStep(minStopLossWaitRange, maxStopLossWaitRange + 1, 1)) {
					data.stopLossWaiterRange = stopLossWaiterRange;
					for (auto stopLossWaiterFullCandleCheck : { true, false }) {
						data.stopLossWaiterFullCandleCheck = stopLossWaiterFullCandleCheck;
						onIterate();
					}
				}
			}
		}
		else {
			data.stopLossWaiterResetAllowed = false;
			data.stopLossWaiterRange = -1;
			data.stopLossWaiterFullCandleCheck = false;
			onIterate();
		}
	}
}

void combinationFactory::onIterate() {
	++combinations;
	if (callback) {
		callback(data, combinations);
	}
}