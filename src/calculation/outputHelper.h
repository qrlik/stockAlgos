#pragma once
#include "json/json.hpp"
#include "market/candle.h"
#include <string>
#include <fstream>

namespace calculation {
	struct calculationInfo {
		std::string ticker; // actual or worst
		double cash = 0.0; // actual or summary
		double profitsFactor = std::numeric_limits<double>::min();
		double recoveryFactor = std::numeric_limits<double>::max();
		double ordersPerInterval = std::numeric_limits<double>::max();
		double maxLossPercent = std::numeric_limits<double>::min();
		double profitPerIntervalAverage = 0.0;
		double profitPerIntervalWorst = std::numeric_limits<double>::max();
		double profitPerInterval = 0.0; // actual or median
	};

	using combinationId = size_t;
	using combinationsCalculations = std::unordered_map<size_t, std::vector<calculationInfo>>;
	using combinationsAverages = std::vector<std::pair<size_t, calculationInfo>>;
	using combinationsJsons = std::unordered_map<size_t, Json>;
	using calculationsType = std::vector<std::pair<std::string, market::eCandleInterval>>;

	double getProfit(const Json& aData);
	double getWeight(double aProfit, double aMaxProfit, double aDegree);
	std::string getAllDataFilename();
	std::string getDirName(const std::string& aTicker, market::eCandleInterval aInterval);
	calculationInfo getCalculationInfo(const std::string& ticker, const Json& data);
	combinationsCalculations getCombinationsFromJson(const Json& balancedJson);

	std::pair<combinationsCalculations, combinationsJsons> getCalculationsConjunction(const calculationsType& calculations);
	void balanceByMaxLossPercent(const std::string& algoType, const combinationsCalculations& combinations, combinationsJsons& jsons, const calculationsType& calculations, size_t threadsAmount);
	combinationsAverages getCalculationsAverages(const combinationsCalculations& aCalculations);
	void saveDataAndStats(const combinationsAverages& combinationsAverages, Json balancedData, int degree);

	void addStats(Json& aStats, const Json& aData, double aWeight);
	void saveStats(Json& aStats, const std::string& aFileName);
	void addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample);
	void addData(std::ofstream& aOutput, const Json& aStats, const Json& aData);
}
