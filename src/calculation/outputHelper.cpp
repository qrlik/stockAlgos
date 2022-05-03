#include "outputHelper.h"

using namespace calculation;

namespace {
	const auto doubleWidth = 16;
	const auto intWidth = 10;
	const auto boolWidth = 8;
	const auto spaceWidth = 4;

	int getWidth(const std::string& aName, const Json& aValue) {
		auto width = static_cast<int>(aName.length()) + spaceWidth;
		if (aValue.is_number_float()) {
			width = std::max(width, doubleWidth);
		}
		else if (aValue.is_number_integer()) {
			width = std::max(width, intWidth);
		}
		else {
			width = std::max(width, boolWidth);
		}
		return width;
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
		std::ostringstream os;
		os << value;
		incrementCb(aStats[name], os.str());
	}
}

void calculation::saveStats(Json& aStats, const std::string& aFileName) {
	for (auto& var : aStats) {
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

	std::ofstream statsOutput(aFileName);
	statsOutput << std::setw(spaceWidth) << aStats;
}

void calculation::addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample) {
	aOutput << std::setw(doubleWidth) << "Cash";
	for (const auto& [name, value] : aExample["stats"].items()) {
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	for (const auto& [name, value] : aExample["data"].items()) {
		if (aStats[name]["counts"].size() < 2) {
			continue;
		}
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	aOutput << '\n';
}

void calculation::addData(std::ofstream& aOutput, const Json& aStats, const Json& aData) {
	aOutput << std::setw(doubleWidth) << aData["cash"];
	for (const auto& [name, value] : aData["stats"].items()) {
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	for (const auto& [name, value] : aData["data"].items()) {
		if (aStats[name]["counts"].size() < 2) {
			continue;
		}
		aOutput << std::setw(getWidth(name, value)) << name;
	}
	aOutput << '\n';
}