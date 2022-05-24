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
	return false;
}

bool stMAlgorithmData::checkCriteriaInternal(const std::string& aName, const Json& aValue) const {
	if (aName == "firstMATrendPrecision") {
		return utils::isEqual(firstMATrendPrecision, aValue.get<double>());
	}
	if (aName == "secondMATrendPrecision") {
		return utils::isEqual(secondMATrendPrecision, aValue.get<double>());
	}
	return false;
}

bool stMAlgorithmData::isValidInternal() const {
	auto result = true;

	result &= firstMATrendPrecision > 0.0;
	result &= secondMATrendPrecision > 0.0;

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {
	aData["firstMATrendPrecision"] = firstMATrendPrecision;
	aData["secondMATrendPrecision"] = secondMATrendPrecision;
}