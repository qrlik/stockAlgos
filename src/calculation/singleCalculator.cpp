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
		return result;
	}

	template<typename algorithmType>
	void singleCalculationInternal(const Json& aCandles, const Json& aData) {
		algorithmType::algorithmDataType data;
		if (!data.initFromJson(aData)) {
			utils::logError("singleCalculation wrong algorithm data");
		}
		auto candles = utils::parseCandles(aCandles);
		auto indicators = market::indicatorSystem(const_cast<const algorithmType::algorithmDataType&>(data).getIndicatorsData());
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
	if (algorithmType == "superTrend") {
		singleCalculationInternal<algorithm::stAlgorithm>(candles, json["data"]);
	}
	else {
		log("wrong algorithm type");
	}
}