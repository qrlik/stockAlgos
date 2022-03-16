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
		data.maxLeverage = tier["maxLeverage"].get<int>();
		data.maintenanceMarginRate = tier["maintenanceMarginRate"].get<double>();
		data.maintenanceAmount = tier["maintenanceAmount"].get<int>();
		tiersData[tier["position"].get<int>()] = data;
	}
}

int marketData::getPrecision() const {
	return precision;
}

double marketData::getLiquidationPrice() const {
	return 0.0;
}