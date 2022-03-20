#pragma once
#include <vector>

#define MARKET_DATA market::marketData::getInstance

namespace market {
	struct tierData {
		double position = 0;
		double maintenanceMarginRate = 0.0;
		int maintenanceAmount = 0;
		int maxLeverage = 0;
	};

	class marketData {
	public:
		static marketData* getInstance();
		int getPrecision() const;
		const tierData& getTierData(double aPosition) const;
		const std::vector<tierData>& getTiersData() const;
		double getMaximumLeveragePosition(int aLeverage) const;
	private:
		marketData();
		void init();

		static marketData* instance;
		std::vector<tierData> tiersData;
		int precision = 0;
	};
}