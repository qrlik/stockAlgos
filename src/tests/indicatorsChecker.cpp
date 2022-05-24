#include "indicatorsChecker.h"
#include "market/indicatorsSystem.h"
#include "utils/utils.h"

using namespace tests;

indicatorsChecker::indicatorsChecker() {
	auto json = utils::readFromJson("assets/tests/testIndicators");
	data.enableSuperTrend();
	data.enableMA(2);
	for (const auto& [field, value] : json["indicatorsData"].items()) {
		if (!data.initDataField(field, value)) {
			utils::logError("indicatorsChecker - " + field + " invalid indicators data field");
		}
	}
	if (!data.isValid()) {
		utils::logError("indicatorsChecker - invalid indicators data");
	}
	actualSystem = std::make_unique<market::indicatorsSystem>(data);
	testSystem = std::make_unique<market::indicatorsSystem>(data);
	testData = json["testIndicatorsData"];
	testNextTime = testData[0]["time"].get<std::string>();

	auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
	candles = utils::parseCandles(jsonCandles);
}

void indicatorsChecker::check() {
	for (auto& candle : candles) {
		actualSystem->processCandle(candle);
		updateTestSystem(candle.time);
		if (actualSystem->isInited()) {
			if (!(*actualSystem == *testSystem)) {
				utils::logError("[ERROR] indicatorsChecker");
				return;
			}
		}
		actualIndex += 1;
	}
	utils::log("[OK] indicatorsChecker");
}

void indicatorsChecker::updateTestSystem(const std::string& aTime) {
	if (aTime != testNextTime) {
		return;
	}
	Json stepData = testData[testIndex];
	testSystem->initFromJson(stepData);
	testIndex += 1;
	testNextTime = (testIndex < static_cast<int>(testData.size())) ? testData[testIndex]["time"].get<std::string>() : "ENDED";
}