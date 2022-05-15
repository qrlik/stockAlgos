#pragma once
#include "json/json.hpp"
#include <string>

namespace market {
	enum class eCandleInterval {
		NONE = 0,
		ONE_MIN = 60000,
		THREE_MIN = 180000,
		FIVE_MIN = 300000,
		FIFTEEN_MIN = 900000,
		THIRTY_MIN = 1800000,
		ONE_HOUR = 3600000,
		TWO_HOUR = 7200000,
		FOUR_HOUR = 14400000,
		SIX_HOUR = 21600000,
		EIGHT_HOUR = 28800000,
		TWELVE_HOUR = 43200000,
		ONE_DAY = 86400000,
		THREE_DAY = 259200000,
		ONE_WEEK = 604800000
	};

	struct candle {
		std::string time;
		double open = 0.f;
		double high = 0.f;
		double low = 0.f;
		double close = 0.f;
	};

	std::string getCandleIntervalApiStr(eCandleInterval aInterval);
	eCandleInterval getCandleIntervalFromStr(const std::string& aInterval);
	candle parseCandleFromJson(const Json& aJson);
}