#include "combinationCalculator.h"
#include "../structs/algorithmData.h"
#include "../utils/utils.h"
#include <iostream>
#include <future>
#include <OpenXLSX/OpenXLSX.hpp>

using namespace calculation;
using namespace OpenXLSX;

calculationSystem::calculationSystem() {
	auto json = utils::readFromJson("assets/candles/1h_year");
	candlesSource = utils::parseCandles(json);
	threadsData = std::vector<threadInfo>(threadCount);
}

bool calculationSystem::threadInfo::isCached(indicators::eAtrType aType, int aSize, double aFactor) {
	return aType == cachedAtrType && aSize == cachedAtrSize && aFactor == cachedStFactor;
}

void calculationSystem::threadInfo::saveCache(indicators::eAtrType aType, int aSize, double aFactor) {
	cachedAtrType = aType;
	cachedAtrSize = aSize;
	cachedStFactor = aFactor;
}

void calculationSystem::calculate() {
	std::vector<std::future<void>> futures;
	auto factory = combinationFactory();
	combinations = factory.getCombinationsAmount();
	factory.reset();
	for (auto i = 0; i < threadCount; ++i) {
		futures.push_back(std::async(std::launch::async, [this, &factory, i]() {return iterate(factory, i); }));
	}
	for (auto& future : futures) {
		future.wait();
	}
	saveFinalData();
}

void calculationSystem::iterate(combinationFactory& aFactory, int aThread) {
	std::vector<candle> candles;
	aFactory.iterateCombination(aThread, [this, &candles, aThread](const algorithmData& aData, size_t aIndex) {
		auto& threadInfo = threadsData[aThread];
		if (!threadInfo.isCached(aData.atrType, aData.atrSize, aData.stFactor)) {
			candles = candlesSource;
			indicators::getProcessedCandles(candles, aData.atrType, aData.atrSize, aData.stFactor, 8760);
			threadInfo.saveCache(aData.atrType, aData.atrSize, aData.stFactor);
		}
		auto moneyMaker = algorithm::moneyMaker(aData, 100.0);
		moneyMaker.calculate(candles);
		threadInfo.finalData.push_back(getData(moneyMaker));
		printProgress(aIndex);
	});
}

void calculationSystem::printProgress(size_t aIndex) {
	const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations, 2) * 100;
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
	result.atrType = indicators::atrTypeToString(aMM.atrType);
	result.atrSize = aMM.atrSize;
	result.stFactor = aMM.stFactor;
	result.dealPercent = aMM.dealPercent;
	result.leverage = aMM.leverage;
	result.activationPercent = aMM.activationPercent;
	result.stopLossPercent = aMM.stopLossPercent;
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
	XLDocument doc;
	doc.create("finalData.xlsx");
	auto wks = doc.workbook().worksheet("Sheet1");
	wks.cell(1, 1).value() = "Cash $";
	wks.cell(1, 2).value() = "Profit Orders";
	wks.cell(1, 3).value() = "Profit Streak";
	wks.cell(1, 4).value() = "Unprofit Orders";
	wks.cell(1, 5).value() = "Unprofit Streak";
	wks.cell(1, 6).value() = "Max Loss %";
	wks.cell(1, 7).value() = "Summary Loss $";
	wks.cell(1, 8).value() = "RF Common";
	wks.cell(1, 9).value() = "RF Summary";
	wks.cell(1, 10).value() = "Touch Orders";
	wks.cell(1, 11).value() = "Break Orders";
	wks.cell(1, 12).value() = "ATR Type";
	wks.cell(1, 13).value() = "ATR Size";
	wks.cell(1, 14).value() = "ST Factor";
	wks.cell(1, 15).value() = "Deal %";
	wks.cell(1, 16).value() = "Leverage";
	wks.cell(1, 17).value() = "Activation %";
	wks.cell(1, 18).value() = "Stop Loss %";
	wks.cell(1, 19).value() = "Min. Profit %";
	wks.cell(1, 20).value() = "Dyn.Stop Loss %";
	wks.cell(1, 21).value() = "Dyn.Stop Loss Trend";
	wks.cell(1, 22).value() = "Touch Wait Mode";
	wks.cell(1, 23).value() = "Break Enabled";
	wks.cell(1, 24).value() = "Break Wait Mode";
	wks.cell(1, 25).value() = "AlwaysUseNewTrend";
	wks.cell(1, 26).value() = "Activation ResetAllowed";
	wks.cell(1, 27).value() = "Activation Range";
	wks.cell(1, 28).value() = "Activation FullCandleCheck";
	wks.cell(1, 29).value() = "SL Waiter Enabled";
	wks.cell(1, 30).value() = "SL Waiter ResetAllowed";
	wks.cell(1, 31).value() = "SL Waiter Range";
	wks.cell(1, 32).value() = "SL Waiter FullCandleCheck";

	size_t row = 2;
	for (const auto& threadData : threadsData) {
		for (const auto& data : threadData.finalData) {
			wks.cell(row, 1).value() = data.cash;
			wks.cell(row, 2).value() = data.profitableOrder;
			wks.cell(row, 3).value() = data.profitableStreak;
			wks.cell(row, 4).value() = data.unprofitableOrder;
			wks.cell(row, 5).value() = data.unprofitableStreak;
			wks.cell(row, 6).value() = data.maxLossPercent;
			wks.cell(row, 7).value() = data.summaryLoss;
			wks.cell(row, 8).value() = data.RFCommon;
			wks.cell(row, 9).value() = data.RFSummary;
			wks.cell(row, 10).value() = data.touchTrendOrder;
			wks.cell(row, 11).value() = data.breakTrendOrder;
			wks.cell(row, 12).value() = data.atrType;
			wks.cell(row, 13).value() = data.atrSize;
			wks.cell(row, 14).value() = data.stFactor;
			wks.cell(row, 15).value() = data.dealPercent;
			wks.cell(row, 16).value() = data.leverage;
			wks.cell(row, 17).value() = data.activationPercent;
			wks.cell(row, 18).value() = data.stopLossPercent;
			wks.cell(row, 19).value() = data.minimumProfitPercent;
			wks.cell(row, 20).value() = data.dynamicSLPercent;
			wks.cell(row, 21).value() = data.dynamicStopLossTrendMode;
			wks.cell(row, 22).value() = data.trendTouchOpenerModuleActivationWaitMode;
			wks.cell(row, 23).value() = data.trendBreakOpenerModuleEnabled;
			wks.cell(row, 24).value() = data.trendBreakOpenerModuleActivationWaitMode;
			wks.cell(row, 25).value() = data.trendBreakOpenerModuleAlwaysUseNewTrend;
			wks.cell(row, 26).value() = data.activationWaiterModuleResetAllowed;
			wks.cell(row, 27).value() = data.activationWaiterModuleActivationWaitRange;
			wks.cell(row, 28).value() = data.activationWaiterModuleFullCandleCheck;
			wks.cell(row, 29).value() = data.stopLossWaiterModuleEnabled;
			wks.cell(row, 30).value() = data.stopLossWaiterModuleResetAllowed;
			wks.cell(row, 31).value() = data.stopLossWaiterModuleStopLossWaitRange;
			wks.cell(row, 32).value() = data.stopLossWaiterModuleFullCandleCheck;
			++row;
		}
	}
	doc.save();
}
