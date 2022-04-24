#include "tests/tests.h"
#include "calculation/singleCalculator.h"
#include "calculation/calculationSystem.h"
#include "utils/utils.h"
#include <chrono>
#include <iostream>

void calculate() {
	auto start = std::chrono::steady_clock::now();

	calculation::calculationSystem system;
	system.calculate();

	auto end = std::chrono::steady_clock::now();;
	std::cout << "CALCULATED TIME - [" << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << "] mins\n\a";
}

int main() {
	tests::runTests();

	//calculation::singleCalculation();

	calculate();

	return 0;
}