#include "indicators/indicators.h"
#include "utils/utils.h"

int main() {
	auto json = utils::readFromJson("assets/candles/9760_1h");
	auto candles = utils::parseCandles(json);
	indicators::calculateRangeAtr(candles, indicators::eAtrType::RMA, 14);
	indicators::calculateSuperTrends(candles, 3.0);
	return 0;
}