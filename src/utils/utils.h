#pragma once
#include "../../third_party/json.hpp"
#include "../structs/candle.h"

namespace utils {
	Json readFromJson(const std::string& aPath);
	std::string getStringFromFile(const std::string& aPath);
	void saveToJson(const std::string& aPath, const Json& aData);
	std::vector<candle> parseCandles(const Json& aJson);

	double round(double aValue, int aSize);
	double ceil(double aValue, int aSize);
	double floor(double aValue, int aSize);
}