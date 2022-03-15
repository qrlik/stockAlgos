#pragma once
#include "../structs/candle.h"
#include <vector>

namespace indicators {
	enum class eAtrType : unsigned char {
		NONE = 0,
		RMA = 1,
		EMA = 2,
		WMA = 3,
		SMA = 4
	};
	struct indicatorsData {
		double lastEma = 0.0;
		double lastUpperBand = 0.0;
		double lastLowerBand = 0.0;
		double lastTrend = 0.0;
		double lastClose = 0.0;
	};

	std::string atrTypeToString(eAtrType aType);
	eAtrType atrTypeFromString(const std::string& aStr);
	void getProcessedCandles(std::vector<candle>& aCandles, eAtrType aType, size_t aSize, double aFactor, size_t aAmount = 0);
}
