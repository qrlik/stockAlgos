#include "marketRules.h"
#include "../utils/utils.h"

using namespace market;

marketData* marketData::instance = new marketData();

marketData* marketData::getInstance() {
	return instance;
}

marketData::marketData() {
	init();
}

void marketData::init() {
	auto tradingRules = utils::readFromJson("assets/market/tradingRules");
	precision = tradingRules["precision"].get<int>();
	for (auto& tier : tradingRules["marginTiers"]) {
		auto data = tierData{};
		data.position = tier["position"].get<int>();
		data.maxLeverage = tier["maxLeverage"].get<int>();
		data.maintenanceMarginRate = tier["maintenanceMarginRate"].get<double>();
		data.maintenanceAmount = tier["maintenanceAmount"].get<int>();
		tiersData.push_back(data);
	}
	std::sort(tiersData.begin(), tiersData.end(), [](const tierData& aLhs, const tierData& aRhs) { return aLhs.position < aRhs.position; });
	runTests();
}

int marketData::getPrecision() const {
	return precision;
}

double marketData::getLiquidationPrice() const {
	return 0.0;
}

double marketData::getMaximumLeveragePosition(int aLeverage) const {
	auto tier = std::upper_bound(tiersData.rbegin(), tiersData.rend(), aLeverage, 
								 [](auto aLeverage, const tierData& aData) {
									 return  aLeverage <= aData.maxLeverage;
								 });
	if (tier != tiersData.rend()) {
		return tier->position;
	}
	return 0.0;
}

void marketData::runTests() {
	assert(getMaximumLeveragePosition(125) == tiersData[0].position);
	assert(getMaximumLeveragePosition(124) == tiersData[0].position);
	assert(getMaximumLeveragePosition(101) == tiersData[0].position);

	assert(getMaximumLeveragePosition(100) == tiersData[1].position);
	assert(getMaximumLeveragePosition(99) == tiersData[1].position);
	assert(getMaximumLeveragePosition(51) == tiersData[1].position);

	assert(getMaximumLeveragePosition(50) == tiersData[2].position);
	assert(getMaximumLeveragePosition(49) == tiersData[2].position);
	assert(getMaximumLeveragePosition(21) == tiersData[2].position);

	assert(getMaximumLeveragePosition(20) == tiersData[3].position);
	assert(getMaximumLeveragePosition(19) == tiersData[3].position);
	assert(getMaximumLeveragePosition(11) == tiersData[3].position);

	assert(getMaximumLeveragePosition(10) == tiersData[4].position);
	assert(getMaximumLeveragePosition(9) == tiersData[4].position);
	assert(getMaximumLeveragePosition(6) == tiersData[4].position);

	assert(getMaximumLeveragePosition(5) == tiersData[5].position);
	assert(getMaximumLeveragePosition(4) == tiersData[6].position);
	assert(getMaximumLeveragePosition(3) == tiersData[7].position);
	assert(getMaximumLeveragePosition(2) == tiersData[8].position);
	assert(getMaximumLeveragePosition(1) == tiersData[9].position);
}