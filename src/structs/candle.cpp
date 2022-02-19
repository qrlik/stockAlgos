#include "candle.h"
#include "../utils/utils.h"

namespace {
	const std::unordered_map<eCandleInterval, std::string> candleIntervalToStr{
		{ eCandleInterval::ONE_MIN, "1m" },
		{ eCandleInterval::THREE_MIN, "3m" },
		{ eCandleInterval::FIVE_MIN, "5m" },
		{ eCandleInterval::FIFTEEN_MIN, "15m" },
		{ eCandleInterval::THIRTY_MIN, "30m" },
		{ eCandleInterval::ONE_HOUR, "1h" },
		{ eCandleInterval::TWO_HOUR, "2h" },
		{ eCandleInterval::FOUR_HOUR, "4h" },
		{ eCandleInterval::SIX_HOUR, "6h" },
		{ eCandleInterval::EIGHT_HOUR, "8h" },
		{ eCandleInterval::TWELVE_HOUR, "12h" },
		{ eCandleInterval::ONE_DAY, "1d" },
		{ eCandleInterval::THREE_DAY, "3d" },
		{ eCandleInterval::ONE_WEEK, "1w" }
	};
}

std::string getCandleIntervalApiStr(eCandleInterval aInterval) {
	if (auto it = candleIntervalToStr.find(aInterval); it != candleIntervalToStr.end()) {
		return it->second;
	}
	return "";
}

candle parseCandleFromJson(const Json& aJson) {
	auto result = candle();
	for (const auto& [field, value] : aJson.items()) {
		if (field == "time") {
			result.time = value.get<std::string>();
		}
		else if (field == "open") {
			result.open = value.get<double>();
		}
		else if (field == "high") {
			result.high = value.get<double>();
		}
		else if (field == "low") {
			result.low = value.get<double>();
		}
		else if (field == "close") {
			result.close = value.get<double>();
		}
	}
	return result;
}