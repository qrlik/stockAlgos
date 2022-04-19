#include "checkers.h"
#include "../structs/statistic.h"
#include "../utils/utils.h"
#include "../market/marketRules.h"
#include <iostream>

using namespace tests;

void tests::checkAlgorithmData(const algorithmData& aData) {
	bool result = true;
	result &= aData.atrType != market::eAtrType::NONE;
	result &= aData.atrSize > 0;
	result &= aData.stFactor > 0.0;

	result &= aData.dealPercent > 0.0;
	result &= aData.leverage > 0 && aData.leverage <= 125;

	result &= aData.startCash > market::marketData::getInstance()->getMinNotionalValue() / aData.leverage;
	result &= aData.startCash > aData.maxLossCash;

	const auto minLiqPercent = (aData.orderSize > 0.0)
		? market::marketData::getLiquidationPercent(aData.orderSize, aData.leverage) 
		: market::marketData::getInstance()->getLeverageLiquidationRange(aData.leverage).first;
	result &= aData.liquidationOffsetPercent < minLiqPercent;
	result &= aData.minimumProfitPercent > 2 * algorithmData::tax * 100.0;

	result &= (utils::isEqual(aData.dynamicSLPercent, -1.0) && aData.dynamicSLTrendMode) || aData.dynamicSLPercent > 0.0;

	auto waiter = aData.touchOpenerActivationWaitMode;
	if (aData.breakOpenerEnabled) {
		waiter &= aData.breakOpenerActivationWaitMode;
	}
	if (waiter) {
		result &= aData.activationWaiterRange >= 0;
	}
	if (aData.stopLossWaiterEnabled) {
		result &= aData.stopLossWaiterRange >= 0;
	}

	if (!result) {
		assert("tests::checkAlgorithmData fail" && result);
		std::cout << "[ERROR] tests::checkAlgorithmData fail\n";
		std::cout << aData.toJson() << '\n';
	}
}

mmChecker::mmChecker(std::string aName) :
	name(std::move(aName))
{
	auto json = utils::readFromJson("assets/tests/" + name);
	auto data = algorithmData::initAlgorithmDataFromJson(json["algorithmData"]);
	tests::checkAlgorithmData(data);
	actualMoneyMaker = std::make_unique<algorithm::moneyMaker>(data);
	testMoneyMaker = std::make_unique<algorithm::moneyMaker>(data);
	testMoneyMakerData = json["testMoneyMakerData"];
	testNextTime = testMoneyMakerData[0]["time"].get<std::string>();

	auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
	candles = utils::parseCandles(jsonCandles);
	auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
	indicators.getProcessedCandles(candles, json["candlesAmount"].get<int>());
}

void mmChecker::check() {
	for (const auto& candle : candles) {
		actualMoneyMaker->doAction(candle);
		updateTestMoneyMaker(candle.time);
		assert(*actualMoneyMaker == *testMoneyMaker);
		actualIndex += 1;
	}
	if (testMoneyMaker->getFullCheck()) {
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
			orderData::initOrderDataFromJson(order, value);
			order.setFullCheck(testMoneyMaker->getFullCheck());
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
			testMoneyMaker->state = algorithm::moneyMaker::stateFromString(value.get<std::string>());
		}
		else if (key == "cash") {
			testMoneyMaker->cash = value.get<double>();
		}
	}
	testIndex += 1;
	testNextTime = (testIndex < static_cast<int>(testMoneyMakerData.size())) ? testMoneyMakerData[testIndex]["time"].get<std::string>() : "ENDED";
}
