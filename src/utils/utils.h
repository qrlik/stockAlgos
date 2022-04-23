#pragma once
#include "json/json.hpp"
#include "structs/candle.h"

namespace utils {
	Json readFromJson(const std::string& aPath);
	void saveToJson(const std::string& aPath, const Json& aData);
	std::vector<candle> parseCandles(const Json& aJson);

	void logError(const std::string& aStr);
	double round(double aValue, double aPrecision);
	double floor(double aValue, double aPrecision);
	double ceil(double aValue, double aPrecision);
	bool isEqual(double aLhs, double aRhs, double aPrecision = -1.0);
}