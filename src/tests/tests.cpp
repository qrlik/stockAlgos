#include "tests.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "checkers.hpp"
#include "market/marketRules.h"
#include "utils/utils.h"

using namespace tests;

namespace {
	void mmCheckerTests() {
		auto checker1 = algorithmChecker<algorithm::stAlgorithm>("test1");
		checker1.check();
		auto checker2 = algorithmChecker<algorithm::stAlgorithm>("test2");
		checker2.check();
		auto checker3 = algorithmChecker<algorithm::stAlgorithm>("test3");
		checker3.check();
		auto checker4 = algorithmChecker<algorithm::stAlgorithm>("test4");
		checker4.check();
		utils::log("[OK] Money Maker Tests");
	}

	void marketDataTests() {
		auto* md = MARKET_DATA;
		const auto& tiersData = md->getTiersData();
		{ // getMaximumLeveragePosition
			assert(md->getLeverageMaxPosition(125) == tiersData[0].notionalCap);
			assert(md->getLeverageMaxPosition(124) == tiersData[0].notionalCap);
			assert(md->getLeverageMaxPosition(101) == tiersData[0].notionalCap);

			assert(md->getLeverageMaxPosition(100) == tiersData[1].notionalCap);
			assert(md->getLeverageMaxPosition(99) == tiersData[1].notionalCap);
			assert(md->getLeverageMaxPosition(51) == tiersData[1].notionalCap);

			assert(md->getLeverageMaxPosition(50) == tiersData[2].notionalCap);
			assert(md->getLeverageMaxPosition(49) == tiersData[2].notionalCap);
			assert(md->getLeverageMaxPosition(21) == tiersData[2].notionalCap);

			assert(md->getLeverageMaxPosition(20) == tiersData[3].notionalCap);
			assert(md->getLeverageMaxPosition(19) == tiersData[3].notionalCap);
			assert(md->getLeverageMaxPosition(11) == tiersData[3].notionalCap);

			assert(md->getLeverageMaxPosition(10) == tiersData[4].notionalCap);
			assert(md->getLeverageMaxPosition(9) == tiersData[4].notionalCap);
			assert(md->getLeverageMaxPosition(6) == tiersData[4].notionalCap);

			assert(md->getLeverageMaxPosition(5) == tiersData[5].notionalCap);
			assert(md->getLeverageMaxPosition(4) == tiersData[6].notionalCap);
			assert(md->getLeverageMaxPosition(3) == tiersData[7].notionalCap);
			assert(md->getLeverageMaxPosition(2) == tiersData[8].notionalCap);
			assert(md->getLeverageMaxPosition(1) == tiersData[9].notionalCap);
		}
		{ // getTierData
			for (size_t i = 0, size = tiersData.size(); i < size; ++i) {
				if (i >= 1) {
					assert(md->getTierData(tiersData[i - 1].notionalCap + 0.1).maxLeverage == tiersData[i].maxLeverage);
				}
				else {
					assert(md->getTierData(tiersData[i].notionalCap / 10).maxLeverage == tiersData[i].maxLeverage);
				}
				assert(md->getTierData(tiersData[i].notionalCap).maxLeverage == tiersData[i].maxLeverage);
			}
		}
		{ // getLiquidationPrice
			const auto aPrice = 50'000;
			//longs
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 125, 0.001, true), 49'799.2));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50'000, 125, 1, true), 49'799.2));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 100, 0.001, true), 49'698.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 250'000, 100, 5, true), 49'738.7));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 50, 0.001, true), 49'196.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 1'000'000, 50, 20, true), 49'429.3));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 20, 0.001, true), 47'690.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 7'500'000, 20, 150, true), 48'606.5));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 10, 0.001, true), 45'180.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 40'000'000, 10, 800, true), 47'100.3));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 5, 0.001, true), 40'160.7));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 100'000'000, 5, 2000, true), 43'220.2));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 4, 0.001, true), 37'650.7));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 200'000'000, 4, 4000, true), 41'513.2));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 51, 3, 0.001, true), 33'132.6));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 400'000'000, 3, 8000, true), 37'788.7));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 2, 0.001, true), 25'100.5));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 600'000'000, 2, 12000, true), 28'460.5));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 1, 0.001, true), 0));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 1'000'000'000, 1, 20000, true), 0));

			//shorts
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 125, 0.001, false), 50'199.2));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50'000, 125, 1, false), 50'199.0));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 100, 0.001, false), 50'298.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 250'000, 100, 5, false), 50'257.4));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 50, 0.001, false), 50'796.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 1'000'000, 50, 20, false), 50'551.2));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 20, 0.001, false), 52'290.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 7'500'000, 20, 150, false), 51'293.9));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 10, 0.001, false), 54'780.8));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 40'000'000, 10, 800, false), 52'504.3));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 5, 0.001, false), 59'760.9));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 100'000'000, 5, 2000, false), 55'423.9));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 4, 0.001, false), 62'250.9));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 200'000'000, 4, 4000, false), 56'457.3));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 51, 3, 0.001, false), 66'733.0));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 400'000'000, 3, 8000, false), 58'303.7));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 2, 0.001, false), 74'701.1));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 600'000'000, 2, 12000, false), 61'094.6));

			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 50, 1, 0.001, false), 99'601.5));
			assert(utils::isEqual(md->getLiquidationPrice(aPrice, 1'000'000'000, 1, 20000, false), 73'323.4));
		}
		utils::log("[OK] Market Data Tests");
	}
}

void tests::runTests() {
#ifdef NDEBUG
	return;
#endif
	if (!MARKET_DATA->loadTickerData("BTCUSDT")) {
		utils::logError("tests::runTests wrong ticker json for tests");
		return;
	}
	marketDataTests();
	mmCheckerTests();
}
