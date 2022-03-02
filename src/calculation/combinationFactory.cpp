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
	std::vector<algorithmData> tmpAllData;
}

combinationFactory::combinationFactory(size_t aThreadsCount) : threadsCount(aThreadsCount) {
	reset();
	generateSuperTrend();
	data[0] = algorithmData{};

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
	inited = true;
}

size_t combinationFactory::getCombinationsAmount() const {
	return combinations;
}

size_t combinationFactory::getCurrentIndex() const {
	return std::accumulate(indexes.cbegin(), indexes.cend(), size_t{ 0 });
}

void combinationFactory::reset() {
	combinationsData = std::vector<std::vector<algorithmData>>(threadsCount);
	indexes = std::vector<size_t>(threadsCount, 0);
	data = std::vector<algorithmData>(threadsCount);
	callbacks = std::vector<iterateCallback>(threadsCount);
}

void combinationFactory::iterateCombination(int aPosition, iterateCallback aCallback) {
	callbacks[aPosition] = std::move(aCallback);
	generateSuperTrend(aPosition);
	callbacks[aPosition] = nullptr;
}

void combinationFactory::generateSuperTrend() {
	for (auto atrType : { indicators::eAtrType::RMA, indicators::eAtrType::EMA, indicators::eAtrType::WMA, indicators::eAtrType::SMA }) {
		data[0].atrType = atrType;
		for (auto atrSize : iotaWithStep(minAtrSize, maxAtrSize + stepInt, stepInt)) {
			data[0].atrSize = atrSize;
			for (auto stFactor : iotaWithStep(minStFactor, maxStFactor + stepFloat, stepFloat)) {
				data[0].stFactor = stFactor;
				generateDeal(0);
			}
		}
	}
}

void combinationFactory::generateSuperTrend(int aPosition) {
	data[aPosition].atrType = static_cast<indicators::eAtrType>((aPosition / 2) + 1); // 2 = threads / atrTypes
	auto atrSizes = iotaWithStep(minAtrSize, maxAtrSize + stepInt, stepInt);
	auto halfSize = atrSizes.size() / 2;
	if (aPosition % 2 == 0) {
		atrSizes.erase(std::next(atrSizes.begin(), halfSize), atrSizes.end());
	}
	else {
		atrSizes.erase(atrSizes.begin(), std::next(atrSizes.begin(), halfSize));
	}

	for (auto atrSize : atrSizes) {
		data[aPosition].atrSize = atrSize;
		for (auto stFactor : iotaWithStep(minStFactor, maxStFactor + stepFloat, stepFloat)) {
			data[aPosition].stFactor = stFactor;
			generateDeal(aPosition);
		}
	}
}

void combinationFactory::generateDeal(int aPosition) {
	for (auto dealPercent : iotaWithStep(minDealPercent, maxDealPercent + stepFloat, stepFloat)) {
		data[aPosition].dealPercent = dealPercent;
		for (auto leverage : iotaWithStep(minLeverage, maxLeverage + stepInt, stepInt)) {
			data[aPosition].leverage = leverage;
			generatePercent(aPosition);
		}
	}
}

void combinationFactory::generatePercent(int aPosition) {
	const auto liquidationPercent = 100 / data[aPosition].leverage;
	for (auto activationPercent : iotaWithStep(minActivationPercent, liquidationPercent + stepFloat, stepFloat)) {
		data[aPosition].activationPercent = activationPercent;
		for (auto stopLossPercent : iotaWithStep(std::max(activationPercent, minStopLossPercent), liquidationPercent + stepFloat, stepFloat)) {
			data[aPosition].stopLossPercent = stopLossPercent;
			for (auto minimumProfitPercent : iotaWithStep(minMinProfitPercent, maxMinProfitPercent + stepFloat, stepFloat)) {
				data[aPosition].minimumProfitPercent = minimumProfitPercent;
				generateDynamicSL(aPosition);
			}
		}
	}
}

void combinationFactory::generateDynamicSL(int aPosition) {
	for (auto dynamicSLTrendMode : { true, false }) {
		data[aPosition].dynamicSLTrendMode = dynamicSLTrendMode;
		if (!dynamicSLTrendMode) {
			const auto liquidationPercent = 100 / data[aPosition].leverage;
			assert(maxDynamicSLPercent <= liquidationPercent);
			for (auto dynamicSLPercent : iotaWithStep(minDynamicSLPercent, maxDynamicSLPercent + stepFloat, stepFloat)) {
				data[aPosition].dynamicSLPercent = dynamicSLPercent;
				generateOpener(aPosition);
			}
		}
		else {
			data[aPosition].dynamicSLPercent = -1.0;
			generateOpener(aPosition);
		}
	}
}

void combinationFactory::generateOpener(int aPosition) {
	for (auto touchOpenerActivationWaitMode : { true, false }) {
		data[aPosition].touchOpenerActivationWaitMode = touchOpenerActivationWaitMode;
		for (auto breakOpenerEnabled : { true, false }) {
			data[aPosition].breakOpenerEnabled = breakOpenerEnabled;
			if (breakOpenerEnabled) {
				for (auto breakOpenerActivationWaitMode : { true, false }) {
					data[aPosition].breakOpenerActivationWaitMode = breakOpenerActivationWaitMode;
					for (auto alwaysUseNewTrend : { true, false }) {
						data[aPosition].alwaysUseNewTrend = alwaysUseNewTrend;
						generateActivation(aPosition);
					}
				}
			}
			else {
				data[aPosition].breakOpenerActivationWaitMode = false;
				data[aPosition].alwaysUseNewTrend = false;
				generateActivation(aPosition);
			}
		}
	}
}

void combinationFactory::generateActivation(int aPosition) {
	if (data[aPosition].breakOpenerActivationWaitMode || data[aPosition].touchOpenerActivationWaitMode) {
		for (auto activationWaiterResetAllowed : { true, false }) {
			data[aPosition].activationWaiterResetAllowed = activationWaiterResetAllowed;
			for (auto activationWaiterRange : iotaWithStep(minTrendActivationWaitRange, maxTrendActivationWaitRange + 1, 1)) {
				data[aPosition].activationWaiterRange = activationWaiterRange;
				for (auto activationWaiterFullCandleCheck : { true, false }) {
					data[aPosition].activationWaiterFullCandleCheck = activationWaiterFullCandleCheck;
					generateStop(aPosition);
				}
			}
		}
	}
	else {
		data[aPosition].activationWaiterResetAllowed = false;
		data[aPosition].activationWaiterRange = -1;
		data[aPosition].activationWaiterFullCandleCheck = false;
		generateStop(aPosition);
	}
}

void combinationFactory::generateStop(int aPosition) {
	for (auto stopLossWaiterEnabled : { true, false }) {
		data[aPosition].stopLossWaiterEnabled = stopLossWaiterEnabled;
		if (stopLossWaiterEnabled) {
			for (auto stopLossWaiterResetAllowed : { true, false }) {
				data[aPosition].stopLossWaiterResetAllowed = stopLossWaiterResetAllowed;
				for (auto stopLossWaiterRange : iotaWithStep(minStopLossWaitRange, maxStopLossWaitRange + 1, 1)) {
					data[aPosition].stopLossWaiterRange = stopLossWaiterRange;
					for (auto stopLossWaiterFullCandleCheck : { true, false }) {
						data[aPosition].stopLossWaiterFullCandleCheck = stopLossWaiterFullCandleCheck;
						onIterate(aPosition);
					}
				}
			}
		}
		else {
			data[aPosition].stopLossWaiterResetAllowed = false;
			data[aPosition].stopLossWaiterRange = -1;
			data[aPosition].stopLossWaiterFullCandleCheck = false;
			onIterate(aPosition);
		}
	}
}

void combinationFactory::onIterate(int aPosition) {
	if (!inited) {
		++combinations;
		tmpAllData.push_back(data[0]);
		return;
	}
	++indexes[aPosition];
	if (callbacks[aPosition]) {
		callbacks[aPosition](data[aPosition], getCombinationsAmount());
	}
}