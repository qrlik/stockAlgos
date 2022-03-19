#pragma once
#include <vector>

#define MARKET_DATA market::marketData::getInstance

namespace market {
	class marketData {
	public:
		static marketData* getInstance();
		int getPrecision() const;
		double getLiquidationPrice() const;
		double getMaximumLeveragePosition(int aLeverage) const;
	private:
		struct tierData {
			double position = 0;
			double maintenanceMarginRate = 0.0;
			int maintenanceAmount = 0;
			int maxLeverage = 0;
		};
		marketData();
		void init();
		void runTests();

		static marketData* instance;
		std::vector<tierData> tiersData;
		int precision = 0;
	};
}