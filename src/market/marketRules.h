#pragma once
#include <map>

#define MARKET_DATA market::marketData::getInstance

namespace market {
	class marketData {
	public:
		static marketData* getInstance();
		int getPrecision();
	private:
		struct tierData {
			double maintenanceMarginRate = 0.0;
			int maintenanceAmount = 0;
			int maxLeverage = 0;
		};
		marketData();
		void init();

		static marketData* instance;
		std::map<int, tierData> tiersData;
		int precision = 0;
	};
}