#include "checkers.h"
#include "../utils/utils.h"
#include <iostream>

using namespace tests;

void tests::checkAlgorithmData(const algorithmData& aData) {
	assert(aData.atrType != market::eAtrType::NONE);
	assert(aData.stFactor > 0.0);
	assert(aData.dealPercent > 0.0);
	assert(aData.leverage > 0 && aData.leverage <= 125);

	const auto liquidationPercent = 100.0 / aData.leverage;
	//assert(aData.stopLossPercent >= aData.activationPercent && aData.stopLossPercent <= liquidationPercent);
	//add check for liq % range
	assert(aData.minimumProfitPercent > 2 * algorithmData::tax);
}

mmChecker::mmChecker(std::string aName) :
	name(std::move(aName))
{
	auto json = utils::readFromJson("assets/tests/" + name);
	auto data = algorithmData::initAlgorithmDataFromJson(json["algorithmData"]);
	actualMoneyMaker = std::make_unique<algorithm::moneyMaker>(data, json["cash"].get<double>());
	testMoneyMaker = std::make_unique<algorithm::moneyMaker>(data, json["cash"].get<double>());
	actualMoneyMaker->setTest(true);
	testMoneyMaker->setTest(true);
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
	// add full cash check
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
