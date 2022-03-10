#include "combinationFactory.h"
#include <iostream>

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
	std::vector<algorithmData> tmpAllData;
	algorithmData tmpData;
}

combinationFactory::combinationFactory(size_t aThreadsCount) : 
	threadsCount(aThreadsCount),
	combinationsData(aThreadsCount),
	indexes(aThreadsCount, 0)
{
	generateSuperTrend();
	auto threadDataAmount = combinations / threadsCount;
	auto lastThreadDataAmount = threadDataAmount + combinations % threadsCount;
	for (size_t i = 0; i < threadsCount; ++i) {
		const bool isLast = i == 0;
		const auto index = threadsCount - 1 - i;
		const auto amount = (isLast) ? lastThreadDataAmount : threadDataAmount;
		combinationsData[index].reserve(amount);
		std::copy(tmpAllData.begin() + (tmpAllData.size() - amount), tmpAllData.end(), std::back_inserter(combinationsData[index]));
		tmpAllData.erase(tmpAllData.begin() + (tmpAllData.size() - amount), tmpAllData.end());
	}
	if (!tmpAllData.empty()) {
		std::cout << "[ERROR] combinationFactory tmpAllData not empty" << std::endl;
	}
	inited = true;
}

size_t combinationFactory::getCombinationsAmount() const {
	return combinations;
}

size_t combinationFactory::getCurrentIndex() const {
	return std::accumulate(indexes.cbegin(), indexes.cend(), size_t{ 0 });
}

const std::vector<algorithmData>& combinationFactory::getThreadData(int aThread) {
	return combinationsData[aThread];
}

void combinationFactory::incrementThreadIndex(int aThread) {
	++indexes[aThread];
}

void combinationFactory::onFinish() {
	if (std::accumulate(indexes.begin(), indexes.end(), size_t{ 0 }) != combinations) {
		std::cout << "[ERROR] combinationFactory onFinish combinations not correct" << std::endl;
	}
}

void combinationFactory::generateSuperTrend() {
	if (inited) {
		return;
	}
	tmpData = algorithmData{};
	for (auto atrType : { indicators::eAtrType::RMA, indicators::eAtrType::EMA, indicators::eAtrType::WMA, indicators::eAtrType::SMA }) {
		tmpData.atrType = atrType;
		for (auto atrSize : iotaWithStep(minAtrSize, maxAtrSize + stepInt, stepInt)) {
			tmpData.atrSize = atrSize;
			for (auto stFactor : iotaWithStep(minStFactor, maxStFactor + stepFloat, stepFloat)) {
				tmpData.stFactor = stFactor;
				generateDeal();
			}
		}
	}
}

void combinationFactory::generateDeal() {
	for (auto dealPercent : iotaWithStep(minDealPercent, maxDealPercent + stepFloat, stepFloat)) {
		tmpData.dealPercent = dealPercent;
		for (auto leverage : iotaWithStep(minLeverage, maxLeverage + stepInt, stepInt)) {
			tmpData.leverage = leverage;
			generatePercent();
		}
	}
}

void combinationFactory::generatePercent() {
	const auto liqPercentFormal = 100 / tmpData.leverage;
	const auto liqPercent = algorithmData::getLiqudationPercent(tmpData.leverage);
	for (auto activationPercent : iotaWithStep(minActivationPercent, liqPercentFormal + stepFloat, stepFloat)) {
		tmpData.activationPercent = activationPercent;
		if (auto stopLossFloor = std::max(activationPercent, minStopLossPercent); stopLossFloor < liqPercent) {
			for (auto stopLossPercent : iotaWithStep(stopLossFloor, liqPercent, stepFloat)) {
				tmpData.stopLossPercent = stopLossPercent;
				for (auto minimumProfitPercent : iotaWithStep(minMinProfitPercent, maxMinProfitPercent + stepFloat, stepFloat)) {
					tmpData.minimumProfitPercent = minimumProfitPercent;
					generateDynamicSL();
				}
			}
		}
		tmpData.stopLossPercent = -1.0;
		for (auto minimumProfitPercent : iotaWithStep(minMinProfitPercent, maxMinProfitPercent + stepFloat, stepFloat)) {
			tmpData.minimumProfitPercent = minimumProfitPercent;
			generateDynamicSL();
		}
	}
}

void combinationFactory::generateDynamicSL() {
	for (auto dynamicSLTrendMode : { true, false }) {
		tmpData.dynamicSLTrendMode = dynamicSLTrendMode;
		if (!dynamicSLTrendMode) {
			const auto liquidationPercent = 100 / tmpData.leverage;
			assert(maxDynamicSLPercent <= liquidationPercent);
			for (auto dynamicSLPercent : iotaWithStep(minDynamicSLPercent, maxDynamicSLPercent + stepFloat, stepFloat)) {
				tmpData.dynamicSLPercent = dynamicSLPercent;
				generateOpener();
			}
		}
		else {
			tmpData.dynamicSLPercent = -1.0;
			generateOpener();
		}
	}
}

void combinationFactory::generateOpener() {
	for (auto touchOpenerActivationWaitMode : { true, false }) {
		tmpData.touchOpenerActivationWaitMode = touchOpenerActivationWaitMode;
		for (auto breakOpenerEnabled : { true, false }) {
			tmpData.breakOpenerEnabled = breakOpenerEnabled;
			if (breakOpenerEnabled) {
				for (auto breakOpenerActivationWaitMode : { true, false }) {
					tmpData.breakOpenerActivationWaitMode = breakOpenerActivationWaitMode;
					for (auto alwaysUseNewTrend : { true, false }) {
						tmpData.alwaysUseNewTrend = alwaysUseNewTrend;
						generateActivation();
					}
				}
			}
			else {
				tmpData.breakOpenerActivationWaitMode = false;
				tmpData.alwaysUseNewTrend = false;
				generateActivation();
			}
		}
	}
}

void combinationFactory::generateActivation() {
	if (tmpData.breakOpenerActivationWaitMode || tmpData.touchOpenerActivationWaitMode) {
		for (auto activationWaiterResetAllowed : { true, false }) {
			tmpData.activationWaiterResetAllowed = activationWaiterResetAllowed;
			for (auto activationWaiterRange : iotaWithStep(minTrendActivationWaitRange, maxTrendActivationWaitRange + 1, 1)) {
				tmpData.activationWaiterRange = activationWaiterRange;
				for (auto activationWaiterFullCandleCheck : { true, false }) {
					tmpData.activationWaiterFullCandleCheck = activationWaiterFullCandleCheck;
					generateStop();
				}
			}
		}
	}
	else {
		tmpData.activationWaiterResetAllowed = false;
		tmpData.activationWaiterRange = -1;
		tmpData.activationWaiterFullCandleCheck = false;
		generateStop();
	}
}

void combinationFactory::generateStop() {
	for (auto stopLossWaiterEnabled : { true, false }) {
		tmpData.stopLossWaiterEnabled = stopLossWaiterEnabled;
		if (stopLossWaiterEnabled) {
			for (auto stopLossWaiterResetAllowed : { true, false }) {
				tmpData.stopLossWaiterResetAllowed = stopLossWaiterResetAllowed;
				for (auto stopLossWaiterRange : iotaWithStep(minStopLossWaitRange, maxStopLossWaitRange + 1, 1)) {
					tmpData.stopLossWaiterRange = stopLossWaiterRange;
					for (auto stopLossWaiterFullCandleCheck : { true, false }) {
						tmpData.stopLossWaiterFullCandleCheck = stopLossWaiterFullCandleCheck;
						onIterate();
					}
				}
			}
		}
		else {
			tmpData.stopLossWaiterResetAllowed = false;
			tmpData.stopLossWaiterRange = -1;
			tmpData.stopLossWaiterFullCandleCheck = false;
			onIterate();
		}
	}
}

void combinationFactory::onIterate() {
	++combinations;
	tmpAllData.push_back(tmpData);
}