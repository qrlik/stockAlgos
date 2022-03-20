#include "tests.h"
#include "checkers.h"
#include "../market/marketRules.h"
#include <iostream>

using namespace tests;

namespace {
	void mmCheckerTests() {
		auto checker1 = mmChecker("test1");
		checker1.check();
		auto checker2 = mmChecker("test2");
		checker2.check();
		auto checker3 = mmChecker("test3");
		checker3.check();
		auto checker4 = mmChecker("test4");
		checker4.check();
		std::cout << "[OK] Money Maker Tests\n";
	}

	void marketDataTests() {
		auto* md = market::marketData::getInstance();
		const auto& tiersData = md->getTiersData();
		{ // getMaximumLeveragePosition
			assert(md->getMaximumLeveragePosition(125) == tiersData[0].position);
			assert(md->getMaximumLeveragePosition(124) == tiersData[0].position);
			assert(md->getMaximumLeveragePosition(101) == tiersData[0].position);

			assert(md->getMaximumLeveragePosition(100) == tiersData[1].position);
			assert(md->getMaximumLeveragePosition(99) == tiersData[1].position);
			assert(md->getMaximumLeveragePosition(51) == tiersData[1].position);

			assert(md->getMaximumLeveragePosition(50) == tiersData[2].position);
			assert(md->getMaximumLeveragePosition(49) == tiersData[2].position);
			assert(md->getMaximumLeveragePosition(21) == tiersData[2].position);

			assert(md->getMaximumLeveragePosition(20) == tiersData[3].position);
			assert(md->getMaximumLeveragePosition(19) == tiersData[3].position);
			assert(md->getMaximumLeveragePosition(11) == tiersData[3].position);

			assert(md->getMaximumLeveragePosition(10) == tiersData[4].position);
			assert(md->getMaximumLeveragePosition(9) == tiersData[4].position);
			assert(md->getMaximumLeveragePosition(6) == tiersData[4].position);

			assert(md->getMaximumLeveragePosition(5) == tiersData[5].position);
			assert(md->getMaximumLeveragePosition(4) == tiersData[6].position);
			assert(md->getMaximumLeveragePosition(3) == tiersData[7].position);
			assert(md->getMaximumLeveragePosition(2) == tiersData[8].position);
			assert(md->getMaximumLeveragePosition(1) == tiersData[9].position);
		}
		{ // getTierData
			for (size_t i = 0, size = tiersData.size(); i < size; ++i) {
				if (i >= 1) {
					assert(md->getTierData(tiersData[i - 1].position + 0.1).maxLeverage == tiersData[i].maxLeverage);
				}
				else {
					assert(md->getTierData(tiersData[i].position / 10).maxLeverage == tiersData[i].maxLeverage);
				}
				assert(md->getTierData(tiersData[i].position).maxLeverage == tiersData[i].maxLeverage);
			}
		}
		std::cout << "[OK] Market Data Tests\n";
	}
}

void tests::runTests() {
	marketDataTests();
	//mmCheckerTests();
}
