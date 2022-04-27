#pragma once
#include "json/json.hpp"
#include <string>

struct statistic {
	static void initStatisticFromJson(statistic& aStats, const Json& aJson);

	statistic(double aStartCash, double aMaxLossPercent, double aMaxLossCash);
	void onOpenOrder(bool isLong, bool aIsBreak);
	bool onCloseOrder(double aCash, double aProfit);
	bool operator==(const statistic& aOther) const;

	double maxLossHighCash = 0.0;
	double maxLossLowCash = 0.0;
	double summaryLoss = 0.0;

	size_t profitableOrder = 0;
	size_t unprofitableOrder = 0;
	size_t touchTrendOrder = 0;
	size_t breakTrendOrder = 0;
	size_t longOrder = 0;
	size_t shortOrder = 0;

	size_t profitableStreak = 0;
	size_t unprofitableStreak = 0;

private:
	double currentLossHighCash = 0.0;
	double currentLossLowCash = 0.0;
	double maxLossPercent = 0.0;
	double maxLossCash = 0.0;
	size_t currentStreak = 0;
	bool lastOrderIsProfitable = false;
	bool inited = false;
};