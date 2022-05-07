#pragma once
#include "json/json.hpp"

namespace algorithm {
	class algorithmDataBase {
	public:
		algorithmDataBase() = default;
		bool operator==(const algorithmDataBase& aOther) const;
		void addJsonData(Json& aData) const;
		bool initFromJson(const Json& aValue);
		bool initDataField(const std::string& aName, const Json& aValue);
		bool checkCriteria(const std::string& aName, const Json& aValue) const;
		bool isValid() const;

		double getDealPercent() const { return dealPercent; }
		double getOrderSize() const { return orderSize; }
		int getLeverage() const { return leverage; }

		double getStartCash() const { return startCash; }
		double getMaxLossPercent() const { return maxLossPercent; }
		double getMaxLossCash() const { return maxLossCash; }

		double getLiquidationOffsetPercent() const { return liquidationOffsetPercent; }
		double getMinimumProfitPercent() const { return minimumProfitPercent; }
		bool getFullCheck() const { return fullCheck; }
	protected:
		virtual bool initDataFieldInternal(const std::string& aName, const Json& aValue) = 0;
		virtual bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const = 0;
		virtual bool isValidInternal() const = 0;
		virtual void addJsonDataInternal(Json& aData) const = 0;
	private:
		double dealPercent = -1.0;
		double orderSize = -1.0;
		int leverage = -1;

		double startCash = 0.0;
		double maxLossPercent = 0.0;
		double maxLossCash = -1.0;

		double liquidationOffsetPercent = -1.0;
		double minimumProfitPercent = -1.0;

		bool fullCheck = false;
	};
}