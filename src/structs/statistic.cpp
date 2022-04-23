#include "statistic.h"
#include "market/marketRules.h"
#include "utils/utils.h"

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
		else if (field == "longOrder") {
			aStats.longOrder = value.get<size_t>();
		}
		else if (field == "shortOrder") {
			aStats.shortOrder = value.get<size_t>();
		}
		else if (field == "profitableStreak") {
			aStats.profitableStreak = value.get<size_t>();
		}
		else if (field == "unprofitableStreak") {
			aStats.unprofitableStreak = value.get<size_t>();
		}
	}
}

statistic::statistic(double aStartCash, double aMaxLossPercent, double aMaxLossCash):
	currentLossHighCash(aStartCash),
	currentLossLowCash(aStartCash),
	maxLossPercent(aMaxLossPercent),
	maxLossCash(aMaxLossCash) {}

void statistic::onOpenOrder(bool isLong, bool aIsBreak) {
	if (aIsBreak) {
		breakTrendOrder += 1;
	}
	else {
		touchTrendOrder += 1;
	}
	if (isLong) {
		longOrder += 1;
	}
	else {
		shortOrder += 1;
	}
}

bool statistic::onCloseOrder(double aCash, double aProfit) {
	if (aCash > currentLossHighCash) {
		currentLossHighCash = aCash;
		currentLossLowCash = aCash;
	}
	if (aCash < currentLossLowCash) {
		currentLossLowCash = aCash;
	}
	if (utils::isEqual(maxLossHighCash, 0.0) && !utils::isEqual(currentLossHighCash, 0.0)
		|| (currentLossLowCash / currentLossHighCash) < (maxLossLowCash / maxLossHighCash)
		|| (maxLossCash > 0.0 && (currentLossHighCash - currentLossLowCash) > (maxLossHighCash - maxLossLowCash))) {
		maxLossHighCash = currentLossHighCash;
		maxLossLowCash = currentLossLowCash;
	}

	const auto curMaxLoss = maxLossHighCash - maxLossLowCash;
	const auto curMaxLossPercent = curMaxLoss / maxLossHighCash * 100.0;
	if (curMaxLossPercent > maxLossPercent || (maxLossCash > 0.0 && curMaxLoss > maxLossCash)) {
		return true;
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
	return false;
}

bool statistic::operator==(const statistic& aOther) {
	assert(utils::isEqual(maxLossHighCash, aOther.maxLossHighCash, market::marketData::getInstance()->getQuotePrecision()));
	assert(utils::isEqual(maxLossLowCash, aOther.maxLossLowCash, market::marketData::getInstance()->getQuotePrecision()));
	assert(utils::isEqual(summaryLoss, aOther.summaryLoss, market::marketData::getInstance()->getQuotePrecision()));
	assert(profitableOrder == aOther.profitableOrder);
	assert(unprofitableOrder == aOther.unprofitableOrder);
	assert(touchTrendOrder == aOther.touchTrendOrder);
	assert(breakTrendOrder == aOther.breakTrendOrder);
	assert(longOrder == aOther.longOrder);
	assert(shortOrder == aOther.shortOrder);
	assert(profitableStreak == aOther.profitableStreak);
	assert(unprofitableStreak == aOther.unprofitableStreak);
	return true;
}
