#pragma once
#include "algorithm/superTrend/stAlgorithm.h"
#include <memory>

namespace tests {
	void checkAlgorithmData(const stAlgorithmData& aData);

	class mmChecker {
	public:
		mmChecker(std::string aName);
		void check();
	private:
		void updateTestMoneyMaker(const std::string& aTime);

		std::vector<candle> candles;
		Json testMoneyMakerData;
		std::unique_ptr<algorithm::stAlgorithm> actualMoneyMaker;
		std::unique_ptr < algorithm::stAlgorithm> testMoneyMaker;

		std::string name;
		std::string testNextTime;
		int actualIndex = 0;
		int testIndex = 0;
	};
}