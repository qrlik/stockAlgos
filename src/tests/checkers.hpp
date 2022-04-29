#pragma once
#include "algorithm/algorithmBase.hpp"
#include "algorithm/superTrend/stAlgorithmData.h"
#include <memory>

namespace tests {
	template<typename AlgType,
		typename AlgDataType,
		typename = typename std::enable_if_t<std::is_base_of_v<algorithm::algorithmBase<AlgDataType>, AlgType>>>
	class algorithmChecker final {
	public:
		algorithmChecker(std::string aName) : name(std::move(aName)) {
			auto json = utils::readFromJson("assets/tests/" + name);
			AlgDataType data(json["algorithmData"]);
			if (!data.isValid()) {
				utils::logError("algorithmChecker - " + name + " invalid algorithm data");
				assert(false && "!data.isValid()");
			}
			actualAlgorithm = std::make_unique<AlgType>(data);
			testAlgorithm = std::make_unique<AlgType>(data);
			testAlgorithmData = json["testAlgorithmData"];
			testNextTime = testAlgorithmData[0]["time"].get<std::string>();

			auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
			candles = utils::parseCandles(jsonCandles);
			auto indicators = market::indicatorSystem(data.getAtrType(), data.getAtrSize(), data.getStFactor());
			indicators.getProcessedCandles(candles, json["candlesAmount"].get<int>());
		}
		void check() {
			for (const auto& candle : candles) {
				actualAlgorithm->doAction(candle);
				updateTestAlgorithm(candle.time);
				assert(*actualAlgorithm == *testAlgorithm);
				actualIndex += 1;
			}
			std::cout << "[OK] algorithmChecker - " + name + '\n';
		}
	private:
		void updateTestAlgorithm(const std::string& aTime) {
			if (aTime != testNextTime) {
				return;
			}
			Json data = testAlgorithmData[testIndex];
			testAlgorithm->initFromJson(data);
			testIndex += 1;
			testNextTime = (testIndex < static_cast<int>(testAlgorithmData.size())) ? testAlgorithmData[testIndex]["time"].get<std::string>() : "ENDED";
		}

		std::vector<candle> candles;
		Json testAlgorithmData;
		std::unique_ptr<AlgType> actualAlgorithm;
		std::unique_ptr<AlgType> testAlgorithm;

		std::string name;
		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}