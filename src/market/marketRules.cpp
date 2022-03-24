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
	auto tradingRules = utils::readFromJson("assets/market/BTCUSDT_data");
	quantityPrecision = tradingRules["quantityStep"].get<double>();
	pricePrecision = tradingRules["priceTick"].get<double>();
	minNotionalValue = tradingRules["minNotionalValue"].get<double>();
	for (auto& tier : tradingRules["brackets"]) {
		auto data = tierData{};
		data.notionalCap = tier["notionalCap"].get<int>();
		data.notionalFloor = tier["notionalFloor"].get<int>();
		data.maxLeverage = tier["maxLeverage"].get<int>();
		data.maintenanceMarginRate = tier["maintenanceMarginRate"].get<double>();
		data.maintenanceAmount = tier["maintenanceAmount"].get<double>();
		tiersData.push_back(data);
	}
	std::sort(tiersData.begin(), tiersData.end(), [](const tierData& aLhs, const tierData& aRhs) { return aLhs.notionalCap < aRhs.notionalCap; });
}

const std::vector<tierData>& marketData::getTiersData() const {
	return tiersData;
}

const tierData& marketData::getTierData(double aPosition) const {
	auto data = std::upper_bound(tiersData.begin(), tiersData.end(), aPosition,
								 [](auto aPosition, const tierData& aData) { return aPosition <= aData.notionalCap; });
	return *data;
}

double marketData::getLiquidationPrice(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) {
	auto liqPrice = -1.0;
	auto notional = -1.0;
	auto* tierData = &MARKET_DATA->getTierData(aNotional);

	while ((aLong || utils::isEqual(notional, -1.0)) ? notional < tierData->notionalFloor : (notional > tierData->notionalCap && tierData->maxLeverage != 1)) {
		if (notional > 0.0) {
			tierData = &MARKET_DATA->getTierData(notional);
		}

		const auto sign = (aLong) ? 1 : -1;
		const auto upper = aNotional / aLeverage + tierData->maintenanceAmount - sign * aQuantity * aPrice;
		const auto lower = aQuantity * tierData->maintenanceMarginRate - sign * aQuantity;
		liqPrice = std::abs(upper / lower);
		notional = liqPrice * aQuantity;
	}

	if (aLong) {
		return utils::ceil(liqPrice, MARKET_DATA->getPricePrecision());
	}
	return utils::floor(liqPrice, MARKET_DATA->getPricePrecision());
}

double marketData::getLiquidationPercent(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) {
	return getLiquidationPrice(aPrice, aNotional, aLeverage, aQuantity, aLong) / aPrice * 100 - 100;
}

std::pair<double, double> marketData::getLeverageLiquidationRange(int aLeverage) const {
	const double price = 50'000.0;
	const double minPosByQuantity = quantityPrecision * price;
	const double minPosbyNotional = getMinNotionalValue();
	const double minPos = std::max(minPosByQuantity, minPosbyNotional);
	const double maxPos = getLeverageMaxPosition(aLeverage);

	const double maxLiqPercent = getLiquidationPercent(price, minPos, aLeverage, utils::floor(minPos / price, quantityPrecision), false);
	const double minLiqPercent = getLiquidationPercent(price, maxPos, aLeverage, utils::floor(maxPos / price, quantityPrecision), false);
	return { minLiqPercent, maxLiqPercent };
}

double marketData::getLeverageMaxPosition(int aLeverage) const {
	auto tier = std::upper_bound(tiersData.rbegin(), tiersData.rend(), aLeverage, 
								 [](auto aLeverage, const tierData& aData) { return  aLeverage <= aData.maxLeverage; });
	return tier->notionalCap;
}

double marketData::getQuantityPrecision() const {
	return quantityPrecision;
}

double marketData::getPricePrecision() const {
	return pricePrecision;
}

double marketData::getMinNotionalValue() const {
	return minNotionalValue;
}