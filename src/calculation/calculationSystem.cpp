#include "calculationSystem.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
#include "outputHelper.h"
#include "market/marketRules.h"
#include "utils/utils.h"
#include <execution>
#include <future>

using namespace calculation;

calculationSystem::calculationSystem() {
	loadSettings();
}

namespace {
	bool checkSettingsJson(const Json& aSettings) {
		bool result = true;
		result &= aSettings.is_object();
		result &= aSettings.contains("threadsAmount") && aSettings["threadsAmount"].is_number_unsigned();
		result &= aSettings.contains("parabolaDegree") && aSettings["parabolaDegree"].is_number_unsigned();
		result &= aSettings.contains("weightPrecision") && aSettings["weightPrecision"].is_number_float();
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

void calculationSystem::loadSettings() {
	const auto log = []() { utils::logError("calculationSystem::loadSettings parse error"); };
	auto settings = utils::readFromJson("calculationSettings");
	if (!checkSettingsJson(settings)) {
		log();
		return;
	}
	threadsAmount = settings["threadsAmount"].get<unsigned int>();
	weightPrecision = settings["weightPrecision"].get<double>();
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
	const auto newProgress = utils::round(static_cast<double>(aIndex) / combinations * 100, 1);
	if (utils::isGreater(newProgress, progress)) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		utils::log(std::to_string(progress) + '%');
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
	const auto dirName = utils::outputDir + '/' + aTicker + '_' + market::getCandleIntervalApiStr(aInterval) + '/';
	utils::createDir(dirName);
	Json jsonData;
	Json stats;
	{
		std::ofstream dataAll(dirName + "dataAll.txt");
		std::ofstream dataWeighted(dirName + "dataWeighted.txt");
		auto getProfit = [](const Json& aData) { return aData["cash"].get<double>() - aData["data"]["startCash"].get<double>(); };
		const auto maxProfit = getProfit(finalVector[0]);

		for (const auto& data : finalVector) {
			const auto weight = std::pow(getProfit(data) / maxProfit, parabolaDegree);
			if (utils::isLess(weight, weightPrecision)) {
				continue;
			}
			addStats(stats, data["data"], weight);
			jsonData.push_back(data["data"]);
		}

		addHeadlines(dataAll, stats, finalVector[0]);
		addHeadlines(dataWeighted, stats, finalVector[0]);
		for (const auto& data : finalVector) {
			addData(dataAll, stats, data);

			const auto weight = std::pow(getProfit(data) / maxProfit, parabolaDegree);
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
		std::ofstream jsonOutput(dirName + "jsonDataWeighted.json");
		jsonOutput << jsonData;
		jsonData.clear();
	}
	saveStats(stats, dirName + "stats.json");
}
