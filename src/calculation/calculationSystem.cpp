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
	const auto* allDataFileName = "jsonDataAll";

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

	double getProfit(const Json& aData) {
		return aData["cash"].get<double>() - aData["data"]["startCash"].get<double>();
	}

	std::string getDirName(const std::string& aTicker, market::eCandleInterval aInterval) {
		return utils::outputDir + '/' + aTicker + '_' + market::getCandleIntervalApiStr(aInterval) + '/';
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
			const auto weight = std::pow(getProfit(data) / maxProfit, parabolaDegree);
			if (utils::isLess(weight, weightPrecision)) {
				continue;
			}
			addStats(stats, data["data"], weight);
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
		utils::saveToJson(dirName + allDataFileName, jsonAllData);
		jsonAllData.clear();
	}
	saveStats(stats, dirName + "stats.json");
}

void calculationSystem::uniteResults() {
	std::unordered_map<size_t, std::vector<calculationInfo>> unitedInfo;
	std::unordered_map<size_t, Json> idToJsons;

	const auto size = calculations.size();
	for (const auto& [ticker, timeframe] : calculations) {
		const auto dirName = getDirName(ticker, timeframe);
		const auto allData = utils::readFromJson(dirName + allDataFileName);

		const auto maxProfit = getProfit(allData[0]);

		for (const auto& data : allData) {
			calculationInfo info;
			auto profit = getProfit(data);
			if (!utils::isGreater(profit, 0.0)) {
				continue;
			}
			info.weight = getProfit(data) / maxProfit;
			info.cash = data["cash"].get<double>();
			info.profitsFactor = data["stats"]["profitsFactor"].get<double>();
			info.recoveryFactor = data["stats"]["recoveryFactor"].get<double>();
			info.ordersPerInterval = data["stats"]["ordersPerInterval"].get<double>();
			unitedInfo[data["data"]["id"].get<size_t>()].push_back(info);
			idToJsons.try_emplace(data["data"]["id"].get<size_t>(), data["data"]);
		}
	}
	utils::log("<Disjunction> size - [ " + std::to_string(unitedInfo.size()) + " ] ");

	for (auto it = unitedInfo.begin(); it != unitedInfo.end();) {
		if (it->second.size() < size) {
			it = unitedInfo.erase(it);
		}
		else {
			++it;
		}
	}
	utils::log("<Conjunction> size - [ " + std::to_string(unitedInfo.size()) + " ] ");

	std::vector<std::pair<size_t, calculationInfo>> averageInfo;
	for (const auto& united : unitedInfo) {
		calculationInfo average;
		for (const auto& info : united.second) {
			average.weight += info.weight;
			average.cash += info.cash;
			average.profitsFactor += info.profitsFactor;
			average.recoveryFactor += info.recoveryFactor;
			average.ordersPerInterval += info.ordersPerInterval;
		}
		average.weight /= size;
		average.profitsFactor /= size;
		average.recoveryFactor /= size;
		average.ordersPerInterval /= size;
		averageInfo.push_back({ united.first, average });
	}

	std::sort(averageInfo.begin(), averageInfo.end(), [](const auto& aLhs, const auto& aRhs) { return aLhs.second.cash > aRhs.second.cash; });
	{
		Json unitedData;
		for (const auto& info : averageInfo) {
			Json data;
			data["cash"] = info.second.cash;
			data["data"] = idToJsons[info.first];
			auto& stats = data["stats"];
			stats["weight"] = info.second.weight;
			stats["profitsFactor"] = info.second.profitsFactor;
			stats["recoveryFactor"] = info.second.recoveryFactor;
			stats["ordersPerInterval"] = info.second.ordersPerInterval;
			unitedData.push_back(data);
		}

		std::ofstream unitedOutput(utils::outputDir + "/unitedData.txt");
		addHeadlines(unitedOutput, Json{}, unitedData[0]);
		for (const auto& data : unitedData) {
			addData(unitedOutput, Json{}, data);
		}
	}
}