#include "combinationCalculator.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <execution>
#include <iostream>
#include <fstream>
#include <future>

using namespace calculation;

calculationSystem::calculationSystem(const std::string& aTicker, eCandleInterval aInterval):
	interval(aInterval),
	ticker(aTicker)
{
	auto json = utils::readFromJson("assets/candles/" + ticker + '/' + getCandleIntervalApiStr(interval));
	candlesSource = utils::parseCandles(json);
	threadsData = std::vector<threadInfo>(threadsAmount);
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
	auto factory = combinationFactory(threadsAmount);
	combinations = factory.getCombinationsAmount();
	for (size_t i = 0; i < threadsAmount; ++i) {
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
	for (const auto& data : threadData) {
		if (!threadInfo.isCached(data.atrType, data.atrSize, data.stFactor)) {
			candles = candlesSource;
			auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
			auto finalSize = static_cast<int>(candles.size()) - atrSizeDegree * aFactory.getMaxAtrSize();
			if (finalSize <= 0) {
				utils::logError("wrong atr size for candles amount");
				finalSize = static_cast<int>(candles.size());
			}
			indicators.getProcessedCandles(candles, finalSize);
			threadInfo.saveCache(data.atrType, data.atrSize, data.stFactor);
		}
		auto moneyMaker = algorithm::stAlgorithm(data);
		const auto result = moneyMaker.calculate(candles);
		if (result) {
			threadInfo.finalData.push_back(getData(moneyMaker));
		}
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

finalData calculationSystem::getData(const algorithm::stAlgorithm& aMM) {
	finalData result;
	result.cash = aMM.getFullCash();
	result.startCash = aMM.getStartCash();
	const auto& stats = aMM.stats;
	result.profitableOrder = stats.profitableOrder;
	result.profitableStreak = stats.profitableStreak;
	result.unprofitableOrder = stats.unprofitableOrder;
	result.unprofitableStreak = stats.unprofitableStreak;
	auto maxLoss = stats.maxLossHighCash - stats.maxLossLowCash;
	auto maxLossPercent = (aMM.orderSize > 0.0) ? maxLoss / aMM.startCash * 100 : maxLoss / stats.maxLossHighCash * 100;
	result.maxLossPercent = maxLossPercent;
	result.RF = (result.cash - aMM.startCash) / stats.summaryLoss;
	result.touchTrendOrder = stats.touchTrendOrder;
	result.breakTrendOrder = stats.breakTrendOrder;
	result.longOrder = stats.longOrder;
	result.shortOrder = stats.shortOrder;
	result.atrType = market::atrTypeToString(aMM.atrType);
	result.atrSize = aMM.atrSize;
	result.stFactor = aMM.stFactor;
	result.dealPercent = aMM.dealPercent;
	result.leverage = aMM.leverage;
	const auto liqPercent = (aMM.orderSize > 0.0)
		? market::marketData::getInstance()->getLiquidationPercent(aMM.orderSize, aMM.leverage)
		: market::marketData::getInstance()->getLeverageLiquidationRange(aMM.leverage).first;
	result.stopLossPercent = liqPercent - aMM.liquidationOffsetPercent;
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

namespace {
	void addHeadlines(std::ofstream& aOutput) {
		aOutput
			<< std::setw(24) << "Cash $"
			<< std::setw(10) << "PrOrd"
			<< std::setw(8) << "PrStr"
			<< std::setw(10) << "UnprOrd"
			<< std::setw(8) << "UnprStr"
			<< std::setw(24) << "MaxLoss %"
			<< std::setw(16) << "RF"
			<< std::setw(8) << "TochOrd"
			<< std::setw(8) << "BrekOrd"
			<< std::setw(8) << "LongOrd"
			<< std::setw(8) << "ShrtOrd"
			<< std::setw(6) << "ATR T"
			<< std::setw(6) << "ATR S"
			<< std::setw(12) << "STFactor"
			<< std::setw(12) << "Deal %"
			<< std::setw(7) << "Level"
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
		aOutput << std::fixed;
	}

	void addStats(Json& aStats, const finalData& aData, double aWeight) {
		const auto incrementCb = [aWeight](Json& aJson, const std::string& aName) {
			auto& counts = aJson["counts"][aName];
			if (counts.is_null()) {
				counts = 0;
			}
			counts = counts.get<int>() + 1;

			auto& weight = aJson["weight"][aName];
			if (weight.is_null()) {
				weight = 0.0;
			}
			weight = weight.get<double>() + aWeight;
		};
		incrementCb(aStats["atrType"], aData.atrType);
		incrementCb(aStats["stFactor"], std::to_string(aData.stFactor));
		incrementCb(aStats["dealPercent"], std::to_string(aData.dealPercent));
		incrementCb(aStats["stopLossPercent"], std::to_string(aData.stopLossPercent));
		incrementCb(aStats["minimumProfitPercent"], std::to_string(aData.minimumProfitPercent));
		incrementCb(aStats["dynamicSLPercent"], std::to_string(aData.dynamicSLPercent));
		incrementCb(aStats["atrSize"], std::to_string(aData.atrSize));
		incrementCb(aStats["leverage"], std::to_string(aData.leverage));
		incrementCb(aStats["activationWaiterModuleActivationWaitRange"], std::to_string(aData.activationWaiterModuleActivationWaitRange));
		incrementCb(aStats["stopLossWaiterModuleStopLossWaitRange"], std::to_string(aData.stopLossWaiterModuleStopLossWaitRange));
		incrementCb(aStats["dynamicStopLossTrendMode"], std::to_string(aData.dynamicStopLossTrendMode));
		incrementCb(aStats["trendTouchOpenerModuleActivationWaitMode"], std::to_string(aData.trendTouchOpenerModuleActivationWaitMode));
		incrementCb(aStats["trendBreakOpenerModuleEnabled"], std::to_string(aData.trendBreakOpenerModuleEnabled));
		incrementCb(aStats["trendBreakOpenerModuleActivationWaitMode"], std::to_string(aData.trendBreakOpenerModuleActivationWaitMode));
		incrementCb(aStats["trendBreakOpenerModuleAlwaysUseNewTrend"], std::to_string(aData.trendBreakOpenerModuleAlwaysUseNewTrend));
		incrementCb(aStats["activationWaiterModuleResetAllowed"], std::to_string(aData.activationWaiterModuleResetAllowed));
		incrementCb(aStats["activationWaiterModuleFullCandleCheck"], std::to_string(aData.activationWaiterModuleFullCandleCheck));
		incrementCb(aStats["stopLossWaiterModuleEnabled"], std::to_string(aData.stopLossWaiterModuleEnabled));
		incrementCb(aStats["stopLossWaiterModuleResetAllowed"], std::to_string(aData.stopLossWaiterModuleResetAllowed));
		incrementCb(aStats["stopLossWaiterModuleFullCandleCheck"], std::to_string(aData.stopLossWaiterModuleFullCandleCheck));
	}

	void addData(std::ofstream& aOutput, const finalData& aData) {
		aOutput
			<< std::setw(24) << aData.cash
			<< std::setw(10) << aData.profitableOrder
			<< std::setw(8) << aData.profitableStreak
			<< std::setw(10) << aData.unprofitableOrder
			<< std::setw(8) << aData.unprofitableStreak
			<< std::setw(24) << aData.maxLossPercent
			<< std::setw(16) << aData.RF
			<< std::setw(8) << aData.touchTrendOrder
			<< std::setw(8) << aData.breakTrendOrder
			<< std::setw(8) << aData.longOrder
			<< std::setw(8) << aData.shortOrder
			<< std::setw(6) << aData.atrType
			<< std::setw(6) << aData.atrSize
			<< std::setw(12) << aData.stFactor
			<< std::setw(12) << aData.dealPercent
			<< std::setw(7) << aData.leverage
			<< std::setw(12) << aData.stopLossPercent
			<< std::setw(12) << aData.minimumProfitPercent
			<< std::setw(12) << aData.dynamicSLPercent
			<< std::setw(10) << aData.dynamicStopLossTrendMode
			<< std::setw(10) << aData.trendTouchOpenerModuleActivationWaitMode
			<< std::setw(10) << aData.trendBreakOpenerModuleEnabled
			<< std::setw(10) << aData.trendBreakOpenerModuleActivationWaitMode
			<< std::setw(10) << aData.trendBreakOpenerModuleAlwaysUseNewTrend
			<< std::setw(10) << aData.activationWaiterModuleResetAllowed
			<< std::setw(10) << aData.activationWaiterModuleActivationWaitRange
			<< std::setw(10) << aData.activationWaiterModuleFullCandleCheck
			<< std::setw(10) << aData.stopLossWaiterModuleEnabled
			<< std::setw(10) << aData.stopLossWaiterModuleResetAllowed
			<< std::setw(10) << aData.stopLossWaiterModuleStopLossWaitRange
			<< std::setw(10) << aData.stopLossWaiterModuleFullCandleCheck << '\n';
	}

	Json getJson(const finalData& aData) {
		// TO DO make map with name + field offset + size of setw
		Json result;
		result["Cash $"] = aData.cash;
		result["PrOrd"] = aData.profitableOrder;
		result["PrStr"] = aData.profitableStreak;
		result["UnprOrd"] = aData.unprofitableOrder;
		result["UnprStr"] = aData.unprofitableStreak;
		result["MaxLoss %"] = aData.maxLossPercent;
		result["RF"] = aData.RF;
		result["TochOrd"] = aData.touchTrendOrder;
		result["BrekOrd"] = aData.breakTrendOrder;
		result["LongOrd"] = aData.longOrder;
		result["ShrtOrd"] = aData.shortOrder;
		result["ATR T"] = aData.atrType;
		result["ATR S"] = aData.atrSize;
		result["STFactor"] = aData.stFactor;
		result["Deal %"] = aData.dealPercent;
		result["Level"] = aData.leverage;
		result["SL %"] = aData.stopLossPercent;
		result["MinPrf %"] = aData.minimumProfitPercent;
		result["DSL %"] = aData.dynamicSLPercent;
		result["DSLTrend"] = aData.dynamicStopLossTrendMode;
		result["Touch WM"] = aData.trendTouchOpenerModuleActivationWaitMode;
		result["Break En"] = aData.trendBreakOpenerModuleEnabled;
		result["Break WM"] = aData.trendBreakOpenerModuleActivationWaitMode;
		result["UseNewTr"] = aData.trendBreakOpenerModuleAlwaysUseNewTrend;
		result["ActResAl"] = aData.activationWaiterModuleResetAllowed;
		result["ActRange"] = aData.activationWaiterModuleActivationWaitRange;
		result["ActFCChk"] = aData.activationWaiterModuleFullCandleCheck;
		result["SLWEnbld"] = aData.stopLossWaiterModuleEnabled;
		result["SLWResAl"] = aData.stopLossWaiterModuleResetAllowed;
		result["SLWRange"] = aData.stopLossWaiterModuleStopLossWaitRange;
		result["SLWFCChk"] = aData.stopLossWaiterModuleFullCandleCheck;
		return result;
	}
}

void calculationSystem::saveFinalData() {
	std::vector<finalData> finalVector;
	size_t size = 0;
	for (auto& threadData : threadsData) {
		size += threadData.finalData.size();
	}
	finalVector.reserve(size);
	for (auto& threadData : threadsData) {
		std::move(std::make_move_iterator(threadData.finalData.begin()), std::make_move_iterator(threadData.finalData.end()), std::back_inserter(finalVector));
		threadData.finalData.clear();
	}
	std::sort(std::execution::par_unseq, finalVector.begin(), finalVector.end(), [](const auto& lhs, const auto& rhs) { return lhs.cash > rhs.cash; });
	std::cout << "calculationSystem::saveFinalData finalVector size - [" + std::to_string(finalVector.size()) + "]\n";
	if (!finalVector.empty()) {
		std::cout << "calculationSystem::saveFinalData maxCash - [" + std::to_string(finalVector[0].cash) + "]\n";
	}

	Json jsonData;
	Json stats;
	{
		std::ofstream allData("positiveDataAll_" + ticker + '_' + getCandleIntervalApiStr(interval) + ".txt");
		std::ofstream positiveOutput("positiveData_" + ticker + '_' + getCandleIntervalApiStr(interval) + ".txt");
		addHeadlines(allData);
		addHeadlines(positiveOutput);
		auto maxProfit = -1.0;
		for (const auto& data : finalVector) {
			const auto profit = data.cash - data.startCash;
			if (profit <= 0.0) {
				break;
			}
			if (maxProfit < 0.0) {
				maxProfit = profit;
			}
			addData(allData, data);

			const auto weight = std::pow(profit / maxProfit, parabolaDegree);
			if (weight < weightPrecision) {
				continue;
			}
			addData(positiveOutput, data);
			addStats(stats, data, weight);
			jsonData.push_back(getJson(data));
		}
	}
	{
		std::ofstream jsonOutput("jsonData_" + ticker + '_' + getCandleIntervalApiStr(interval) + ".json");
		jsonOutput << jsonData;
		jsonData.clear();
	}
	{
		for (auto& var : stats) {
			auto sum = 0.0;
			for (auto& value : var["counts"]) {
				sum += value.get<int>();
			}
			for (auto& value : var["counts"]) {
				value = value.get<int>() / sum * 100.0;
			}

			sum = 0.0;
			for (auto& value : var["weight"]) {
				sum += value.get<double>();
			}
			for (auto& value : var["weight"]) {
				value = value.get<double>() / sum * 100.0;
			}
		}

		std::ofstream statsOutput("stats_" + ticker + '_' + getCandleIntervalApiStr(interval) + ".json");
		statsOutput << std::setw(4) << stats;
	}
}
