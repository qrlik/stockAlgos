#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#define MARKET_SYSTEM market::marketSystem::getInstance()

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
		marketData(const std::string& ticker);

		const tierData& getTierData(double aPosition) const;
		const std::vector<tierData>& getTiersData() const;
		std::pair<double, double> getLeverageLiquidationRange(int aLeverage) const;
		double getLeverageMaxPosition(int aLeverage) const;
		double getQuantityPrecision() const;
		double getPricePrecision(double price) const;
		double getQuotePrecision() const;
		double getMinNotionalValue() const;
		double getLiquidationPrice(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const;
		double getLiquidationPercent(double aPrice, double aNotional, double aLeverage, double aQuantity, bool aLong) const;
		double getLiquidationPercent(double aMargin, int aLeverage) const;
		int getMaxLeverage() const;
	private:
		bool loadTickerData(const std::string& ticker);

		std::vector<tierData> tiersData;
		std::string mTicker;
		double quantityPrecision = 0.0;
		double pricePrecision = 0.0;
		double quotePrecision = 0.0;
		double minNotionalValue = 0.0;
	};

	class marketSystem {
	public:
		static marketSystem* getInstance();

		const marketData& getMarketData(const std::string& ticker);
		double getTaxFactor() const;
		double getTickFactor() const;
	private:
		marketSystem();
		void loadExchangeSettings();

		static marketSystem* instance;

		std::unordered_map<std::string, marketData> data;
		double tax = 0.0;
		double priceTickFactor = 0.0;
	};
}