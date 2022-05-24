#include "maDirectionModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

maDirectionModule::maDirectionModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

void maDirectionModule::updateData(maData& aData, double aMa, double aPresicion) {
	if (aMa > aData.lastMaximum) {
		aData.lastMaximum = aMa;
		aData.isLastMaximum = true;
	}
	if (aMa < aData.lastMinimum) {
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