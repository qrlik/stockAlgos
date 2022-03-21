#include "statistic.h"

void statistic::initStatisticFromJson(statistic& aStats, const Json& aJson) {
	for (const auto& [field, value] : aJson.items()) {
		if (field == "maxLossHighCash") {
			aStats.maxLossHighCash = value.get<double>();
		}
		else if (field == "maxLossLowCash") {
			aStats.maxLossLowCash = value.get<double>();
		}
		else if (field == "summaryLoss") {
			aStats.summaryLoss = value.get<double>();
		}
		else if (field == "profitableOrder") {
			aStats.profitableOrder = value.get<size_t>();
		}
		else if (field == "unprofitableOrder") {
			aStats.unprofitableOrder = value.get<size_t>();
		}
		else if (field == "touchTrendOrder") {
			aStats.touchTrendOrder = value.get<size_t>();
		}
		else if (field == "breakTrendOrder") {
			aStats.breakTrendOrder = value.get<size_t>();
		}
		else if (field == "profitableStreak") {
			aStats.profitableStreak = value.get<size_t>();
		}
		else if (field == "unprofitableStreak") {
			aStats.unprofitableStreak = value.get<size_t>();
		}
	}
}

void statistic::onOpenOrder(bool aIsBreak) {
	if (aIsBreak) {
		breakTrendOrder += 1;
	}
	else {
		touchTrendOrder += 1;
	}
}

void statistic::onCloseOrder(double aCash, double aProfit) {
	if (aCash > currentLossHighCash) {
		currentLossHighCash = aCash;
		currentLossLowCash = aCash;
	}
	if (aCash < currentLossLowCash) {
		currentLossLowCash = aCash;
	}
	if ((currentLossHighCash - currentLossLowCash) > (maxLossHighCash - maxLossLowCash)) {
		maxLossHighCash = currentLossHighCash;
		maxLossLowCash = currentLossLowCash;
	}

	const bool isProfitable = aProfit > 0.0;
	if (!inited || lastOrderIsProfitable == isProfitable) {
		currentStreak += 1;
		inited = true;
	}
	else {
		currentStreak = 1;
	}
	lastOrderIsProfitable = isProfitable;

	if (isProfitable) {
		profitableOrder += 1;
		profitableStreak = std::max(profitableStreak, currentStreak);
	}
	else {
		unprofitableOrder += 1;
		unprofitableStreak = std::max(unprofitableStreak, currentStreak);
		summaryLoss += std::abs(aProfit);
	}
}

bool statistic::operator==(const statistic& aOther) {
	return maxLossHighCash == aOther.maxLossHighCash
		&& maxLossLowCash == aOther.maxLossLowCash
		&& summaryLoss == aOther.summaryLoss
		&& profitableOrder == aOther.profitableOrder
		&& unprofitableOrder == aOther.unprofitableOrder
		&& touchTrendOrder == aOther.touchTrendOrder
		&& breakTrendOrder == aOther.breakTrendOrder
		&& profitableStreak == aOther.profitableStreak
		&& unprofitableStreak == aOther.unprofitableStreak;
}
