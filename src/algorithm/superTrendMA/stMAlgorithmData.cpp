#include "stMAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;

stMAlgorithmData::stMAlgorithmData() {
	getIndicatorsData().enableMA(2);
	getIndicatorsData().enableSuperTrend();
	//getIndicatorsData().enableRSI();
}

bool stMAlgorithmData::operator==(const stMAlgorithmData& aOther) const {
	bool result = baseClass::operator==(aOther);
	result &= utils::isEqual(firstMATrendPrecision, aOther.firstMATrendPrecision);
	result &= utils::isEqual(secondMATrendPrecision, aOther.secondMATrendPrecision);
	result &= utils::isEqual(closerMATrendPrecision, aOther.closerMATrendPrecision);
	result &= utils::isEqual(closerTrailPrecision, aOther.closerTrailPrecision);

	result &= closerTrendChangeCheck == aOther.closerTrendChangeCheck;
	result &= closerMACheck == aOther.closerMACheck;
	result &= closerTrailStop == aOther.closerTrailStop;
	result &= closerConjuctionCheck == aOther.closerConjuctionCheck;
	return result;
}

bool stMAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aName == "firstMATrendPrecision") {
		firstMATrendPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "secondMATrendPrecision") {
		secondMATrendPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "closerMATrendPrecision") {
		closerMATrendPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "closerTrailPrecision") {
		closerTrailPrecision = aValue.get<double>();
		return true;
	}
	if (aName == "closerTrendChangeCheck") {
		closerTrendChangeCheck = aValue.get<bool>();
		return true;
	}
	if (aName == "closerMACheck") {
		closerMACheck = aValue.get<bool>();
		return true;
	}
	if (aName == "closerTrailStop") {
		closerTrailStop = aValue.get<bool>();
		return true;
	}
	if (aName == "closerConjuctionCheck") {
		closerConjuctionCheck = aValue.get<bool>();
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
	if (aName == "closerMATrendPrecision") {
		return utils::isEqual(closerMATrendPrecision, aValue.get<double>());
	}
	if (aName == "closerTrailPrecision") {
		return utils::isEqual(closerTrailPrecision, aValue.get<double>());
	}
	if (aName == "closerTrendChangeCheck") {
		return closerTrendChangeCheck == aValue.get<bool>();
	}
	if (aName == "closerMACheck") {
		return closerMACheck == aValue.get<bool>();
	}
	if (aName == "closerTrailStop") {
		return closerTrailStop == aValue.get<bool>();
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
	if (closerMACheck) {
		result &= !closerTrailStop;
		result &= utils::isGreater(closerMATrendPrecision, 0.0);
	}
	if (closerTrailStop) {
		result &= !closerMACheck;
		result &= utils::isGreater(closerTrailPrecision, 0.0);
	}
	result &= closerTrendChangeCheck || closerMACheck || closerTrailStop;
	if (closerConjuctionCheck) {
		result &= closerTrendChangeCheck && closerMACheck;
	}

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {
	aData["firstMATrendPrecision"] = firstMATrendPrecision;
	aData["secondMATrendPrecision"] = secondMATrendPrecision;
	aData["closerMATrendPrecision"] = closerMATrendPrecision;
	aData["closerTrailingPrecision"] = closerTrailPrecision;
	aData["closerTrendChangeCheck"] = closerTrendChangeCheck;
	aData["closerMACheck"] = closerMACheck;
	aData["closerTrailStop"] = closerTrailStop;
	aData["closerConjuctionCheck"] = closerConjuctionCheck;
}