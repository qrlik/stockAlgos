#include "indicatorsData.h"
#include "utils/utils.h"

using namespace market;

std::string market::atrTypeToString(eAtrType aType) {
	switch (aType) {
	case market::eAtrType::RMA:
		return "RMA";
	case market::eAtrType::EMA:
		return "EMA";
	case market::eAtrType::WMA:
		return "WMA";
	case market::eAtrType::SMA:
		return "SMA";
	default:
		return "NONE";
	}
}

eAtrType market::atrTypeFromString(const std::string& aStr) {
	if (aStr == "RMA") {
		return market::eAtrType::RMA;
	}
	else if (aStr == "EMA") {
		return market::eAtrType::EMA;
	}
	else if (aStr == "WMA") {
		return market::eAtrType::WMA;
	}
	else if (aStr == "SMA") {
		return market::eAtrType::SMA;
	}
	return market::eAtrType::NONE;
}

bool indicatorsData::operator==(const indicatorsData& aOther) const {
	bool result = true;
	result &= atrType == aOther.atrType;
	result &= atrSize == aOther.atrSize;
	result &= utils::isEqual(stFactor, aOther.stFactor);
	return result;
}