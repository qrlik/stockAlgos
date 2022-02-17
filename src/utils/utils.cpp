#include "utils.h"
#include <fstream>

using namespace utils;

Json utils::readFromJson(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	Json json;
	input >> json;
	return json;
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

double utils::round(double aValue, int size) {
	double factor = std::pow(10, size);
	return std::round(aValue * factor) / factor;
}