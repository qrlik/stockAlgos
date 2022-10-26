#include "statistic.h"
#include "algorithmDataBase.h"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace algorithm;

statistic::statistic(const algorithmDataBase& aData, market::eCandleInterval aTimeframe):
	data(aData),
	timeframe(aTimeframe),
	currentLossHighCash(aData.getStartCash()),
	currentLossLowCash(aData.getStartCash())
{
	statCounters["profitableOrder"];
	statCounters["unprofitableOrder"];
	statCounters["orderCounter"];
}

void statistic::incrementCounter(const std::string& aName, int aAmount) {
	statCounters[aName] += aAmount;
}

void statistic::decrementCounter(const std::string& aName, int aAmount) {
	statCounters[aName] -= aAmount;
}

void statistic::onOpenOrder(bool isLong, double aTax) {
	if (isLong) {
		incrementCounter("longOrder");
	}
	else {
		incrementCounter("shortOrder");
	}
	incrementCounter("orderCounter");
	summaryLoss += aTax;
}

bool statistic::onCloseOrder(double aCash, double aProfit) {
	if (utils::isGreater(aCash, currentLossHighCash)) {
		currentLossHighCash = aCash;
		currentLossLowCash = aCash;
	}
	if (utils::isLess(aCash, currentLossLowCash)) {
		currentLossLowCash = aCash;
	}
	if (utils::isEqual(maxLossHighCash, 0.0) && !utils::isEqual(currentLossHighCash, 0.0)
		|| utils::isLess(currentLossLowCash / currentLossHighCash, maxLossLowCash / maxLossHighCash)) {
		maxLossHighCash = currentLossHighCash;
		maxLossLowCash = currentLossLowCash;
	}
	maxLossCash = utils::maxFloat(maxLossCash, (currentLossHighCash - currentLossLowCash));

	if (utils::isGreater(getMaxLossPercentActual(), data.getMaxLossPercent()) ||
		(utils::isGreater(data.getMaxLossCash(), 0.0) && utils::isGreaterOrEqual(maxLossCash, data.getMaxLossCash()))) {
		return true;
	}

	const bool isProfitable = utils::isGreater(aProfit, 0.0);
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
	const auto& marketData = data.getMarketData();
	result &= utils::isEqual(maxLossHighCash, aOther.maxLossHighCash, marketData.getQuotePrecision());
	result &= utils::isEqual(maxLossLowCash, aOther.maxLossLowCash, marketData.getQuotePrecision());
	result &= utils::isEqual(summaryLoss, aOther.summaryLoss, marketData.getQuotePrecision());
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

double statistic::getMaxLossPercentActual() const {
	auto maxLoss = maxLossHighCash - maxLossLowCash;
	return (utils::isGreater(data.getOrderSize(), 0.0)) ? (maxLoss / data.getStartCash() * 100) : (maxLoss / maxLossHighCash * 100);
}

void statistic::addJsonData(Json& aJson, double aCash, size_t aCandlesAmount) const {
	aJson["orderProfitStreak"] = profitableStreak;
	aJson["orderUnprofitStreak"] = unprofitableStreak;
	aJson["maxLossPercent"] = getMaxLossPercentActual();
	aJson["recoveryFactor"] = (aCash - data.getStartCash()) / summaryLoss; // more - good

	const auto divider = (statCounters.at("profitableOrder")) ? statCounters.at("profitableOrder") : 1;
	aJson["profitsFactor"] = static_cast<double>(statCounters.at("unprofitableOrder")) / divider; // less - good

	const auto timeframesPerInterval = static_cast<int>(data.getStatsInterval()) / static_cast<int>(timeframe);
	const auto intervalsAmount = static_cast<double>(aCandlesAmount) / timeframesPerInterval;
	aJson["ordersPerInterval"] = statCounters.at("orderCounter") / intervalsAmount;

	const auto cashProfitFactor = aCash / data.getStartCash();
	aJson["profitPerInterval"] = (std::pow(cashProfitFactor, 1 / (intervalsAmount - 1)) - 1) * 100.0; // % per stats interval

	for (const auto& [name, count] : statCounters) {
		aJson[name] = count;
	}
}
