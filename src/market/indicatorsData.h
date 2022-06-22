#pragma once
#include "candle.h"
#include <vector>
#include <deque>

namespace market {
	enum class eAtrType : unsigned char {
		NONE = 0,
		RMA = 1,
		EMA = 2,
		WMA = 3,
		SMA = 4
	};
	std::string atrTypeToString(eAtrType aType);
	eAtrType atrTypeFromString(const std::string& aStr);

	class indicatorsData {
	public:
		bool operator==(const indicatorsData& aOther) const;
		size_t getHash() const;
		void addJsonData(Json& aData) const;
		bool initDataField(const std::string& aName, const Json& aValue);
		bool checkCriteria(const std::string& aName, const Json& aValue) const;

		void enableAtr() { atrFlag = true; }
		void enableSuperTrend();
		void enableRSI() { rsiFlag = true; }
		void enableMA(size_t aAmount) { maFlag = aAmount; }

		bool isValid() const;
		bool isAtr() const { return atrFlag; }
		bool isSuperTrend() const { return superTrendFlag; }
		bool isMA() const { return maFlag > 0; }
		bool isRSI() const { return rsiFlag; }

		int getSkipAmount() const { return candlesToSkip; }
		market::eAtrType getAtrType() const { return atrType; }
		int getAtrSize() const { return atrSize; }
		double getStFactor() const { return stFactor; }
		int getFirstMASize() const { return firstMA; }
		int getSecondMASize() const { return secondMA; }
		int getRsiSize() const { return rsiSize; }
	private:
		double stFactor = -1.0;
		int atrSize = -1;
		market::eAtrType atrType = market::eAtrType::NONE;

		int firstMA = 0;
		int secondMA = 0;

		int rsiSize = 0;

		int candlesToSkip = 0;
		size_t maFlag = false;
		bool atrFlag = false;
		bool superTrendFlag = false;
		bool rsiFlag = false;
	};
}
