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
		if (!aData.isLastMaximum) {
			utils::logError("maDirectionModule::update wrong UP logic");
		}
		const auto diffPercents = (1.0 - aMa / aData.lastMaximum) * 100.0;
		if (utils::isGreater(diffPercents, aPresicion)) {
			aData.state = eMaState::DOWN;
			aData.isLastMaximum = false;
			aData.lastMinimum = aMa;
		}
	}
	else if (aData.state == eMaState::DOWN) {
		if (aData.isLastMaximum) {
			utils::logError("maDirectionModule::update wrong DOWN logic");
		}
		const auto diffPercents = (aMa / aData.lastMinimum - 1.0) * 100.0;
		if (utils::isGreater(diffPercents, aPresicion)) {
			aData.state = eMaState::UP;
			aData.isLastMaximum = true;
			aData.lastMaximum = aMa;
		}
	}
}

bool maDirectionModule::operator==(const maDirectionModule& aOther) const {
	auto result = true;
	result &= firstData.state == aOther.firstData.state;
	result &= secondData.state == aOther.secondData.state;
	if (algorithm.getData().getFullCheck()) {
		result &= utils::isEqual(firstData.lastMinimum, aOther.firstData.lastMinimum);
		result &= utils::isEqual(firstData.lastMaximum, aOther.firstData.lastMaximum);
		result &= firstData.isLastMaximum == aOther.firstData.isLastMaximum;

		result &= utils::isEqual(secondData.lastMinimum, aOther.secondData.lastMinimum);
		result &= utils::isEqual(secondData.lastMaximum, aOther.secondData.lastMaximum);
		result &= secondData.isLastMaximum == aOther.secondData.isLastMaximum;
	}
	return result;
}

bool maDirectionModule::update() {
	updateData(firstData, algorithm.getIndicators().getFirstMA(), algorithm.getData().getFirstMATrendPrecision());
	updateData(secondData, algorithm.getIndicators().getSecondMA(), algorithm.getData().getSecondMATrendPrecision());
	return firstData.state != eMaState::NONE && secondData.state != eMaState::NONE;
}

bool maDirectionModule::isFirstUp() const {
	return (firstData.state == eMaState::UP) ? true : false;
}

bool maDirectionModule::isSecondUp() const {
	return (secondData.state == eMaState::UP) ? true : false;
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
		else if (name == "isLastMaximum") {
			aData.isLastMaximum = value.get<bool>();
		}
	}
}