#pragma once
#include "market/indicatorsData.h"
#include "json/json.hpp"

namespace algorithm {
	class algorithmDataBase {
	public:
		algorithmDataBase() = default;
		bool operator==(const algorithmDataBase& aOther) const;
		void setID();
		size_t getID() const { return id; }
		void addJsonData(Json& aData) const;
		bool initFromJson(const Json& aValue);
		bool initDataField(const std::string& aName, const Json& aValue);
		bool checkCriteria(const std::string& aName, const Json& aValue) const;
		bool isValid() const;

		const market::indicatorsData& getIndicatorsData() const { return indicatorsData; }
		double getDealPercent() const { return dealPercent; }
		double getOrderSize() const { return orderSize; }
		int getLeverage() const { return leverage; }

		double getStartCash() const { return startCash; }
		double getMaxLossPercent() const { return maxLossPercent; }
		double getMaxLossCash() const { return maxLossCash; }

		double getStopLossPercent() const { return stopLossPercent; }
		double getLiquidationOffsetPercent() const { return liquidationOffsetPercent; }
		double getMinimumProfitPercent() const { return minimumProfitPercent; }
		bool getFullCheck() const { return fullCheck; }
		bool getFullCheckCustom() const { return fullCheckCustom; }

	protected:
		using baseClass = algorithmDataBase;

		virtual size_t getCustomHash() const = 0;
		virtual bool initDataFieldInternal(const std::string& aName, const Json& aValue) = 0;
		virtual bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const = 0;
		virtual bool isValidInternal() const = 0;
		virtual void addJsonDataInternal(Json& aData) const = 0;

		market::indicatorsData& getIndicatorsData() { return indicatorsData; }
	private:
		size_t getHash() const;
		size_t getBaseHash() const;

		market::indicatorsData indicatorsData;

		double dealPercent = -1.0;
		double orderSize = -1.0;
		int leverage = -1;
		size_t id = 0;

		double startCash = 0.0;
		double maxLossPercent = 0.0;
		double maxLossCash = -1.0;

		double stopLossPercent = -1.0;
		double liquidationOffsetPercent = -1.0;
		double minimumProfitPercent = -1.0;

		bool fullCheck = false;
		bool fullCheckCustom = false;
	};
}