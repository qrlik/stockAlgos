#include "maDirectionModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

maDirectionModule::maDirectionModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

void maDirectionModule::updateData(maData& aData, double aMa, double aPresicion) {
	if (utils::isGreater(aMa, aData.lastMaximum)) {
		aData.lastMaximum = aMa;
		aData.isLastMaximum = true;
	}
	if (utils::isLess(aMa, aData.lastMinimum)) {
		aData.lastMinimum = aMa;
		aData.isLastMaximum = false;
	}
	if (aData.state == eMaState::NONE) {
		const auto diff = aData.lastMaximum - aData.lastMinimum;
		auto diffPercents = 0.0;
		if (aData.isLastMaximum) {
			diffPercents = diff / aData.lastMinimum * 100.0;
		}
		else {
			diffPercents = diff / aData.lastMaximum * 100.0;
		}
		if (utils::isGreater(diffPercents, aPresicion)) {
			aData.state = aData.isLastMaximum ? eMaState::UP : eMaState::DOWN;
		}
	}
	else if (aData.state == eMaState::UP) {
		const auto diffPercents = (1.0 - aMa / aData.lastMaximum) * 100.0;
		if (utils::isGreater(diffPercents, aPresicion)) {
			aData.state = eMaState::DOWN;
			aData.lastMinimum = aMa;
		}
	}
	else if (aData.state == eMaState::DOWN) {
		const auto diffPercents = (aMa / aData.lastMinimum - 1.0) * 100.0;
		if (utils::isGreater(diffPercents, aPresicion)) {
			aData.state = eMaState::UP;
			aData.lastMaximum = aMa;
		}
	}
}

bool maDirectionModule::operator==(const maDirectionModule& aOther) const {
	auto result = true;
	result &= firstData.state == aOther.firstData.state;
	result &= secondData.state == aOther.secondData.state;
	if (algorithm.getData().getFullCheckCustom()) {
		result &= utils::isEqual(firstData.lastMinimum, aOther.firstData.lastMinimum);
		result &= utils::isEqual(firstData.lastMaximum, aOther.firstData.lastMaximum);

		result &= utils::isEqual(secondData.lastMinimum, aOther.secondData.lastMinimum);
		result &= utils::isEqual(secondData.lastMaximum, aOther.secondData.lastMaximum);
	}
	return result;
}

bool maDirectionModule::update() {
	updateData(firstData, algorithm.getIndicators().getFirstMA(), algorithm.getData().getFirstMATrendPrecision());
	updateData(secondData, algorithm.getIndicators().getSecondMA(), algorithm.getData().getSecondMATrendPrecision());
	updateData(closerData, algorithm.getIndicators().getSecondMA(), algorithm.getData().getCloserMATrendPrecision());
	return firstData.state != eMaState::NONE && secondData.state != eMaState::NONE && closerData.state != eMaState::NONE;
}

bool maDirectionModule::isFirstUp() const {
	return (firstData.state == eMaState::UP) ? true : false;
}

bool maDirectionModule::isSecondUp() const {
	return (secondData.state == eMaState::UP) ? true : false;
}

bool maDirectionModule::isCloserUp() const {
	return (closerData.state == eMaState::UP) ? true : false;
}

maDirectionModule::eMaState maDirectionModule::stateFromStr(const std::string aName) const {
	if (aName == "DOWN") {
		return eMaState::DOWN;
	}
	if (aName == "UP") {
		return eMaState::UP;
	}
	return eMaState::NONE;
}

void maDirectionModule::updateData(maData& aData, const Json& aJson) {
	for (const auto& [name, value] : aJson.items()) {
		if (name == "lastMinimum") {
			aData.lastMinimum = value.get<double>();
		}
		else if (name == "lastMaximum") {
			aData.lastMaximum = value.get<double>();
		}
		else if (name == "state") {
			aData.state = stateFromStr(value.get<std::string>());
		}
	}
}