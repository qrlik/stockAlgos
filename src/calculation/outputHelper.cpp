#include "outputHelper.h"
#include "utils/utils.h"

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
		if (name == "id") {
			continue;
		}
		incrementCb(aStats[name], getStrFromJson(value));
	}
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

std::string calculation::getAllDataFilename() {
	return allDataFileName;
}

std::string calculation::getDirName(const std::string& aTicker, market::eCandleInterval aInterval) {
	return utils::outputDir + '/' + aTicker + '_' + market::getCandleIntervalApiStr(aInterval) + '/';
}

std::pair<combinationsCalculations, combinationsJsons> calculation::getCalculationsConjunction(const std::vector<std::pair<std::string, market::eCandleInterval>>& aCalculations) {
	combinationsCalculations unitedInfo;
	combinationsJsons idToJsons;

	const auto size = aCalculations.size();
	for (const auto& [ticker, timeframe] : aCalculations) {
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
			info.profitPerInterval = data["stats"]["profitPerInterval"].get<double>();
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

	return { std::move(unitedInfo), std::move(idToJsons) };
}

combinationsAverages calculation::getCalculationsAverages(const combinationsCalculations& aCalculations, size_t aSize) {
	combinationsAverages averageInfo;
	for (const auto& united : aCalculations) {
		calculationInfo average;
		for (const auto& info : united.second) {
			average.weight += info.weight;
			average.cash += info.cash;
			average.profitsFactor += info.profitsFactor;
			average.recoveryFactor += info.recoveryFactor;
			average.ordersPerInterval += info.ordersPerInterval;
			average.profitPerInterval += info.profitPerInterval;
		}
		average.weight /= aSize;
		average.profitsFactor /= aSize;
		average.recoveryFactor /= aSize;
		average.ordersPerInterval /= aSize;
		average.profitPerInterval /= aSize;
		averageInfo.push_back({ united.first, average });
	}
	std::sort(averageInfo.begin(), averageInfo.end(), [](const auto& aLhs, const auto& aRhs) { return aLhs.second.cash > aRhs.second.cash; });
	return averageInfo;
}