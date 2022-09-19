#include "maxLossBalancer.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
#include "utils/utils.h"

using namespace calculation;

MaxLossBalancer::MaxLossBalancer(const std::string& ticker, market::eCandleInterval interval, const Json& data, double maxLossPercent) :
	mData(data),
	mTicker(ticker),
	mMaxLossPercentCeil(data["maxLossPercent"].get<double>()),
	mActualDealPercent(data["dealPercent"].get<double>()),
	mActualMaxLossPercent(maxLossPercent),
	mInterval(interval)
{
	mLastMaxLossPercent = mActualMaxLossPercent;
	mLastDealPercent = mActualDealPercent;
	if (utils::isGreater(mLastMaxLossPercent, mMaxLossPercentCeil) || utils::isLessOrEqual(mLastDealPercent, 0.0)) {
		utils::logError("MaxLossBalancer wrong data");
	}
	auto candlesJson = utils::readFromJson("assets/candles/" + mTicker + '_' + getCandleIntervalApiStr(mInterval));
	mCandles = utils::parseCandles(candlesJson);
}

void MaxLossBalancer::calculate(const std::string& algoType) {
	while (isReadyForBalance()) {
		if (algoType == "superTrend") {
			iterate<algorithm::stAlgorithm>();
		}
		else if (algoType == "superTrendMA") {
			iterate<algorithm::stMAlgorithm>();
		}
		else {
			terminate();
			utils::logError("calculationSystem::calculate unknown algorithm type");
		}
	}
}

bool MaxLossBalancer::isValid() const {
	auto percentCheck = utils::isLessOrEqual(mLastMaxLossPercent, mMaxLossPercentCeil);
	if (!percentCheck) {
		utils::logError("MaxLossBalancer::isValid max loss ceil break");
	}
	return !mTerminated && percentCheck;
}

bool MaxLossBalancer::isReadyForBalance() const {
	return !mBalanced && isValid();
}

void MaxLossBalancer::terminate() {
	mTerminated = true;
}

void MaxLossBalancer::increaseValues(bool success) {
	if (success) {
		mLastIncreaseFactor = 1.0;
	}
	else {
		mLastIncreaseFactor /= 2;
	}
	mLastDealPercent *= 1 + mLastIncreaseFactor;
	mLastDealPercent = utils::floor(mLastDealPercent, mDealPercentPrecision);
}

void MaxLossBalancer::onSuccess() {
	mActualMaxLossPercent = mLastMaxLossPercent;
	mActualDealPercent = mLastDealPercent;
	increaseValues(true);
}

void MaxLossBalancer::onOverhead() {
	if (utils::isLessOrEqual(mLastDealPercent - mActualDealPercent, mDealPercentPrecision)) {
		mBalanced = true;
		return;
	}
	increaseValues(false);
}