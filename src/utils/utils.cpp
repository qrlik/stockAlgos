#include "utils.h"
#include <fstream>
#include <iostream>

#include <filesystem>


using namespace utils;

Json utils::readFromJson(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	Json json;
	input >> json;
	return json;
}

void utils::saveToJson(const std::string& aPath, const Json& aData) {
	std::ofstream output(aPath + ".json");
	output << aData;
}

std::vector<candle> utils::parseCandles(const Json& aJson) {
	std::vector<candle> result;
	if (!aJson.is_array()) {
		return result;
	}
	result.reserve(aJson.size());
	for (const auto& candle : aJson) {
		result.push_back(parseCandleFromJson(candle));
	}
	return result;
}

void utils::log(const std::string& aStr) {
	std::cout << aStr + '\n';
}

void utils::logError(const std::string& aStr) {
	std::cout << "[ERROR]" + aStr + '\n';
}

double utils::round(double aValue, double aPrecision) {
	return std::round(aValue / aPrecision) * aPrecision;
}

double utils::floor(double aValue, double aPrecision) {
	return std::floor(aValue / aPrecision) * aPrecision;
}

double utils::ceil(double aValue, double aPrecision) {
	return std::ceil(aValue / aPrecision) * aPrecision;
}

bool utils::isEqual(double aLhs, double aRhs, double aPrecision) {
	if (aPrecision == -1.0) {
		return std::fabs(aLhs - aRhs) <= std::min(std::fabs(aLhs), std::fabs(aRhs)) * std::numeric_limits<double>::epsilon();
	}
	return std::fabs(aLhs - aRhs) <= aPrecision;
}