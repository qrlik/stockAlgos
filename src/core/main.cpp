#include "tests/tests.h"
#include "calculation/singleCalculator.h"
#include "calculation/calculationSystem.h"
#include "utils/utils.h"
#include <chrono>

void calculate() {
	auto start = std::chrono::steady_clock::now();

	calculation::calculationSystem system;
	system.calculate();

	auto end = std::chrono::steady_clock::now();
	utils::log("CALCULATED TIME - [" + std::to_string(std::chrono::duration_cast<std::chrono::minutes>(end - start).count()) + "] mins\a");
}

int main() {
	tests::runTests();

	calculate();
	//calculation::singleCalculation();

	return 0;
}