#include "singleCalculator.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "algorithm/superTrendMA/stMAlgorithm.h"
#include "market/candle.h"
#include "utils/utils.h"
#include <string>
#include <iostream>

using namespace calculation;

namespace {
	bool checkSettings(const Json& aSettings) {
		auto result = true;
		result &= aSettings.is_object();
		result &= aSettings.contains("algorithmType") && aSettings["algorithmType"].is_string();
		result &= aSettings.contains("ticker") && aSettings["ticker"].is_string();
		result &= aSettings.contains("timeframe") && aSettings["timeframe"].is_string();
		result &= aSettings.contains("data") && aSettings["data"].is_object();
		return result;
	}

	template<typename algorithmType>
	void singleCalculationInternal(const Json& aCandles, const Json& aData, market::eCandleInterval aTimeFrame) {
		algorithmType::algorithmDataType data;
		if (!data.initFromJson(aData)) {
			utils::logError("singleCalculation wrong algorithm data");
		}
		auto algorithm = algorithmType(data, aTimeFrame);
		algorithm.setWithLogs();
		auto result = algorithm.calculate(utils::parseCandles(aCandles));
		auto json = algorithm.getJsonData();
		utils::log("singleCalculation result - " + (result) ? "true" : "false");
		std::cout << json;
	}
}

void calculation::singleCalculation() {
	auto json = utils::readFromJson("input/singleCalculation");
	if (json.is_null()) {
		return;
	}
	auto log = [](const std::string& aStr) { utils::logError("calculation::singleCalculation " + aStr); };
	if (!checkSettings(json)) {
		log("bad json");
		return;
	}
	const auto timeframe = market::getCandleIntervalFromStr(json["timeframe"].get<std::string>());
	const auto ticker = json["ticker"].get<std::string>();
	const auto algorithmType = json["algorithmType"].get<std::string>();
	if (timeframe == market::eCandleInterval::NONE || ticker.empty()) {
		log("bad ticker/timeframe");
		return;
	}
	auto candles = utils::readFromJson("assets/candles/" + ticker + '/' + market::getCandleIntervalApiStr(timeframe));
	if (candles.is_null()) {
		log("bad candles");
		return;
	}
	if (!MARKET_DATA->loadTickerData(ticker)) {
		log("wrong ticker market json - " + ticker);
		return;
	}

	if (algorithmType == "superTrend") {
		singleCalculationInternal<algorithm::stAlgorithm>(candles, json["data"], timeframe);
	}
	if (algorithmType == "superTrendMA") {
		singleCalculationInternal<algorithm::stMAlgorithm>(candles, json["data"], timeframe);
	}
	else {
		log("wrong algorithm type");
	}
}