#pragma once
#include "json/json.hpp"
#include "market/candle.h"
#include <string>
#include <fstream>

namespace calculation {
	struct calculationInfo {
		double cash = 0.0;
		double profitsFactor = 0.0;
		double recoveryFactor = 0.0;
		double ordersPerInterval = 0.0;
		double maxLossPercent = 0.0;
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
	std::pair<combinationsCalculations, combinationsJsons> getCalculationsConjunction(const std::vector<std::pair<std::string, market::eCandleInterval>>& aCalculations, int aDegree);
	combinationsAverages getCalculationsAverages(const combinationsCalculations& aCalculations, size_t aSize);

	void addStats(Json& aStats, const Json& aData, double aWeight);
	void saveStats(Json& aStats, const std::string& aFileName);
	void addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample);
	void addData(std::ofstream& aOutput, const Json& aStats, const Json& aData);
}
