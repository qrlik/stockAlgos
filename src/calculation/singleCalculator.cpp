#include "singleCalculator.h"
#include "algorithm/superTrend/stAlgorithm.h"
#include "market/candle.h"
#include "utils/utils.h"
#include <string>

using namespace calculation;

namespace {
	bool checkSettings(const Json& aSettings) {
		auto result = true;
		result &= aSettings.is_object();
		result &= aSettings.contains("algorithmType") && aSettings["algorithmType"].is_string();
		result &= aSettings.contains("ticker") && aSettings["ticker"].is_string();
		result &= aSettings.contains("timeframe") && aSettings["timeframe"].is_string();
		result &= aSettings.contains("data") && aSettings["data"].is_object();
		return true;
	}

	template<typename algorithmType>
	void singleCalculationInternal(const Json& aCandles, const Json& aData) {
		algorithmType::algorithmDataType data;
		if (!data.initFromJson(aData)) {
			utils::logError("singleCalculation wrong algorithm data");
		}
		auto candles = utils::parseCandles(aCandles);
		auto indicators = market::indicatorSystem(data.getAtrType(), data.getAtrSize(), data.getStFactor());
		auto finalSize = static_cast<int>(candles.size()) - 2200; // TO DO FIX THIS
		if (finalSize <= 0) {
			utils::logError("singleCalculation atr size for candles amount");
			finalSize = static_cast<int>(candles.size());
		}
		indicators.getProcessedCandles(candles, finalSize);

		auto algorithm = algorithmType(data);
		algorithm.setWithLogs(true);
		algorithm.calculate(candles);
		utils::log("singleCalculation full cash - " + std::to_string(algorithm.getFullCash()));
	}
}

void calculation::singleCalculation() {
	auto json = utils::readFromJson("singleCalculation");
	auto log = []() { utils::logError("calculation::singleCalculation wrong algorithm type"); };
	if (!checkSettings(json)) {
		log();
		return;
	}
	const auto timeframe = market::getCandleIntervalFromStr(json["timeframe"].get<std::string>());
	const auto ticker = json["ticker"].get<std::string>();
	const auto algorithmType = json["algorithmType"].get<std::string>();
	if (timeframe == market::eCandleInterval::NONE || ticker.empty()) {
		log();
		return;
	}
	auto candles = utils::readFromJson("assets/candles/" + ticker + '/' + market::getCandleIntervalApiStr(timeframe));
	if (candles.is_null()) {
		log();
		return;
	}
	if (algorithmType == "superTrend") {
		singleCalculationInternal<algorithm::stAlgorithm>(candles, json["data"]);
	}
	else {
		log();
	}
}