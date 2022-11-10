#include "outputHelper.h"
#include "algorithm/order.h"
#include "maxLossBalancer.h"
#include "utils/utils.h"
#include <future>

using namespace calculation;

namespace {
	const auto* allDataFileName = "jsonDataAll";
	const auto doubleWidth = 16;
	const auto intWidth = 12;
	const auto boolWidth = 8;
	const auto spaceWidth = 4;
	const auto floatPrecision = 3;

	int getWidth(const std::string& aName, const Json& aValue) {
		auto width = static_cast<int>(aName.length()) + spaceWidth;
		if (aValue.is_number_float()) {
			width = std::max(width, doubleWidth);
		}
		else if (aValue.is_number_unsigned()) {
			width = std::max(width, 2 * intWidth);
		}
		else if (aValue.is_number_integer()) {
			width = std::max(width, intWidth);
		}
		else {
			width = std::max(width, boolWidth);
		}
		return width;
	}

	std::string getStrFromJson(const Json& aValue) {
		std::ostringstream os;
		if (aValue.is_number_float()) {
			os << std::fixed << std::setprecision(floatPrecision) << aValue.get<double>();
		}
		else {
			os << aValue;
		}
		return os.str();
	}

	double getMedian(const std::vector<double>& aValues) {
		auto firstIndex = 0;
		auto secondIndex = 0;
		if (aValues.size() % 2 == 0) {
			firstIndex = static_cast<int>(aValues.size()) / 2 - 1;
			secondIndex = firstIndex + 1;
		}
		else {
			firstIndex = static_cast<int>(aValues.size() + 1) / 2 - 1;
			secondIndex = firstIndex;
		}
		return (aValues[firstIndex] + aValues[secondIndex]) / 2;
	}
}

void calculation::addStats(Json& aStats, const Json& aData, double aWeight) {
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
	for (const auto& [name, value] : aData.items()) {
		if (name == "id" || name == "dealPercent") {
			continue;
		}
		incrementCb(aStats[name], getStrFromJson(value));
	}
	incrementCb(aStats["minLiquidationPercent"], getStrFromJson(Json(algorithm::getMinLiquidationPercent())));
}

void calculation::saveStats(Json& aStats, const std::string& aFileName) {
	Json singles;
	for (auto& [key, var] : aStats.items()) {
		if (var["counts"].size() == 1) {
			auto value = var["counts"].items().begin().key();
			singles[key] = value;
			continue;
		}

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
	for (auto& [key, var] : singles.items()) {
		aStats.erase(key);
	}
	aStats["singles"] = std::move(singles);

	std::ofstream statsOutput(aFileName);
	statsOutput << std::setw(spaceWidth) << aStats;
}

void calculation::addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample) {
	aOutput << std::fixed << std::setw(doubleWidth) << "Cash";
	for (const auto& [name, value] : aExample["stats"].items()) {
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	for (const auto& [name, value] : aExample["data"].items()) {
		if (aStats.is_object() && aStats.contains(name) && aStats[name]["counts"].size() < 2) {
			continue;
		}
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	aOutput << '\n';
}

void calculation::addData(std::ofstream& aOutput, const Json& aStats, const Json& aData) {
	aOutput << std::setw(doubleWidth) << getStrFromJson(aData["cash"]);
	for (const auto& [name, value] : aData["stats"].items()) {
		aOutput << std::setw(getWidth(name, value)) << getStrFromJson(value);
	}
	for (const auto& [name, value] : aData["data"].items()) {
		if (aStats.is_object() && aStats.contains(name) && aStats[name]["counts"].size() < 2) {
			continue;
		}
		aOutput << std::setw(getWidth(name, value)) << getStrFromJson(value);
	}
	aOutput << '\n';
}

double calculation::getProfit(const Json& aData) {
	if (aData.contains("cash") && aData.contains("data") && aData["data"].contains("startCash")) {
		return aData["cash"].get<double>() - aData["data"]["startCash"].get<double>();
	}
	utils::logError("calculation::getProfit");
	return -1;
}

double calculation::getWeight(double aProfit, double aMaxProfit, double aDegree) {
	return std::pow(aProfit / aMaxProfit, aDegree);
}

std::string calculation::getAllDataFilename() {
	return allDataFileName;
}

std::string calculation::getDirName(const std::string& aTicker, market::eCandleInterval aInterval) {
	return utils::outputDir + '/' + aTicker + '_' + market::getCandleIntervalApiStr(aInterval) + '/';
}

calculationInfo calculation::getCalculationInfo(const std::string& ticker, const Json& data) {
	calculationInfo info;
	info.ticker = ticker;
	info.cash = data["cash"].get<double>();
	info.profitsFactor = data["stats"]["profitsFactor"].get<double>();
	info.recoveryFactor = data["stats"]["recoveryFactor"].get<double>();
	info.ordersPerInterval = data["stats"]["ordersPerInterval"].get<double>();
	info.maxLossPercent = data["stats"]["maxLossPercent"].get<double>();
	info.profitPerInterval = data["stats"]["profitPerInterval"].get<double>();
	return info;
}

std::pair<combinationsCalculations, combinationsJsons> calculation::getCalculationsConjunction(const calculationsType& calculations) {
	auto lastData = utils::readFromJson(utils::lastDataDir);
	combinationsCalculations unitedInfo(lastData.size());
	combinationsJsons idToJsons(lastData.size());

	for (auto& data : lastData) {
		if (!utils::isGreater(getProfit(data), 0.0)) {
			utils::logError("calculation::getCalculationsConjunction wrong data in lastData");
		}
		const auto id = data["data"]["id"].get<size_t>();
		unitedInfo[id].reserve(calculations.size());
		idToJsons[id] = std::move(data["data"]);
	}

	for (const auto& [ticker, timeframe] : calculations) {
		const auto dirName = getDirName(ticker, timeframe);
		const auto allData = utils::readFromJson(dirName + allDataFileName);

		for (const auto& data : allData) {
			if (utils::isLessOrEqual(getProfit(data), 0.0)) {
				break;
			}
			if (!idToJsons.count(data["data"]["id"].get<size_t>())) {
				continue;
			}
			unitedInfo[data["data"]["id"].get<size_t>()].push_back(getCalculationInfo(ticker, data));
		}
	}

	for (const auto& info : unitedInfo) {
		if (info.second.size() < calculations.size()) {
			utils::logError("calculation::getCalculationsConjunction wrong united info size");
		}
	}
	if (lastData.size() != unitedInfo.size()) {
		utils::logError("calculation::getCalculationsConjunction wrong united data size");
	}
	utils::log("<Conjunction> size - [ " + std::to_string(unitedInfo.size()) + " ] ");

	return { std::move(unitedInfo), std::move(idToJsons) };
}

namespace {
	void balanceByMaxLossPercentAsync(const std::string& algoType, const std::string& ticker, market::eCandleInterval interval,
									const std::vector<size_t>& ids, const combinationsJsons& jsons, const combinationsCalculations& combinations,
									std::unordered_map<size_t, double>& minDealPercents) {
		for (auto id : ids) {
			if (utils::isLessOrEqual(minDealPercents[id], 0.0)) {
				continue;
			}
			auto infosIt = combinations.find(id);
			auto jsonIt = jsons.find(id);
			if (infosIt == combinations.end() || jsonIt == jsons.end()) {
				utils::logError("calculation::balanceByMaxLossPercent can't find data - " + ticker + " " + std::to_string(id));
				continue;
			}
			auto infoIt = std::find_if(infosIt->second.begin(), infosIt->second.end(), [&ticker](const auto& info) { return info.ticker == ticker; });
			if (infoIt == infosIt->second.end()) {
				utils::logError("calculation::balanceByMaxLossPercent can't find max loss data - " + ticker + " " + std::to_string(id));
				continue;
			}
			MaxLossBalancer balancer(ticker, interval, jsonIt->second, infoIt->maxLossPercent);
			balancer.calculate(algoType);
			if (balancer.getTerminated()) {
				minDealPercents[id] = -1.0;
			}
			else {
				minDealPercents[id] = utils::minFloat(minDealPercents[id], balancer.getDealPercent());
			}
		}
		utils::printProgress(static_cast<int>(ids.size()));
	}
}

void calculation::balanceByMaxLossPercent(const std::string& algoType, const combinationsCalculations& combinations, combinationsJsons& jsons,
										const calculationsType& calculations, size_t threadsAmount) {
	std::unordered_map<size_t, double> minDealPercents(jsons.size());
	std::vector<std::vector<size_t>> ids(threadsAmount);

	for_each(jsons.begin(), jsons.end(), [&ids, &minDealPercents, index = 0, mods = threadsAmount](const auto& pair) mutable {
		ids[index % mods].push_back(pair.first);
		minDealPercents[pair.first] = std::numeric_limits<double>::max();
		++index;
	});
	utils::setSummaryProgress(static_cast<int>(calculations.size() * jsons.size()));
	for (const auto& [ticker, interval] : calculations) {
		MaxLossBalancer::loadCandles(ticker, interval);
		std::vector<std::future<void>> futures;
		for (size_t i = 0; i < threadsAmount; ++i) {
			futures.push_back(std::async(std::launch::async, &balanceByMaxLossPercentAsync, algoType, ticker, interval, ids[i], jsons,
										combinations, std::ref(minDealPercents)));
		}
		for (const auto& future : futures) {
			future.wait();
		}
	}
	for (auto [id, balancedPercent] : minDealPercents) {
		if (utils::isLessOrEqual(balancedPercent, 0.0)) {
			jsons.erase(id);
		}
		else {
			jsons[id]["dealPercent"] = balancedPercent;
		}
	}
	MaxLossBalancer::cleanupCandels();
	utils::resetProgress();
	utils::log("calculation::balanceByMaxLossPercent finished. size - [ " + std::to_string(jsons.size()) + " ] ");
}

combinationsAverages calculation::getCalculationsAverages(const combinationsCalculations& aCalculations) {
	combinationsAverages averageInfo;
	for (const auto& united : aCalculations) {
		calculationInfo average;
		std::vector<double> profitsPerInterval;

		for (const auto& info : united.second) {
			average.cash += info.cash;

			// worst
			average.profitsFactor = utils::maxFloat(average.profitsFactor, info.profitsFactor);
			average.recoveryFactor = utils::minFloat(average.recoveryFactor,info.recoveryFactor);
			average.ordersPerInterval = utils::minFloat(average.ordersPerInterval, info.ordersPerInterval);
			average.maxLossPercent = utils::maxFloat(average.maxLossPercent, info.maxLossPercent);
			if (utils::isLess(info.profitPerInterval, average.profitPerIntervalWorst)) {
				average.ticker = info.ticker;
				average.profitPerIntervalWorst = info.profitPerInterval;
			}

			// median
			profitsPerInterval.push_back(info.profitPerInterval);
		}
		std::sort(profitsPerInterval.begin(), profitsPerInterval.end());
		average.profitPerInterval = getMedian(profitsPerInterval);

		averageInfo.push_back({ united.first, std::move(average) });
	}
	std::sort(averageInfo.begin(), averageInfo.end(), [](const auto& aLhs, const auto& aRhs) // to do look
		{ return std::make_tuple(aLhs.second.profitPerIntervalWorst, aLhs.second.profitPerInterval)
					> std::make_tuple(aRhs.second.profitPerIntervalWorst, aRhs.second.profitPerInterval) ; });
	return averageInfo;
}

void calculation::saveDataAndStats(const combinationsAverages& combinationsAverages, const combinationsJsons& combinationsJsons, int degree) {
	Json unitedData;
	Json unitedStats;
	double maxProfit = combinationsAverages[0].second.profitPerIntervalWorst;
	for (const auto& info : combinationsAverages) {
		Json data;
		data["cash"] = info.second.cash;
		if (auto it = combinationsJsons.find(info.first); it != combinationsJsons.end()) {
			data["data"] = it->second;
		}
		auto& stats = data["stats"];
		stats["profitsFactor"] = info.second.profitsFactor;
		stats["recoveryFactor"] = info.second.recoveryFactor;
		stats["ordersPerInterval"] = info.second.ordersPerInterval;
		stats["maxLossTicker"] = info.second.ticker;
		stats["maxLossPercent"] = info.second.maxLossPercent;
		stats["profitPerIntervalWorst"] = info.second.profitPerIntervalWorst;
		stats["profitPerInterval"] = info.second.profitPerInterval;
		unitedData.push_back(data);

		const auto weight = getWeight(info.second.profitPerIntervalWorst, maxProfit, degree);
		addStats(unitedStats, data["data"], weight);
	}

	std::ofstream unitedOutput(utils::outputDir + "/unitedData.txt");
	addHeadlines(unitedOutput, unitedStats, unitedData[0]);
	for (const auto& data : unitedData) {
		addData(unitedOutput, unitedStats, data);
	}
	saveStats(unitedStats, utils::outputDir + "/stats.json");
}