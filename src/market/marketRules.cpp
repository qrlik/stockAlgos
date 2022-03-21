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
	tradePrecision = tradingRules["minTradeAmount"].get<double>();
	pricePrecision = tradingRules["minPriceMovement"].get<double>();
	minNotionalValue = tradingRules["minNotionalValue"].get<double>();
	for (auto& tier : tradingRules["marginTiers"]) {
		auto data = tierData{};
		data.position = tier["position"].get<int>();
		data.maxLeverage = tier["maxLeverage"].get<int>();
		data.maintenanceMarginRate = tier["maintenanceMarginRate"].get<double>();
		data.maintenanceAmount = tier["maintenanceAmount"].get<int>();
		tiersData.push_back(data);
	}
	std::sort(tiersData.begin(), tiersData.end(), [](const tierData& aLhs, const tierData& aRhs) { return aLhs.position < aRhs.position; });
}

const std::vector<tierData>& marketData::getTiersData() const {
	return tiersData;
}

const tierData& marketData::getTierData(double aPosition) const {
	auto data = std::upper_bound(tiersData.begin(), tiersData.end(), aPosition,
								 [](auto aPosition, const tierData& aData) { return aPosition <= aData.position; });
	return *data;
}

double marketData::getMaximumLeveragePosition(int aLeverage) const {
	auto tier = std::upper_bound(tiersData.rbegin(), tiersData.rend(), aLeverage, 
								 [](auto aLeverage, const tierData& aData) { return  aLeverage <= aData.maxLeverage; });
	return tier->position;
}

double marketData::getTradePrecision() const {
	return tradePrecision;
}

double marketData::getPricePrecision() const {
	return pricePrecision;
}

double marketData::getMinNotionalValue() const {
	return minNotionalValue;
}