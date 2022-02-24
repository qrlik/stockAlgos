#pragma once
#include "../algorithm/moneyMaker.h"
#include <memory>

namespace tests {
	void checkAlgorithmData(const algorithmData& aData);
	void runTests();

	class mmChecker {
	public:
		mmChecker(std::string aName);
		void check();
	private:
		void updateTestMoneyMaker(const std::string& aTime);

		std::vector<candle> candles;
		Json testMoneyMakerData;
		std::unique_ptr<algorithm::moneyMaker> actualMoneyMaker;
		std::unique_ptr < algorithm::moneyMaker> testMoneyMaker;

		std::string name;
		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}