#include "tests/checkers.h"
#include "calculation/combinationCalculator.h"
#include "utils/utils.h"

int main() {
	//tests::runTests();

	calculation::calculationSystem system;
	system.calculate();

	//auto json = utils::readFromJson("finalDataSync");
	//auto jsonAsync = utils::readFromJson("finalDataAsync");
	//bool isEqual = json == jsonAsync;
	return 0;
}