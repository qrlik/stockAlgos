#include "statistic.h"

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
