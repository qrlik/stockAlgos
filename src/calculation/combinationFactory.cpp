#include "combinationFactory.h"
#include "../market/marketRules.h"
#include "../tests/checkers.h"
#include <iostream>

using namespace calculation;

namespace {
	template<typename T>
	std::vector<T> iotaWithStep(T begin, T end, T step) {
		auto count = static_cast<int>(std::ceil((end - begin) / step));
		std::vector<T> result;
		result.reserve(count);
		for (auto i = 0; i < count; ++i) {
			result.push_back(begin);
			begin += step;
		}
		return result;
	}

	std::vector<double> getLiquidationRange(int aLeverage, double aMargin, int aSteps, double aMinOffset) {
		std::vector<double> result;
		auto liqPercent = market::marketData::getLiquidationPercent(aMargin, aLeverage);
		if (aSteps > 0) {
			auto stepSize = liqPercent / (aSteps + 1);
			for (auto i = 0; i < aSteps; ++i) {
				liqPercent -= stepSize;
				result.push_back(liqPercent);
			}
		}
		result.push_back(aMinOffset);
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

	for (auto atrType : { market::eAtrType::RMA, market::eAtrType::EMA, market::eAtrType::WMA, market::eAtrType::SMA }) {
		tmpData.atrType = atrType;
		for (auto atrSize : iotaWithStep(minAtrSize, maxAtrSize + atrSizeStep, atrSizeStep)) {
			tmpData.atrSize = atrSize;
			for (auto stFactor : iotaWithStep(minStFactor, maxStFactor + stFactorStep, stFactorStep)) {
				tmpData.stFactor = stFactor;
				generateDeal();
			}
		}
	}
}

void combinationFactory::generateDeal() {
	for (auto dealPercent : iotaWithStep(minDealPercent, maxDealPercent + dealPercentStep, dealPercentStep)) {
		tmpData.dealPercent = dealPercent;
		tmpData.orderSize = orderSize;
		tmpData.startCash = startCash;
		tmpData.maxLossCash = maxLossCash;
		tmpData.maxLossPercent = maxLossPercent;
		tmpData.leverage = leverage;
		generatePercent();
	}
}

void combinationFactory::generatePercent() {
	for (auto liquidationOffsetPercent : getLiquidationRange(tmpData.leverage, orderSize, offsetSteps, minLiquidationOffsetPercent)) {
		tmpData.liquidationOffsetPercent = liquidationOffsetPercent;
		for (auto minimumProfitPercent : iotaWithStep(minMinProfitPercent, maxMinProfitPercent + minProfitPercentStep, minProfitPercentStep)) {
			tmpData.minimumProfitPercent = minimumProfitPercent;
			generateDynamicSL();
		}
	}
}

void combinationFactory::generateDynamicSL() {
	for (auto dynamicSLTrendMode : dynamicSLTrendModeFlags) {
		tmpData.dynamicSLTrendMode = dynamicSLTrendMode;
		if (!dynamicSLTrendMode) {
			for (auto dynamicSLPercent : iotaWithStep(minDynamicSLPercent, maxDynamicSLPercent + dynamicSLPercentStep, dynamicSLPercentStep)) {
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
		for (auto breakOpenerEnabled : breakOpenerEnabledFlags) {
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
	for (auto stopLossWaiterEnabled : stopLossWaiterEnabledFlags) {
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
	tests::checkAlgorithmData(tmpData);
	tmpAllData.push_back(tmpData);
}