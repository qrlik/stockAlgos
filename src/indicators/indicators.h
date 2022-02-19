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

	void calculateRangeAtr(std::vector<candle>& aCandles, eAtrType aType, size_t aSize);
	void calculateSuperTrends(std::vector<candle>& aCandles, double aFactor);
}
