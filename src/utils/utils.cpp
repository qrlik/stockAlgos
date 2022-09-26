#include "utils.h"
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

using namespace utils;

namespace {
	std::mutex printMutex;
	int progress = 0;
}

Json utils::readFromJson(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	Json json;
	if (input) {
		input >> json;
	}
	return json;
}

void utils::saveToJson(const std::string& aPath, const Json& aData) {
	std::ofstream output(aPath + ".json");
	output << aData;
}

std::vector<market::candle> utils::parseCandles(const Json& aJson) {
	std::vector<market::candle> result;
	if (!aJson.is_array()) {
		return result;
	}
	result.reserve(aJson.size());
	for (const auto& candle : aJson) {
		result.push_back(market::parseCandleFromJson(candle));
	}
	return result;
}

void utils::createDir(const std::string& aDir) {
	std::filesystem::create_directories(aDir);
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

double utils::maxFloat(double aLhs, double aRhs) {
	return (isGreater(aLhs, aRhs)) ? aLhs : aRhs;
}

double utils::minFloat(double aLhs, double aRhs) {
	return (isGreater(aLhs, aRhs)) ? aRhs : aLhs;
}

bool utils::isEqual(double aLhs, double aRhs, double aPrecision) {
	if (aPrecision == -1.0) {
		return std::fabs(aLhs - aRhs) <= std::min(std::fabs(aLhs), std::fabs(aRhs)) * std::numeric_limits<double>::epsilon();
	}
	return std::fabs(aLhs - aRhs) <= aPrecision;
}

bool utils::isGreater(double aLhs, double aRhs) {
	return aLhs - aRhs > (std::max(std::fabs(aLhs), std::fabs(aRhs))) * std::numeric_limits<double>::epsilon();
}

bool utils::isGreaterOrEqual(double aLhs, double aRhs) {
	return isGreater(aLhs, aRhs) || isEqual(aLhs, aRhs);
}

bool utils::isLess(double aLhs, double aRhs) {
	return aRhs - aLhs > (std::max(std::fabs(aLhs), std::fabs(aRhs))) * std::numeric_limits<double>::epsilon();
}

bool utils::isLessOrEqual(double aLhs, double aRhs) {
	return isLess(aLhs, aRhs) || isEqual(aLhs, aRhs);
}

void utils::printProgress(int index, int summary) {
	const auto newProgress = static_cast<int>(static_cast<double>(index) / summary * 100);
	if (newProgress > progress) {
		std::lock_guard<std::mutex> lock(printMutex);
		progress = newProgress;
		std::cout << std::to_string(progress) + "%...";
		std::cout.flush();
	}
}

void utils::resetProgress() {
	progress = 0;
}