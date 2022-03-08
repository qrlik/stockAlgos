#include "tests/checkers.h"
#include "calculation/singleCalculator.h"
#include "calculation/combinationCalculator.h"
#include "utils/utils.h"

int main() {
	//tests::runTests();

	//calculation::singleCalculation();

	calculation::calculationSystem system(eCandleInterval::ONE_HOUR);
	system.calculate();

	return 0;
}