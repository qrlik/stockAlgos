#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace algorithm {
	class algorithmDataBase;
	struct statistic {
		statistic(const algorithmDataBase& aData, market::eCandleInterval aTimeframe);
		void onOpenOrder(bool isLong, double aTax);
		bool onCloseOrder(double aCash, double aProfit);
		bool operator==(const statistic& aOther) const;
		void initFromJson(const Json& aJson);
		void addJsonData(Json& aJson, double aCash, size_t aCandlesAmount) const;
		void incrementCounter(const std::string& aName, int aAmount = 1);
		void decrementCounter(const std::string& aName, int aAmount = 1);
	private:
		double getMaxLossPercentActual() const;
		const algorithmDataBase& data;

		std::unordered_map<std::string, int> statCounters;
		market::eCandleInterval timeframe = market::eCandleInterval::NONE;

		double maxLossHighCash = 0.0;
		double maxLossLowCash = 0.0;
		double maxLossCash = 0.0;
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