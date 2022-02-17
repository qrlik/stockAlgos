#include "indicators/trueRange.h"
#include "utils/utils.h"

int main() {
	auto json = utils::readFromJson("assets/candles/9760_1h");
	auto candles = utils::parseCandles(json);
	indicators::calculateRangeAtr(candles, indicators::eAtrType::SMA, 14);
	return 0;
}