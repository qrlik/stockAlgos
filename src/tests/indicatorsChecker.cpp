#include "indicatorsChecker.h"
#include "market/indicatorsData.h"
#include "market/indicatorsSystem.h"
#include "utils/utils.h"

using namespace tests;

indicatorsChecker::indicatorsChecker() {
	auto json = utils::readFromJson("assets/tests/testIndicators");
	market::indicatorsData data;
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
	//testData = json["testIndicatorsData"];
	//testNextTime = testData[0]["time"].get<std::string>();

	//auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
	//candles = utils::parseCandles(jsonCandles);
}