#pragma once

namespace tests {
	template<typename algorithmType>
	class algorithmChecker final {
	public:
		algorithmChecker(std::string aName) : name(std::move(aName)) {
			auto json = utils::readFromJson("assets/tests/" + name);
			algorithmType::algorithmDataType data(json["algorithmData"]);
			if (!data.isValid()) {
				utils::logError("algorithmChecker - " + name + " invalid algorithm data");
				assert(false && "!data.isValid()");
			}
			actualAlgorithm = std::make_unique<algorithmType>(data);
			testAlgorithm = std::make_unique<algorithmType>(data);
			testAlgorithmData = json["testAlgorithmData"];
			testNextTime = testAlgorithmData[0]["time"].get<std::string>();

			auto jsonCandles = utils::readFromJson("assets/tests/" + json["candlesFileName"].get<std::string>());
			candles = utils::parseCandles(jsonCandles);
			auto indicators = market::indicatorsSystem(const_cast<const algorithmType::algorithmDataType&>(data).getIndicatorsData());
			indicators.getProcessedCandles(candles, json["candlesAmount"].get<int>());
		}
		void check() {
			for (const auto& candle : candles) {
				actualAlgorithm->doAction(candle);
				updateTestAlgorithm(candle.time);
				assert(*actualAlgorithm == *testAlgorithm);
				actualIndex += 1;
			}
			utils::log("[OK] algorithmChecker - " + name);
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

		std::vector<market::candle> candles;
		Json testAlgorithmData;
		std::unique_ptr<algorithmType> actualAlgorithm;
		std::unique_ptr<algorithmType> testAlgorithm;

		std::string name;
		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}