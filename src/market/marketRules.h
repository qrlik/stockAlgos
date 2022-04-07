#pragma once
#include <vector>

#define MARKET_DATA market::marketData::getInstance()

namespace market {
	struct tierData {
		double notionalCap = 0;
		double notionalFloor = 0;
		double maintenanceMarginRate = 0.0;
		double maintenanceAmount = 0;
		int maxLeverage = 0;
	};

	class marketData {
	public:
		static marketData* getInstance();
		static double getLiquidationPrice(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong);
		static double getLiquidationPercent(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong);
		static double getLiquidationPercent(double aMargin, int aLeverage);

		const tierData& getTierData(double aPosition) const;
		const std::vector<tierData>& getTiersData() const;
		std::pair<double, double> getLeverageLiquidationRange(int aLeverage) const;
		double getLeverageMaxPosition(int aLeverage) const;
		double getQuantityPrecision() const;
		double getPricePrecision() const;
		double getQuotePrecision() const;
		double getMinNotionalValue() const;
	private:
		marketData();
		void init();

		static marketData* instance;
		std::vector<tierData> tiersData;

		double quantityPrecision = 0.0;
		double pricePrecision = 0.0;
		double quotePrecision = 0.0;
		double minNotionalValue = 0.0;
	};
}