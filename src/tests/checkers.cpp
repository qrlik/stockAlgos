#include "checkers.h"
#include "structs/statistic.h"
#include "utils/utils.h"
#include "market/marketRules.h"
#include <iostream>

using namespace tests;

void tests::checkAlgorithmData(const algorithm::stAlgorithmData& aData) {
	bool result = true;
	result &= aData.getAtrType() != market::eAtrType::NONE;
	result &= aData.getAtrSize() > 0;
	result &= aData.getStFactor() > 0.0;

	result &= aData.getDealPercent() > 0.0;
	result &= aData.getLeverage() > 0 && aData.getLeverage() <= 125;

	result &= aData.getStartCash() > market::marketData::getInstance()->getMinNotionalValue() / aData.getLeverage();
	result &= aData.getStartCash() > aData.getMaxLossCash();

	const auto minLiqPercent = (aData.getOrderSize() > 0.0)
		? MARKET_DATA->getLiquidationPercent(aData.getOrderSize(), aData.getLeverage())
		: MARKET_DATA->getLeverageLiquidationRange(aData.getLeverage()).first;
	result &= aData.getLiquidationOffsetPercent() < minLiqPercent;
	result &= aData.getMinimumProfitPercent() > 2 * algorithm::stAlgorithmData::tax * 100.0;

	result &= (utils::isEqual(aData.getDynamicSLPercent(), -1.0) && aData.getDynamicSLTrendMode()) || aData.getDynamicSLPercent() > 0.0;

	auto waiter = aData.getTouchOpenerActivationWaitMode();
	if (aData.getBreakOpenerEnabled()) {
		waiter &= aData.getBreakOpenerActivationWaitMode();
	}
	result &= (waiter) ? aData.getActivationWaiterRange() >= 0 : aData.getActivationWaiterRange() == -1;
	result &= (aData.getStopLossWaiterEnabled()) ? aData.getStopLossWaiterRange() >= 0 : aData.getStopLossWaiterRange() == -1;

	if (!result) {
		assert("tests::checkAlgorithmData fail" && result);
		utils::logError("tests::checkAlgorithmData fail");
		std::cout << aData.toJson() << '\n';
	}
}

mmChecker::mmChecker(std::string aName) :
	name(std::move(aName))
{
	auto json = utils::readFromJson("assets/tests/" + name);
	algorithm::stAlgorithmData data(json["algorithmData"]);
	tests::checkAlgorithmData(data);
	actualMoneyMaker = std::make_unique<algorithm::stAlgorithm>(data);
	testMoneyMaker = std::make_unique<algorithm::stAlgorithm>(data);
	testMoneyMakerData = json["testMoneyMakerData"];
	testNextTime = testMoneyMakerData[0]["time"].get<std::string>();

	auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
	candles = utils::parseCandles(jsonCandles);
	auto indicators = market::indicatorSystem(data.getAtrType(), data.getAtrSize(), data.getStFactor());
	indicators.getProcessedCandles(candles, json["candlesAmount"].get<int>());
}

void mmChecker::check() {
	for (const auto& candle : candles) {
		actualMoneyMaker->doAction(candle);
		updateTestMoneyMaker(candle.time);
		assert(*actualMoneyMaker == *testMoneyMaker);
		actualIndex += 1;
	}
	if (testMoneyMaker->getData().getFullCheck()) {
		assert(utils::isEqual(actualMoneyMaker->getFullCash(), testMoneyMaker->getFullCash(), market::marketData::getInstance()->getQuotePrecision()));
	}
	std::cout << "[OK] mmChecker - " + name + '\n';
}

void mmChecker::updateTestMoneyMaker(const std::string& aTime) {
	if (aTime != testNextTime) {
		return;
	}
	Json data = testMoneyMakerData[testIndex];
	data.erase("time");
	for (const auto& [key, value] : data.items()) {
		if (key == "activationWaitCounter") {
			testMoneyMaker->getActivationWaiter().setCounter(value.get<int>());
		}
		else if (key == "stopLossWaitCounter") {
			testMoneyMaker->getStopLossWaiter().setCounter(value.get<int>());
		}
		else if (key == "order") {
			auto& order = testMoneyMaker->getOrder();
			if (value.contains("lifeState")) {
				value.erase("lifeState");
				order.reset();
			}
			value.erase("type");
			orderData::initOrderData(testMoneyMaker->getData(), order, value);
		}
		else if (key == "stats") {
			statistic::initStatisticFromJson(testMoneyMaker->stats, value);
		}
		else if (key == "lastUpSuperTrend") {
			testMoneyMaker->lastUpSuperTrend = value.get<double>();
		}
		else if (key == "lastDownSuperTrend") {
			testMoneyMaker->lastDownSuperTrend = value.get<double>();
		}
		else if (key == "isTrendUp") {
			testMoneyMaker->isTrendUp = value.get<bool>();
		}
		else if (key == "isNewTrend") {
			testMoneyMaker->isNewTrend = value.get<bool>();
		}
		else if (key == "state") {
			testMoneyMaker->state = algorithm::stAlgorithm::stateFromString(value.get<std::string>());
		}
		else if (key == "cash") {
			testMoneyMaker->cash = value.get<double>();
		}
	}
	testIndex += 1;
	testNextTime = (testIndex < static_cast<int>(testMoneyMakerData.size())) ? testMoneyMakerData[testIndex]["time"].get<std::string>() : "ENDED";
}
