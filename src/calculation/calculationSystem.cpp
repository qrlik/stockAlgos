#include "calculationSystem.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
#include "outputHelper.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <execution>
#include <future>
#include <iostream>

using namespace calculation;

namespace {
	bool checkSettingsJson(const Json& aSettings) {
		bool result = true;
		result &= aSettings.is_object();
		result &= aSettings.contains("threadsAmount") && aSettings["threadsAmount"].is_number_unsigned();
		result &= aSettings.contains("parabolaDegree") && aSettings["parabolaDegree"].is_number_unsigned();
		result &= aSettings.contains("weightPrecision") && aSettings["weightPrecision"].is_number_float();
		result &= aSettings.contains("maxLossToProfitFactor") && aSettings["maxLossToProfitFactor"].is_number_float();
		result &= aSettings.contains("algorithmType") && aSettings["algorithmType"].is_string();
		result &= aSettings.contains("calculations") && aSettings["calculations"].is_array();
		if (result) {
			for (const auto& calculation : aSettings["calculations"]) {
				result &= calculation.is_object();
				result &= calculation.contains("ticker") && calculation["ticker"].is_string();
				result &= calculation.contains("timeframe") && calculation["timeframe"].is_string();
			}
		}
		return result;
	}
}

calculationSystem::calculationSystem() {
	loadSettings();
}

void calculationSystem::loadSettings() {
	const auto log = []() { utils::logError("calculationSystem::loadSettings parse error"); };
	auto settings = utils::readFromJson("input/calculationSettings");
	if (!checkSettingsJson(settings)) {
		log();
		return;
	}
	threadsAmount = settings["threadsAmount"].get<unsigned int>();
	weightPrecision = settings["weightPrecision"].get<double>();
	maxLossToProfitFactor = settings["maxLossToProfitFactor"].get<double>();
	parabolaDegree = settings["parabolaDegree"].get<unsigned int>();
	algorithmType = settings["algorithmType"].get<std::string>();
	for (const auto& calculation : settings["calculations"]) {
		auto ticker = calculation["ticker"].get<std::string>();
		auto timeframe = market::getCandleIntervalFromStr(calculation["timeframe"].get<std::string>());
		if (ticker.empty() || timeframe == market::eCandleInterval::NONE) {
			log();
		}
		else {
			calculations.emplace_back(std::move(ticker), timeframe);
		}
	}
}

void calculationSystem::calculate() {
	if (algorithmType == "superTrend") {
		calculateInternal<algorithm::stAlgorithm>();
	}
	else if (algorithmType == "superTrendMA") {
		calculateInternal<algorithm::stMAlgorithm>();
	}
	else {
		utils::logError("calculationSystem::calculate unknown algorithm type");
	}
}

void calculationSystem::printProgress(size_t aIndex) {
	const auto newProgress = static_cast<int>(static_cast<double>(aIndex) / combinations * 100);
	if (newProgress > progress) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		std::cout << std::to_string(progress) + "%...";
		std::cout.flush();
	}
}

void calculationSystem::saveFinalData(const std::string& aTicker, market::eCandleInterval aInterval) {
	std::vector<Json> finalVector;
	size_t size = 0;
	for (auto& threadData : threadsData) {
		size += threadData.size();
	}
	finalVector.reserve(size);
	for (auto& threadData : threadsData) {
		std::move(std::make_move_iterator(threadData.begin()), std::make_move_iterator(threadData.end()), std::back_inserter(finalVector));
	}
	threadsData.clear();
	std::sort(std::execution::par_unseq, finalVector.begin(), finalVector.end(), [](const Json& aLhs, const Json& aRhs) {
		return utils::isGreater(aLhs["cash"].get<double>(), aRhs["cash"].get<double>());
	});
	utils::log("calculationSystem::saveFinalData finalVector size - [" + std::to_string(finalVector.size()) + ']');
	if (finalVector.empty()) {
		return;
	}
	utils::log("calculationSystem::saveFinalData maxCash - [" + std::to_string(finalVector[0]["cash"].get<double>()) + ']');
	const auto dirName = getDirName(aTicker, aInterval);
	utils::createDir(dirName);
	Json jsonAllData;
	Json stats;
	{
		std::ofstream dataAll(dirName + "dataAll.txt");
		std::ofstream dataWeighted(dirName + "dataWeighted.txt");
		const auto maxProfit = getProfit(finalVector[0]);

		for (const auto& data : finalVector) {
			jsonAllData.push_back(data);
			const auto weight = getWeight(getProfit(data), maxProfit, parabolaDegree);
			if (utils::isLess(weight, weightPrecision)) {
				continue;
			}
			addStats(stats, data["data"], weight);
		}

		addHeadlines(dataAll, stats, finalVector[0]);
		addHeadlines(dataWeighted, stats, finalVector[0]);
		for (const auto& data : finalVector) {
			addData(dataAll, stats, data);

			const auto weight = getWeight(getProfit(data), maxProfit, parabolaDegree);
			if (utils::isLess(weight, weightPrecision)) {
				continue;
			}
			addData(dataWeighted, stats, data);
		}
		dataAll.close();
		dataWeighted.close();
		finalVector.clear();
	}
	{
		utils::saveToJson(dirName + getAllDataFilename(), jsonAllData);
		jsonAllData.clear();
	}
	saveStats(stats, dirName + "stats.json");
}

void calculationSystem::uniteResults() {
	auto [combinationsCalculations, combinationsJsons] = getCalculationsConjunction(calculations, 1);
	auto combinationsAverages = getCalculationsAverages(combinationsCalculations, calculations.size());
	{
		Json unitedData;
		Json unitedStats;
		double maxProfit = (!combinationsAverages.empty()) ? combinationsAverages[0].second.cash : 0.0;
		for (const auto& info : combinationsAverages) {
			Json data;
			data["cash"] = info.second.cash;
			data["data"] = combinationsJsons[info.first];
			auto& stats = data["stats"];
			stats["weight"] = info.second.weight;
			stats["profitsFactor"] = info.second.profitsFactor;
			stats["recoveryFactor"] = info.second.recoveryFactor;
			stats["ordersPerInterval"] = info.second.ordersPerInterval;
			stats["profitPerInterval"] = info.second.profitPerInterval;
			unitedData.push_back(data);

			const auto weight = getWeight(getProfit(data), maxProfit, parabolaDegree);
			addStats(unitedStats, data["data"], weight);
		}

		std::ofstream unitedOutput(utils::outputDir + "/unitedData.txt");
		addHeadlines(unitedOutput, unitedStats, unitedData[0]);
		for (const auto& data : unitedData) {
			addData(unitedOutput, unitedStats, data);
		}
		saveStats(unitedStats, utils::outputDir + "/stats.json");
	}
}