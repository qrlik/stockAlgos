#include "stMAlgorithm.h"

using namespace algorithm;

stMAlgorithm::stMAlgorithm(const stMAlgorithmData& aData) :
	baseClass(aData),
	opener(*this),
	maDirection(*this),
	closer(*this) {}

bool stMAlgorithm::operator==(const stMAlgorithm& aOther) const {
	auto result = baseClass::operator==(aOther);
	result &= maDirection == aOther.maDirection;
	return result;
}

void stMAlgorithm::preLoop() {}

bool stMAlgorithm::loop() {
	if (!maDirection.update()) {
		return false;
	}
	const auto curState = getState();
	if (curState == getIntState(eBaseState::NONE)) {
		return opener.check();
	}
	else if (curState == getIntState(eBaseState::LONG) || curState == getIntState(eBaseState::SHORT)) {
		return closer.check();
	}
	return false;
}

void stMAlgorithm::onOpenOrder() {
	opener.touchActivated = false;
}

void stMAlgorithm::onCloseOrder(eOrderState aState, double aProfit) {
	opener.lastClosedOrder.first = getCandle().time;
	opener.lastClosedOrder.second = aState;
}

void stMAlgorithm::logInternal(std::ofstream& aFile) const {
	// TO DO
}

void stMAlgorithm::initInternal() {}

void stMAlgorithm::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	if (aName == "firstMA") {
		maDirection.updateData(maDirection.firstData, aValue);
	}
	else if (aName == "secondMA") {
		maDirection.updateData(maDirection.secondData, aValue);
	}
}