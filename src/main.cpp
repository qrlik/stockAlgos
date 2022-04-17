#include "tests/tests.h"
#include "calculation/singleCalculator.h"
#include "calculation/combinationCalculator.h"
#include "utils/utils.h"
#include <chrono>
#include <iostream>

int main() {
	tests::runTests();

	//calculation::singleCalculation();

	auto start = std::chrono::steady_clock::now();

	calculation::calculationSystem system("BTCUSDT", eCandleInterval::TWO_HOUR);
	system.calculate();

	auto end = std::chrono::steady_clock::now();;
	std::cout << "CALCULATED TIME - [" << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << "] mins\n\a";

	return 0;
}