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
		market::eAtrType getAtrType() { return atrType; }
		int getAtrSize() { return atrSize; }
		double getStFactor() { return stFactor; }

	private:
		market::eAtrType atrType = market::eAtrType::NONE;
		int atrSize = -1;
		double stFactor = -1.0;
	};
}
