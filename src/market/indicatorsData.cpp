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
	result &= candlesToSkip == aOther.candlesToSkip;
	result &= atrType == aOther.atrType;
	result &= atrSize == aOther.atrSize;
	result &= utils::isEqual(stFactor, aOther.stFactor);
	return result;
}

void indicatorsData::enableSuperTrend() {
	enableAtr();
	superTrendFlag = true;
}

bool indicatorsData::isValid() const {
	auto result = true;
	if (isAtr()) {
		result &= atrType != market::eAtrType::NONE;
		result &= atrSize > 1;
	}
	if (isSuperTrend()) {
		result &= stFactor >= 1.0;
	}
	if (isMA()) {
		result &= firstMA > 0;
		if (maFlag > 1) {
			result &= secondMA > 0;
		}
	}
	return result;
}

void indicatorsData::addJsonData(Json& aData) const {
	if (isAtr()) {
		aData["atrType"] = market::atrTypeToString(atrType);
		aData["atrSize"] = atrSize;
	}
	if (isSuperTrend()) {
		aData["stFactor"] = stFactor;
	}
	if (isMA()) {
		aData["firstMA"] = firstMA;
		if (secondMA > 0) {
			aData["secondMA"] = secondMA;
		}
	}
}

bool indicatorsData::initDataField(const std::string& aName, const Json& aValue) {
	if (aName == "candlesToSkip") {
		candlesToSkip = aValue.get<int>();
		return true;
	}
	else if (aName == "atrType") {
		atrType = market::atrTypeFromString(aValue.get<std::string>());
		return true;
	}
	else if (aName == "atrSize") {
		atrSize = aValue.get<int>();
		return true;
	}
	else if (aName == "stFactor") {
		stFactor = aValue.get<double>();
		return true;
	}
	else if (aName == "firstMA") {
		firstMA = aValue.get<int>();
		return true;
	}
	else if (aName == "secondMA") {
		secondMA = aValue.get<int>();
		return true;
	}
	return false;
}

bool indicatorsData::checkCriteria(const std::string& aName, const Json& aValue) const {
	if (aName == "atrType") {
		return atrType == market::atrTypeFromString(aValue.get<std::string>());
	}
	else if (aName == "atrSize") {
		return atrSize == aValue.get<int>();
	}
	else if (aName == "stFactor") {
		return utils::isEqual(stFactor, aValue.get<double>());
	}
	else if (aName == "firstMA") {
		return firstMA == aValue.get<int>();
	}
	else if (aName == "secondMA") {
		return secondMA == aValue.get<int>();
	}
	return false;
}