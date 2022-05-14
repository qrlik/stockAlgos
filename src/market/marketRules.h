#pragma once
#include <string>
#include <vector>
#include <unordered_map>

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
		bool loadTickerData(const std::string& aTicker);

		const tierData& getTierData(double aPosition) const;
		const std::vector<tierData>& getTiersData() const;
		std::pair<double, double> getLeverageLiquidationRange(int aLeverage) const;
		double getLeverageMaxPosition(int aLeverage) const;
		double getQuantityPrecision() const;
		double getPricePrecision() const;
		double getQuotePrecision() const;
		double getMinNotionalValue() const;
		double getLiquidationPrice(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const;
		double getLiquidationPercent(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const;
		double getLiquidationPercent(double aMargin, int aLeverage) const;
		double getTaxFactor() const;
		int getMaxLeverage() const;
	private:
		marketData();
		void loadExchangeSettings();

		static marketData* instance;

		struct tickerData {
			std::vector<tierData> tiersData;
			double quantityPrecision = 0.0;
			double pricePrecision = 0.0;
			double quotePrecision = 0.0;
			double minNotionalValue = 0.0;
		};

		std::unordered_map<std::string, tickerData> data;
		std::string currentTicker;
		double tax = 0.0;
	};
}