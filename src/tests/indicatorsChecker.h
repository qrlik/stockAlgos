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
		void check();
	private:
		void updateTestSystem(const std::string& aTime);

		std::vector<market::candle> candles;
		Json testData;
		std::unique_ptr<market::indicatorsSystem> actualSystem;
		std::unique_ptr<market::indicatorsSystem> testSystem;

		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}