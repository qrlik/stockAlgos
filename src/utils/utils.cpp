#include "utils.h"
#include <fstream>

using namespace utils;

Json utils::readFromJson(const std::string& aPath) {
	std::ifstream input(aPath + ".json");
	Json json;
	input >> json;
	return json;
}

std::vector<binance::candle> utils::parseCandles(const Json& aJson) {
	std::vector<binance::candle> result;
	if (!aJson.is_array()) {
		return result;
	}
	result.reserve(aJson.size());
	for (const auto& candle : aJson) {
		result.push_back(binance::parseCandleFromJson(candle));
	}
	return result;
}