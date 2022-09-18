#include "maxLossBalancer.h"
#include "utils/utils.h"

using namespace calculation;

namespace {
	const double dealPercentPrecision = 0.01;
}

MaxLossBalancer::MaxLossBalancer(const std::string ticker, market::eCandleInterval interval, const Json& data, double maxLossPercent) :
	mData(data),
	mTicker(ticker),
	mMaxLossPercentCeil(data["maxLossPercent"].get<double>()),
	mCurDealPercent(data["dealPercent"].get<double>()),
	mCurMaxLossPercent(maxLossPercent),
	mInterval(interval)
{
	if (utils::isGreater(mCurMaxLossPercent, mMaxLossPercentCeil)
		|| utils::isLessOrEqual(mCurDealPercent, 0.0)) {
		utils::logError("MaxLossBalancer wrong data");
	}
}

void MaxLossBalancer::calculate() {
	std::vector<market::candle> candles;
	{
		auto candlesJson = utils::readFromJson("assets/candles/" + mTicker + '_' + getCandleIntervalApiStr(mInterval));
		candles = utils::parseCandles(candlesJson);
	}
	while (isReadyForBalance()) {
		iterate();
	}
}

bool MaxLossBalancer::isValid() const {
	return utils::isLessOrEqual(mCurMaxLossPercent, mMaxLossPercentCeil);
}

bool MaxLossBalancer::isReadyForBalance() const {
	return !mBalanced && isValid();
}

void MaxLossBalancer::iterate() {

}