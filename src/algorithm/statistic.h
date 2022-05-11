#pragma once
#include "json/json.hpp"

namespace algorithm {
	class algorithmDataBase;
	struct statistic {
		statistic(const algorithmDataBase& aData);
		void onOpenOrder(bool isLong);
		bool onCloseOrder(double aCash, double aProfit);
		bool operator==(const statistic& aOther) const;
		void initFromJson(const Json& aJson);
		void addJsonData(Json& aJson, double aCash) const;
		void incrementCounter(const std::string& aName, int aAmount = 1);
		void decrementCounter(const std::string& aName, int aAmount = 1);
	private:
		const algorithmDataBase& data;

		std::unordered_map<std::string, int> statCounters;
		double maxLossHighCash = 0.0;
		double maxLossLowCash = 0.0;
		double summaryLoss = 0.0;

		double currentLossHighCash = 0.0;
		double currentLossLowCash = 0.0;

		size_t currentStreak = 0;
		size_t profitableStreak = 0;
		size_t unprofitableStreak = 0;

		bool lastOrderIsProfitable = false;
		bool inited = false;
	};
}