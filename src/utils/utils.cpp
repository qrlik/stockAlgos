#include "utils.h"
#include <fstream>
#include <sstream>

using namespace utils;

Json utils::readFromJson(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	Json json;
	input >> json;
	return json;
}

std::string utils::getStringFromFile(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	std::string os;
	input >> os;
	return os;
}

void utils::saveToJson(const std::string& aPath, const Json& aData) {
	std::ofstream output(aPath + ".json");
	output << aData.dump(4);
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

double utils::round(double aValue, int aSize) {
	auto factor = static_cast<int>(std::pow(10, aSize));
	return std::round(aValue * factor) / factor;
}

double utils::ceil(double aValue, int aSize) {
	auto factor = static_cast<int>(std::pow(10, aSize));
	return std::ceil(aValue * factor) / factor;
}

double utils::floor(double aValue, int aSize) {
	auto factor = static_cast<int>(std::pow(10, aSize));
	return std::floor(aValue * factor) / factor;
}