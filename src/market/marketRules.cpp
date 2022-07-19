#include "marketRules.h"
#include "utils/utils.h"

using namespace market;

namespace {
	const std::string settingsPath = "assets/market/";
}

marketData* marketData::instance = new marketData();

marketData* marketData::getInstance() {
	return instance;
}

marketData::marketData() {
	loadExchangeSettings();
}

namespace {
	bool checkExchangeJson(const Json& aData) {
		auto result = true;
		result &= aData.is_object();
		result &= aData.contains("tax") && aData["tax"].is_number_float();
		if (result) {
			const auto tax = aData["tax"].get<double>();
			result &= !utils::isEqual(tax, 0.0) && utils::isGreater(tax, 0.0);
		}
		return result;
	}
}

void marketData::loadExchangeSettings() {
	auto exchageSettings = utils::readFromJson(settingsPath + "binance");
	if (!checkExchangeJson(exchageSettings)) {
		utils::logError("marketData::loadExchangeSettings wrong exchange json");
	}
	tax = exchageSettings["tax"].get<double>();
}

namespace {
	bool checkTickerJson(const Json& aData) {
		auto result = true;
		result &= aData.is_object();
		result &= aData.contains("brackets") && aData["brackets"].is_array();
		if (result) {
			for (const auto& bracket : aData["brackets"]) {
				result &= bracket.is_object();
				result &= bracket.contains("maxLeverage") && bracket["maxLeverage"].is_number_integer() && bracket["maxLeverage"].get<int>() > 0;
				result &= bracket.contains("notionalCap") && bracket["notionalCap"].is_number_integer() && bracket["notionalCap"].get<int>() > 0;
				result &= bracket.contains("notionalFloor") && bracket["notionalFloor"].is_number_integer() && bracket["notionalFloor"].get<int>() >= 0;
				result &= bracket.contains("maintenanceMarginRate") && bracket["maintenanceMarginRate"].is_number_float()
					&& utils::isGreater(bracket["maintenanceMarginRate"].get<double>(), 0.0);
				result &= bracket.contains("maintenanceAmount") && bracket["maintenanceAmount"].is_number_float()
					&& utils::isGreaterOrEqual(bracket["maintenanceAmount"].get<double>(), 0.0);
			}
		}
		result &= aData.contains("quotePrecision") && aData["quotePrecision"].is_number_integer() && utils::isGreater(aData["quotePrecision"].get<int>(), 0);
		result &= aData.contains("priceTick") && aData["priceTick"].is_number() && utils::isGreater(aData["priceTick"].get<double>(), 0.0);
		result &= aData.contains("quantityStep") && aData["quantityStep"].is_number() && utils::isGreater(aData["quantityStep"].get<double>(), 0.0);
		result &= aData.contains("minNotionalValue") && aData["minNotionalValue"].is_number() && utils::isGreater(aData["minNotionalValue"].get<double>(), 0.0);
		return result;
	}
}

bool marketData::loadTickerData(const std::string& aTicker) {
	if (currentTicker == aTicker) {
		return true;
	}
	if (data.count(aTicker) > 0) {
		currentTicker = aTicker;
		return true;
	}
	auto tickerJson = utils::readFromJson(settingsPath + "tickets/" +aTicker);
	if (!checkTickerJson(tickerJson)) {
		utils::logError("marketData::loadTickerData wrong ticker json");
		return false;
	}
	currentTicker = aTicker;
	auto& tickerData = data[aTicker];
	tickerData.quantityPrecision = tickerJson["quantityStep"].get<double>();
	tickerData.pricePrecision = tickerJson["priceTick"].get<double>();
	tickerData.quotePrecision = 1.0 / std::pow(10, tickerJson["quotePrecision"].get<int>());
	tickerData.minNotionalValue = tickerJson["minNotionalValue"].get<double>();
	for (auto& tier : tickerJson["brackets"]) {
		auto tData = tierData{};
		tData.notionalCap = tier["notionalCap"].get<int>();
		tData.notionalFloor = tier["notionalFloor"].get<int>();
		tData.maxLeverage = tier["maxLeverage"].get<int>();
		tData.maintenanceMarginRate = tier["maintenanceMarginRate"].get<double>();
		tData.maintenanceAmount = tier["maintenanceAmount"].get<double>();
		tickerData.tiersData.push_back(tData);
	}
	std::sort(tickerData.tiersData.begin(), tickerData.tiersData.end(), [](const tierData& aLhs, const tierData& aRhs) { return utils::isLess(aLhs.notionalCap, aRhs.notionalCap); });
	return true;
}

const std::vector<tierData>& marketData::getTiersData() const {
	return data.at(currentTicker).tiersData;
}

const tierData& marketData::getTierData(double aPosition) const {
	auto tier = std::upper_bound(data.at(currentTicker).tiersData.begin(), data.at(currentTicker).tiersData.end(), aPosition,
								 [](auto aPosition, const tierData& aData) { return utils::isLessOrEqual(aPosition, aData.notionalCap); });
	return *tier;
}

double marketData::getLiquidationPrice(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const {
	auto liqPrice = -1.0;
	auto notional = -1.0;
	auto* tierData = &getTierData(aNotional);

	while ((aLong || utils::isEqual(notional, -1.0))
		? utils::isLess(notional, tierData->notionalFloor)
		: (utils::isGreater(notional, tierData->notionalCap) && tierData->maxLeverage != 1))
	{
		if (utils::isGreater(notional, 0.0)) {
			tierData = &getTierData(notional);
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

double marketData::getLiquidationPercent(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const {
	return getLiquidationPrice(aPrice, aNotional, aLeverage, aQuantity, aLong) / aPrice * 100 - 100;
}

double marketData::getLiquidationPercent(double aMargin, int aLeverage) const {
	const auto price = 25000.0;
	auto pos = aMargin * aLeverage;
	const auto maxPos = getLeverageMaxPosition(aLeverage);
	pos = utils::minFloat(pos, maxPos);

	return getLiquidationPercent(price, pos, aLeverage, utils::floor(pos / price, data.at(currentTicker).quantityPrecision), false);
}

std::pair<double, double> marketData::getLeverageLiquidationRange(int aLeverage) const {
	const double price = 25000.0;
	const double minPosByQuantity = data.at(currentTicker).quantityPrecision * price;
	const double minPosbyNotional = getMinNotionalValue();
	const double minPos = utils::maxFloat(minPosByQuantity, minPosbyNotional);
	const double maxPos = getLeverageMaxPosition(aLeverage);
	if (utils::isGreater(minPos, maxPos)) {
		utils::logError("marketData::getLeverageLiquidationRange wrong");
	}

	const double maxLiqPercent = getLiquidationPercent(price, minPos, aLeverage, utils::floor(minPos / price, data.at(currentTicker).quantityPrecision), false);
	const double minLiqPercent = getLiquidationPercent(price, maxPos, aLeverage, utils::floor(maxPos / price, data.at(currentTicker).quantityPrecision), false);
	return { minLiqPercent, maxLiqPercent };
}

double marketData::getLeverageMaxPosition(int aLeverage) const {
	auto tier = std::upper_bound(data.at(currentTicker).tiersData.rbegin(), data.at(currentTicker).tiersData.rend(), aLeverage,
								 [](auto aLeverage, const tierData& aData) { return  aLeverage <= aData.maxLeverage; });
	return tier->notionalCap;
}

double marketData::getQuantityPrecision() const {
	return data.at(currentTicker).quantityPrecision;
}

double marketData::getPricePrecision() const {
	return data.at(currentTicker).pricePrecision;
}

double marketData::getQuotePrecision() const {
	return data.at(currentTicker).quotePrecision;
}

double marketData::getMinNotionalValue() const {
	return data.at(currentTicker).minNotionalValue;
}

int marketData::getMaxLeverage() const {
	const auto& tiers = data.at(currentTicker).tiersData;
	if (tiers.empty()) {
		return 0;
	}
	return tiers.front().maxLeverage;
}

double marketData::getTaxFactor() const {
	return tax / 100.0;
}
