#include "utils/utils.h"

int main() {
	auto json = utils::readFromJson("assets/candles/9760_1h");
	auto candles = utils::parseCandles(json);
	return 0;
}