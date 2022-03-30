#include "combinationCalculator.h"
#include "../structs/algorithmData.h"
#include "../utils/utils.h"
#include <execution>
#include <iostream>
#include <fstream>
#include <future>

using namespace calculation;

calculationSystem::calculationSystem(eCandleInterval aInterval) {
	auto json = utils::readFromJson("assets/candles/3year/" + getCandleIntervalApiStr(aInterval));
	candlesSource = utils::parseCandles(json);
	threadsData = std::vector<threadInfo>(threadsCount);
}

bool calculationSystem::threadInfo::isCached(market::eAtrType aType, int aSize, double aFactor) {
	return aType == cachedAtrType && aSize == cachedAtrSize && aFactor == cachedStFactor;
}

void calculationSystem::threadInfo::saveCache(market::eAtrType aType, int aSize, double aFactor) {
	cachedAtrType = aType;
	cachedAtrSize = aSize;
	cachedStFactor = aFactor;
}

void calculationSystem::calculate() {
	std::vector<std::future<void>> futures;
	auto factory = combinationFactory(threadsCount);
	combinations = factory.getCombinationsAmount();
	for (auto i = 0; i < threadsCount; ++i) {
		futures.push_back(std::async(std::launch::async, [this, &factory, i]() { return iterate(factory, i); }));
	}
	for (auto& future : futures) {
		future.wait();
	}
	factory.onFinish();
	saveFinalData();
}

void calculationSystem::iterate(combinationFactory& aFactory, int aThread) {
	std::vector<candle> candles;
	auto& threadInfo = threadsData[aThread];
	const auto& threadData = aFactory.getThreadData(aThread);
	threadInfo.finalData.reserve(threadData.size());
	for (const auto& data : threadData) {
		if (!threadInfo.isCached(data.atrType, data.atrSize, data.stFactor)) {
			candles = candlesSource;
			auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
			indicators.getProcessedCandles(candles, static_cast<int>(candlesSource.size()) - 1000);
			threadInfo.saveCache(data.atrType, data.atrSize, data.stFactor);
		}
		auto moneyMaker = algorithm::moneyMaker(data);
		moneyMaker.calculate(candles);
		threadInfo.finalData.push_back(getData(moneyMaker));
		aFactory.incrementThreadIndex(aThread);
		printProgress(aFactory.getCurrentIndex());
	}
}

void calculationSystem::printProgress(size_t aIndex) {
	const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations * 100, 1);
	if (newProgress > progress) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		std::cout << std::to_string(progress) + "%\n";
	}
}

calculationSystem::finalData calculationSystem::getData(const algorithm::moneyMaker& aMM) {
	finalData result;
	result.cash = aMM.getFullCash();
	const auto& stats = aMM.stats;
	result.profitableOrder = stats.profitableOrder;
	result.profitableStreak = stats.profitableStreak;
	result.unprofitableOrder = stats.unprofitableOrder;
	result.unprofitableStreak = stats.unprofitableStreak;
	auto maxLoss = stats.maxLossHighCash - stats.maxLossLowCash;
	auto maxLossPercent = maxLoss / stats.maxLossHighCash * 100;
	result.maxLossPercent = maxLossPercent;
	result.summaryLoss = stats.summaryLoss;
	if (result.cash > aMM.startCash) {
		auto profit = result.cash - aMM.startCash;
		auto profitPercent = profit / aMM.startCash * 100;
		result.RFCommon = profitPercent / maxLossPercent;
		result.RFSummary = profit / stats.summaryLoss;
	}
	else {
		result.RFCommon = 0.0;
		result.RFSummary = 0.0;
	}
	result.touchTrendOrder = stats.touchTrendOrder;
	result.breakTrendOrder = stats.breakTrendOrder;
	result.longOrder = stats.longOrder;
	result.shortOrder = stats.shortOrder;
	result.atrType = market::atrTypeToString(aMM.atrType);
	result.atrSize = aMM.atrSize;
	result.stFactor = aMM.stFactor;
	result.dealPercent = aMM.dealPercent;
	result.leverage = aMM.leverage;
	result.liquidationOffsetPercent = aMM.liquidationOffsetPercent;
	result.minimumProfitPercent = aMM.minimumProfitPercent;
	result.dynamicSLPercent = aMM.dynamicStopLossModule.dynamicSLPercent;
	result.dynamicStopLossTrendMode = aMM.dynamicStopLossModule.trendMode;
	result.trendTouchOpenerModuleActivationWaitMode = aMM.trendTouchOpenerModule.activationWaitMode;
	result.trendBreakOpenerModuleEnabled = aMM.trendBreakOpenerModule.enabled;
	result.trendBreakOpenerModuleActivationWaitMode = aMM.trendBreakOpenerModule.activationWaitMode;
	result.trendBreakOpenerModuleAlwaysUseNewTrend = aMM.trendBreakOpenerModule.alwaysUseNewTrend;
	result.activationWaiterModuleResetAllowed = aMM.activationWaiterModule.resetAllowed;
	result.activationWaiterModuleActivationWaitRange = aMM.activationWaiterModule.activationWaitRange;
	result.activationWaiterModuleFullCandleCheck = aMM.activationWaiterModule.fullCandleCheck;
	result.stopLossWaiterModuleEnabled = aMM.stopLossWaiterModule.enabled;
	result.stopLossWaiterModuleResetAllowed = aMM.stopLossWaiterModule.resetAllowed;
	result.stopLossWaiterModuleStopLossWaitRange = aMM.stopLossWaiterModule.stopLossWaitRange;
	result.stopLossWaiterModuleFullCandleCheck = aMM.stopLossWaiterModule.fullCandleCheck;

	return result;
}

void calculationSystem::saveFinalData() {
	std::vector<calculationSystem::finalData> finalVector;
	finalVector.reserve(combinations);
	for (auto& threadData : threadsData) {
		std::move(std::make_move_iterator(threadData.finalData.begin()), std::make_move_iterator(threadData.finalData.end()), std::back_inserter(finalVector));
	}
	std::sort(std::execution::par_unseq, finalVector.begin(), finalVector.end(), [](const auto& lhs, const auto& rhs) { return lhs.cash > rhs.cash; });

	std::ofstream output("finalData.txt");
	output
		<< std::setw(18) << "Cash $"
		<< std::setw(8) << "PrOrd"
		<< std::setw(8) << "PrStr"
		<< std::setw(8) << "UnprOrd"
		<< std::setw(8) << "UnprStr"
		<< std::setw(21) << "MaxLoss %"
		<< std::setw(21) << "SumLoss $"
		<< std::setw(12) << "RFCommon"
		<< std::setw(12) << "RFSummary"
		<< std::setw(8) << "TochOrd"
		<< std::setw(8) << "BrekOrd"
		<< std::setw(8) << "LongOrd"
		<< std::setw(8) << "ShrtOrd"
		<< std::setw(6) << "ATR T"
		<< std::setw(6) << "ATR S"
		<< std::setw(12) << "STFactor"
		<< std::setw(12) << "Deal %"
		<< std::setw(7) << "Level"
		<< std::setw(12) << "Act %"
		<< std::setw(12) << "SL %"
		<< std::setw(12) << "MinPrf %"
		<< std::setw(12) << "DSL %"
		<< std::setw(10) << "DSLTrend"
		<< std::setw(10) << "Touch WM"
		<< std::setw(10) << "Break En"
		<< std::setw(10) << "Break WM"
		<< std::setw(10) << "UseNewTr"
		<< std::setw(10) << "ActResAl"
		<< std::setw(10) << "ActRange"
		<< std::setw(10) << "ActFCChk"
		<< std::setw(10) << "SLWEnbld"
		<< std::setw(10) << "SLWResAl"
		<< std::setw(10) << "SLWRange"
		<< std::setw(10) << "SLWFCChk\n";
	output << std::fixed;

	for (const auto& data : finalVector) {
		output
			<< std::setw(18) << data.cash
			<< std::setw(8) << data.profitableOrder
			<< std::setw(8) << data.profitableStreak
			<< std::setw(8) << data.unprofitableOrder
			<< std::setw(8) << data.unprofitableStreak
			<< std::setw(21) << data.maxLossPercent
			<< std::setw(21) << data.summaryLoss
			<< std::setw(12) << data.RFCommon
			<< std::setw(12) << data.RFSummary
			<< std::setw(8) << data.touchTrendOrder
			<< std::setw(8) << data.breakTrendOrder
			<< std::setw(8) << data.longOrder
			<< std::setw(8) << data.shortOrder
			<< std::setw(6) << data.atrType
			<< std::setw(6) << data.atrSize
			<< std::setw(12) << data.stFactor
			<< std::setw(12) << data.dealPercent
			<< std::setw(7) << data.leverage
			<< std::setw(12) << data.liquidationOffsetPercent
			<< std::setw(12) << data.minimumProfitPercent
			<< std::setw(12) << data.dynamicSLPercent
			<< std::setw(10) << data.dynamicStopLossTrendMode
			<< std::setw(10) << data.trendTouchOpenerModuleActivationWaitMode
			<< std::setw(10) << data.trendBreakOpenerModuleEnabled
			<< std::setw(10) << data.trendBreakOpenerModuleActivationWaitMode
			<< std::setw(10) << data.trendBreakOpenerModuleAlwaysUseNewTrend
			<< std::setw(10) << data.activationWaiterModuleResetAllowed
			<< std::setw(10) << data.activationWaiterModuleActivationWaitRange
			<< std::setw(10) << data.activationWaiterModuleFullCandleCheck
			<< std::setw(10) << data.stopLossWaiterModuleEnabled
			<< std::setw(10) << data.stopLossWaiterModuleResetAllowed
			<< std::setw(10) << data.stopLossWaiterModuleStopLossWaitRange
			<< std::setw(10) << data.stopLossWaiterModuleFullCandleCheck << '\n';
	}
}
