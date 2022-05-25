#include "stMAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;


bool stMAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aName == "firstMATrendPrecision") {
		firstMATrendPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "secondMATrendPrecision") {
		secondMATrendPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "closerTrendChangeCheck") {
		closerTrendChangeCheck = aValue.get<bool>();
		return true;
	}
	if (aName == "closerMainMACheck") {
		closerMainMACheck = aValue.get<bool>();
		return true;
	}
	if (aName == "closerConjuctionCheck") {
		closerConjuctionCheck = aValue.get<double>();
		return true;
	}
	return false;
}

bool stMAlgorithmData::checkCriteriaInternal(const std::string& aName, const Json& aValue) const {
	if (aName == "firstMATrendPrecision") {
		return utils::isEqual(firstMATrendPrecision, aValue.get<double>());
	}
	if (aName == "secondMATrendPrecision") {
		return utils::isEqual(secondMATrendPrecision, aValue.get<double>());
	}
	if (aName == "closerTrendChangeCheck") {
		return closerTrendChangeCheck == aValue.get<bool>();
	}
	if (aName == "closerMainMACheck") {
		return closerMainMACheck == aValue.get<bool>();
	}
	if (aName == "closerConjuctionCheck") {
		return closerConjuctionCheck == aValue.get<bool>();
	}
	return false;
}

bool stMAlgorithmData::isValidInternal() const {
	auto result = true;

	result &= utils::isGreater(firstMATrendPrecision, 0.0);
	result &= utils::isGreater(secondMATrendPrecision, 0.0);
	result &= closerTrendChangeCheck || closerMainMACheck;
	if (closerConjuctionCheck) {
		result &= closerTrendChangeCheck && closerMainMACheck;
	}

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {
	aData["firstMATrendPrecision"] = firstMATrendPrecision;
	aData["secondMATrendPrecision"] = secondMATrendPrecision;
	aData["closerTrendChangeCheck"] = closerTrendChangeCheck;
	aData["closerMainMACheck"] = closerMainMACheck;
	aData["closerConjuctionCheck"] = closerConjuctionCheck;
}