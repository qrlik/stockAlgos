#include "stMAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;


bool stMAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aName == "maTrendPrecision") {
		maTrendPrecision = aValue.get<double>();
		return true;
	}
	return false;
}

bool stMAlgorithmData::checkCriteriaInternal(const std::string& aName, const Json& aValue) const {
	if (aName == "maTrendPrecision") {
		return utils::isEqual(maTrendPrecision, aValue.get<double>());
	}
	return false;
}

bool stMAlgorithmData::isValidInternal() const {
	auto result = true;

	result &= maTrendPrecision > 0.0;

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {
	aData["maTrendPrecision"] = maTrendPrecision;
}