#include "candle.h"

using namespace binance;

namespace {
	const std::unordered_map<candleInterval, std::string> candleIntervalToStr{
		{ candleInterval::ONE_MIN, "1m" },
		{ candleInterval::THREE_MIN, "3m" },
		{ candleInterval::FIVE_MIN, "5m" },
		{ candleInterval::FIFTEEN_MIN, "15m" },
		{ candleInterval::THIRTY_MIN, "30m" },
		{ candleInterval::ONE_HOUR, "1h" },
		{ candleInterval::TWO_HOUR, "2h" },
		{ candleInterval::FOUR_HOUR, "4h" },
		{ candleInterval::SIX_HOUR, "6h" },
		{ candleInterval::EIGHT_HOUR, "8h" },
		{ candleInterval::TWELVE_HOUR, "12h" },
		{ candleInterval::ONE_DAY, "1d" },
		{ candleInterval::THREE_DAY, "3d" },
		{ candleInterval::ONE_WEEK, "1w" }
	};
}

std::string binance::getCandleIntervalApiStr(candleInterval aInterval) {
	if (auto it = candleIntervalToStr.find(aInterval); it != candleIntervalToStr.end()) {
		return it->second;
	}
	return "";
}