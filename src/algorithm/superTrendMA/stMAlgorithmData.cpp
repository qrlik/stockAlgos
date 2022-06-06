#include "stMAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;

stMAlgorithmData::stMAlgorithmData() {
	getIndicatorsData().enableMA(2);
	getIndicatorsData().enableSuperTrend();
	getIndicatorsData().enableRSI();
}

bool stMAlgorithmData::operator==(const stMAlgorithmData& aOther) const {
	bool result = baseClass::operator==(aOther);
	result &= utils::isEqual(firstMATrendPrecision, aOther.firstMATrendPrecision);
	result &= utils::isEqual(secondMATrendPrecision, aOther.secondMATrendPrecision);
	result &= utils::isEqual(closerMATrendPrecision, aOther.closerMATrendPrecision);

	result &= closerTrendChangeCheck == aOther.closerTrendChangeCheck;
	result &= closerMACheck == aOther.closerMACheck;
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
	if (aName == "closerTrendChangeCheck") {
		closerTrendChangeCheck = aValue.get<bool>();
		return true;
	}
	if (aName == "closerMACheck") {
		closerMACheck = aValue.get<bool>();
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
	if (aName == "closerTrendChangeCheck") {
		return closerTrendChangeCheck == aValue.get<bool>();
	}
	if (aName == "closerMACheck") {
		return closerMACheck == aValue.get<bool>();
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
		result &= utils::isGreater(closerMATrendPrecision, 0.0);
	}
	result &= closerTrendChangeCheck || closerMACheck;
	if (closerConjuctionCheck) {
		result &= closerTrendChangeCheck && closerMACheck;
	}

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {
	aData["firstMATrendPrecision"] = firstMATrendPrecision;
	aData["secondMATrendPrecision"] = secondMATrendPrecision;
	aData["closerMATrendPrecision"] = closerMATrendPrecision;
	aData["closerTrendChangeCheck"] = closerTrendChangeCheck;
	aData["closerMACheck"] = closerMACheck;
	aData["closerConjuctionCheck"] = closerConjuctionCheck;
}