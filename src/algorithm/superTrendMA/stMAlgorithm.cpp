#include "stMAlgorithm.h"

using namespace algorithm;

stMAlgorithm::stMAlgorithm(const stMAlgorithmData& aData) :
	baseClass(aData),
	opener(*this),
	maDirection(*this),
	closer(*this) {}

void stMAlgorithm::preLoop() {
	maDirection.update();
}

bool stMAlgorithm::loop() {
	const auto curState = getState();
	if (curState == getIntState(eBaseState::NONE)) {
		return opener.check();
	}
	else if (curState == getIntState(eBaseState::LONG) || curState == getIntState(eBaseState::SHORT)) {
		return closer.check();
	}
	return true;
}

void stMAlgorithm::onOpenOrder() {
	// TO DO
}

void stMAlgorithm::onCloseOrder(double aProfit) {
	// TO DO
}

void stMAlgorithm::logInternal(std::ofstream& aFile) const {
	// TO DO
}

void stMAlgorithm::initInternal() {}

void stMAlgorithm::initDataFieldInternal(const std::string& aName, const Json& aValue) {
	// TO DO
}