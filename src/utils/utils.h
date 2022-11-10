#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace utils {
	const std::string outputDir = "output";
	const std::string lastDataDir = outputDir + "/lastData";
	const std::string errorsDir = outputDir + "/errors";

	Json readFromJson(const std::string& aPath);
	void saveToJson(const std::string& aPath, const Json& aData);
	std::vector<market::candle> parseCandles(const Json& aJson);

	void createDir(const std::string& aDir);
	void log(const std::string& aStr);
	void logError(const std::string& aStr, Json data = Json{});
	void saveErrors();

	double round(double aValue, double aPrecision);
	double floor(double aValue, double aPrecision);
	double ceil(double aValue, double aPrecision);

	double maxFloat(double aLhs, double aRhs);
	double minFloat(double aLhs, double aRhs);

	bool isEqual(double aLhs, double aRhs, double aPrecision = -1.0);
	bool isGreater(double aLhs, double aRhs);
	bool isGreaterOrEqual(double aLhs, double aRhs);
	bool isLess(double aLhs, double aRhs);
	bool isLessOrEqual(double aLhs, double aRhs);

	void printProgress(int index, int summary);
	void printProgress(int added);
	void setSummaryProgress(int summary);
	void resetProgress();

	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}