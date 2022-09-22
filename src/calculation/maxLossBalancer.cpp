#include "maxLossBalancer.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
#include "utils/utils.h"

using namespace calculation;

namespace {
	std::vector<market::candle> lastCandles;
	std::string lastTicker;
}

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
	if (ticker != lastTicker) {
		lastTicker = ticker;
		auto candlesJson = utils::readFromJson("assets/candles/" + mTicker + '_' + getCandleIntervalApiStr(mInterval));
		lastCandles = utils::parseCandles(candlesJson);
	}
}

void MaxLossBalancer::calculate(const std::string& algoType) {
	increaseValues(true);
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
	auto percentCheck = utils::isLessOrEqual(mActualMaxLossPercent, mMaxLossPercentCeil);
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
	if (!mStepsStarted) {
		if (!success) {
			if (utils::isGreater(mIncreaseFactor / 2, 1.0)) {
				mIncreaseFactor /= 2;
			}
			else {
				auto mod = (mIncreaseFactor - 1.0) / 2;
				mIncreaseFactor = 1.0 + mod;
			}
		}
		mLastDealPercent = mActualDealPercent;
		mLastDealPercent *= mIncreaseFactor;
		mLastDealPercent = utils::floor(mLastDealPercent, mDealPercentPrecision);

		if (utils::isLessOrEqual(mLastDealPercent, mActualDealPercent)) {
			mStepsStarted = true;
		}
	}
	if (mStepsStarted) {
		mLastDealPercent += mDealPercentPrecision;
		mLastDealPercent = utils::floor(mLastDealPercent, mDealPercentPrecision);
	}
}

void MaxLossBalancer::onSuccess() {
	mActualMaxLossPercent = mLastMaxLossPercent;
	mActualDealPercent = mLastDealPercent;
	increaseValues(true);
	while (!mStepsStarted && utils::isGreaterOrEqual(mLastDealPercent, mMinFailedPercent)) {
		increaseValues(false);
	}
}

void MaxLossBalancer::onOverhead() {
	mMinFailedPercent = utils::minFloat(mMinFailedPercent, mLastDealPercent);
	if (mStepsStarted) {
		onBalanced();
		return;
	}
	increaseValues(false);
}

void MaxLossBalancer::onBalanced() {
	mBalanced = true;
	if (utils::isLess(mMinFailedPercent, mActualDealPercent) || utils::isGreaterOrEqual(mMinFailedPercent - mActualDealPercent, mDealPercentPrecision * 1.5)) {
		utils::logError("MaxLossBalancer::onBalanced wrong algorithm result - " + std::to_string(mData["id"].get<size_t>()));
	}
}