#include "statistic.h"
#include "algorithmDataBase.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

statistic::statistic(const algorithmDataBase& aData):
	data(aData),
	currentLossHighCash(aData.getStartCash()),
	currentLossLowCash(aData.getStartCash()) {}

void statistic::incrementCounter(const std::string& aName, int aAmount) {
	statCounters[aName] += aAmount;
}

void statistic::decrementCounter(const std::string& aName, int aAmount) {
	statCounters[aName] -= aAmount;
}

void statistic::onOpenOrder(bool isLong) {
	if (isLong) {
		incrementCounter("longOrder");
	}
	else {
		incrementCounter("shortOrder");
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
		|| (data.getMaxLossCash() > 0.0 && (currentLossHighCash - currentLossLowCash) > (maxLossHighCash - maxLossLowCash))) {
		maxLossHighCash = currentLossHighCash;
		maxLossLowCash = currentLossLowCash;
	}

	const auto curMaxLoss = maxLossHighCash - maxLossLowCash;
	const auto curMaxLossPercent = curMaxLoss / maxLossHighCash * 100.0;
	if (curMaxLossPercent > data.getMaxLossPercent() || (data.getMaxLossCash() > 0.0 && curMaxLoss > data.getMaxLossCash())) {
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
		incrementCounter("profitableOrder");
		profitableStreak = std::max(profitableStreak, currentStreak);
	}
	else {
		incrementCounter("unprofitableOrder");
		unprofitableStreak = std::max(unprofitableStreak, currentStreak);
		summaryLoss += std::abs(aProfit);
	}
	return false;
}

bool statistic::operator==(const statistic& aOther) const {
	bool result = true;
	result &= utils::isEqual(maxLossHighCash, aOther.maxLossHighCash, MARKET_DATA->getQuotePrecision());
	result &= utils::isEqual(maxLossLowCash, aOther.maxLossLowCash, MARKET_DATA->getQuotePrecision());
	result &= utils::isEqual(summaryLoss, aOther.summaryLoss, MARKET_DATA->getQuotePrecision());
	result &= statCounters == aOther.statCounters;
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
		else if (field == "profitableStreak") {
			profitableStreak = value.get<size_t>();
		}
		else if (field == "unprofitableStreak") {
			unprofitableStreak = value.get<size_t>();
		}
		else {
			statCounters[field] = value.get<int>();
		}
	}
}

void statistic::addJsonData(Json& aJson, double aCash) const {
	aJson["orderProfitStreak"] = profitableStreak;
	aJson["orderUnprofitStreak"] = unprofitableStreak;
	auto maxLoss = maxLossHighCash - maxLossLowCash;
	auto maxLossPercentActual = (data.getOrderSize() > 0.0) ? maxLoss / data.getStartCash() * 100 : maxLoss / maxLossHighCash * 100;
	aJson["maxLossPercent"] = utils::round(maxLossPercentActual, 0.01);
	const auto recoveryFactor = (aCash - data.getStartCash()) / summaryLoss;
	aJson["recoveryFactor"] = utils::round(recoveryFactor, 0.01);

	for (const auto& [name, count] : statCounters) {
		aJson[name] = count;
	}
}
