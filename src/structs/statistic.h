#pragma once
#include <string>

struct statistic {
	void onOpenOrder(bool aIsBreak);
	void onCloseOrder(double aCash, double aProfit);
	bool operator==(const statistic& aOther);

	double maxLossHighCash = 0.0;
	double maxLossLowCash = 0.0;
	double summaryLoss = 0.0;

	size_t profitableOrder = 0;
	size_t unprofitableOrder = 0;
	size_t touchTrendOrder = 0;
	size_t breakTrendOrder = 0;

	size_t profitableStreak = 0;
	size_t unprofitableStreak = 0;

private:
	double currentLossHighCash = 0.0;
	double currentLossLowCash = 0.0;
	size_t currentStreak = 0;
	bool lastOrderIsProfitable = false;
	bool inited = false;
};