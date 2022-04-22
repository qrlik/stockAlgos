#include "tests/tests.h"
#include "calculation/singleCalculator.h"
#include "calculation/combinationCalculator.h"
#include "utils/utils.h"
#include <chrono>
#include <iostream>

void calculate(const std::string& aTicker, eCandleInterval aInterval) {
	auto start = std::chrono::steady_clock::now();

	calculation::calculationSystem system(aTicker, aInterval);
	system.calculate();

	auto end = std::chrono::steady_clock::now();;
	std::cout << "CALCULATED TIME - [" << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << "] mins\n\a";
}

int main() {
	tests::runTests();

	//calculation::singleCalculation();

	calculate("BTCUSDT", eCandleInterval::FIFTEEN_MIN);

	return 0;
}