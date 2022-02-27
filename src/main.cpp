#include "tests/checkers.h"
#include "calculation/combinationCalculator.h"
#include "utils/utils.h"

int main() {
	//tests::runTests();
	
	//calculation::calculate();

	auto str1 = utils::getStringFromFile("finalDataAsync");
	auto str2 = utils::getStringFromFile("finalDataSync");

	for (size_t i = 0, size = str1.size(); i < size; ++i) {
		auto char1 = str1[i];
		auto char2 = str2[i];
		if (char1 != char2) {
			auto test = 5;
		}
	}

	return 0;
}