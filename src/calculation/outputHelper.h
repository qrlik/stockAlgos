#pragma once
#include "json/json.hpp"
#include "market/candle.h"
#include "calculationSystem.h"
#include <string>
#include <fstream>

namespace calculation {
	struct calculationInfo {
		std::string ticker;
		double cash = 0.0;
		double profitsFactor = std::numeric_limits<double>::min();
		double recoveryFactor = std::numeric_limits<double>::max();
		double ordersPerInterval = std::numeric_limits<double>::max();
		double maxLossPercent = std::numeric_limits<double>::min();
		double profitPerInterval = 0.0;
	};

	using combinationId = size_t;
	using combinationsCalculations = std::unordered_map<size_t, std::vector<calculationInfo>>;
	using combinationsAverages = std::vector<std::pair<size_t, calculationInfo>>;
	using combinationsJsons = std::unordered_map<size_t, Json>;

	double getProfit(const Json& aData);
	double getWeight(double aProfit, double aMaxProfit, double aDegree);
	std::string getAllDataFilename();
	std::string getDirName(const std::string& aTicker, market::eCandleInterval aInterval);

	std::pair<combinationsCalculations, combinationsJsons> getCalculationsConjunction(const calculationsType& aCalculations);
	void alignByMaxLossPercent(const std::string& algoType, const combinationsCalculations& combinations, const combinationsJsons& jsons, const calculationsType& calculations);
	combinationsAverages getCalculationsAverages(const combinationsCalculations& aCalculations, size_t aSize);
	void saveDataAndStats(const combinationsAverages& combinationsAverages, const combinationsJsons& combinationsJsons, int degree);

	void addStats(Json& aStats, const Json& aData, double aWeight);
	void saveStats(Json& aStats, const std::string& aFileName);
	void addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample);
	void addData(std::ofstream& aOutput, const Json& aStats, const Json& aData);
}
