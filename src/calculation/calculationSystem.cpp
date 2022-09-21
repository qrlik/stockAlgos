#include "calculationSystem.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
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
	utils::log("\ncalculationSystem::saveFinalData finalVector size - [" + std::to_string(finalVector.size()) + ']');
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
		const auto maxProfit = getProfit(finalVector[0]);

		for (const auto& data : finalVector) {
			const auto profit = getProfit(data);
			if (utils::isLessOrEqual(profit, 0.0)) {
				break;
			}
			addStats(stats, data["data"], getWeight(profit, maxProfit, parabolaDegree));
		}

		addHeadlines(dataAll, stats, finalVector[0]);
		for (auto& data : finalVector) {
			if (utils::isLessOrEqual(getProfit(data), 0.0)) {
				break;
			}
			addData(dataAll, stats, data);
			jsonAllData.push_back(std::move(data));
		}
		dataAll.close();
		finalVector.clear();
	}
	utils::log("calculationSystem::saveFinalData profits size - [" + std::to_string(jsonAllData.size()) + "]");
	{
		utils::saveToJson(dirName + getAllDataFilename(), jsonAllData);
		utils::saveToJson(utils::lastDataDir, jsonAllData);
		jsonAllData.clear();
	}
	saveStats(stats, dirName + "stats.json");
}

combinationsJsons calculationSystem::balanceResultsByMaxLoss() {
	auto [combinationsCalculations, combinationsJsons] = getCalculationsConjunction(calculations);
	balanceByMaxLossPercent(algorithmType, combinationsCalculations, combinationsJsons, calculations);
	return std::move(combinationsJsons);
}

void calculationSystem::uniteResults(const combinationsCalculations& calculations, const combinationsJsons& jsons) {
	auto combinationsAverages = getCalculationsAverages(calculations);
	if (combinationsAverages.empty()) {
		utils::log("calculationSystem::uniteResults empty averages");
		return;
	}
	saveDataAndStats(combinationsAverages, jsons, parabolaDegree);
}