#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace utils {
	const std::string outputDir = "output";

	Json readFromJson(const std::string& aPath);
	void saveToJson(const std::string& aPath, const Json& aData);
	std::vector<market::candle> parseCandles(const Json& aJson);

	void createDir(const std::string& aDir);
	void log(const std::string& aStr);
	void logError(const std::string& aStr);

	double round(double aValue, double aPrecision);
	double floor(double aValue, double aPrecision);
	double ceil(double aValue, double aPrecision);

	bool isEqual(double aLhs, double aRhs, double aPrecision = -1.0);
	bool isGreater(double aLhs, double aRhs);
	bool isLess(double aLhs, double aRhs);
}