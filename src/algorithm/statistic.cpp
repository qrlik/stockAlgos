#include "statistic.h"
#include "algorithmDataBase.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

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

bool statistic::operator==(const statistic& aOther) const {
	bool result = true;
	result &= utils::isEqual(maxLossHighCash, aOther.maxLossHighCash, market::marketData::getInstance()->getQuotePrecision());
	result &= utils::isEqual(maxLossLowCash, aOther.maxLossLowCash, market::marketData::getInstance()->getQuotePrecision());
	result &= utils::isEqual(summaryLoss, aOther.summaryLoss, market::marketData::getInstance()->getQuotePrecision());
	result &= profitableOrder == aOther.profitableOrder;
	result &= unprofitableOrder == aOther.unprofitableOrder;
	result &= touchTrendOrder == aOther.touchTrendOrder;
	result &= breakTrendOrder == aOther.breakTrendOrder;
	result &= longOrder == aOther.longOrder;
	result &= shortOrder == aOther.shortOrder;
	result &= profitableStreak == aOther.profitableStreak;
	result &= unprofitableStreak == aOther.unprofitableStreak;
	return result;
}

void statistic::initFromJson(const Json& aJson) {
	for (const auto& [field, value] : aJson.items()) {
		if (field == "maxLossHighCash") {
			maxLossHighCash = value.get<double>();
		}
		else if (field == "maxLossLowCash") {
			maxLossLowCash = value.get<double>();
		}
		else if (field == "summaryLoss") {
			summaryLoss = value.get<double>();
		}
		else if (field == "profitableOrder") {
			profitableOrder = value.get<size_t>();
		}
		else if (field == "unprofitableOrder") {
			unprofitableOrder = value.get<size_t>();
		}
		else if (field == "touchTrendOrder") {
			touchTrendOrder = value.get<size_t>();
		}
		else if (field == "breakTrendOrder") {
			breakTrendOrder = value.get<size_t>();
		}
		else if (field == "longOrder") {
			longOrder = value.get<size_t>();
		}
		else if (field == "shortOrder") {
			shortOrder = value.get<size_t>();
		}
		else if (field == "profitableStreak") {
			profitableStreak = value.get<size_t>();
		}
		else if (field == "unprofitableStreak") {
			unprofitableStreak = value.get<size_t>();
		}
	}
}

void statistic::addJsonData(Json& aJson, const algorithmDataBase& aData, double aCash) const {
	aJson["orderProfit"] = profitableOrder;
	aJson["orderProfitStreak"] = profitableStreak;
	aJson["orderUnprofit"] = unprofitableOrder;
	aJson["orderUnprofitStreak"] = unprofitableStreak;
	auto maxLoss = maxLossHighCash - maxLossLowCash;
	auto maxLossPercentActual = (aData.getOrderSize() > 0.0) ? maxLoss / aData.getStartCash() * 100 : maxLoss / maxLossHighCash * 100;
	aJson["maxLossPercent"] = utils::round(maxLossPercentActual, 2);
	const auto recoveryFactor = (aCash - aData.getStartCash()) / summaryLoss;
	aJson["recoveryFactor"] = utils::round(recoveryFactor, 2);
	aJson["trendTouchOrder"] = touchTrendOrder;
	aJson["trendBreakOrder"] = breakTrendOrder;
	aJson["tLong"] = longOrder;
	aJson["tShort"] = shortOrder;
}
