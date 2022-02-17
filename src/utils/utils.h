#pragma once
#include "../../third_party/json.hpp"
#include "../simpleStructs/candle.h"

namespace utils {
	Json readFromJson(const std::string& aPath);
	std::vector<binance::candle> parseCandles(const Json& aJson);
}