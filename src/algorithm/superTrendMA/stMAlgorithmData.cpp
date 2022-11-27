#include "stMAlgorithmData.h"
#include "utils/utils.h"

using namespace algorithm;

stMAlgorithmData::stMAlgorithmData(const std::string ticker) : algorithmDataBase(ticker) {
	getIndicatorsData().enableSuperTrend();
}

bool stMAlgorithmData::operator==(const stMAlgorithmData& aOther) const {
	bool result = baseClass::operator==(aOther);

	return result;
}

size_t stMAlgorithmData::getCustomHash() const {
	size_t result{};

	return result;
}

size_t stMAlgorithmData::getCustomID() const {
	size_t result{};

	return result;
}

bool stMAlgorithmData::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	return false;
}

bool stMAlgorithmData::checkCriteriaInternal(const std::string& aName, const Json& aValue) const {
	return false;
}

bool stMAlgorithmData::isValidInternal() const {
	auto result = true;

	return result;
}

void stMAlgorithmData::addJsonDataInternal(Json& aData) const {

}