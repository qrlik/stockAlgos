#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace market {
	class indicatorsSystem;
}

namespace tests {
	class indicatorsChecker final {
	public:
		indicatorsChecker();
		//void check() {
		//	for (auto& candle : candles) {
		//		actualAlgorithm->doAction(candle);
		//		updateTestAlgorithm(candle.time);
		//		assert(*actualAlgorithm == *testAlgorithm);
		//		actualIndex += 1;
		//	}
		//	utils::log("[OK] algorithmChecker - " + name);
		//}
	private:
		//void updateTestAlgorithm(const std::string& aTime) {
		//	if (aTime != testNextTime) {
		//		return;
		//	}
		//	Json data = testAlgorithmData[testIndex];
		//	testAlgorithm->initFromJson(data);
		//	testIndex += 1;
		//	testNextTime = (testIndex < static_cast<int>(testAlgorithmData.size())) ? testAlgorithmData[testIndex]["time"].get<std::string>() : "ENDED";
		//}

		std::vector<market::candle> candles;
		Json testData;
		std::unique_ptr<market::indicatorsSystem> actualSystem;
		std::unique_ptr<market::indicatorsSystem> testSystem;

		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}