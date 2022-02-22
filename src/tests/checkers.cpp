#include "checkers.h"

using namespace tests;

void tests::checkAlgorithmData(const algorithmData& aData) {
	assert(aData.atrType != indicators::eAtrType::NONE);
	assert(aData.stFactor > 0.0);
	assert(aData.dealPercent > 0.0);

	const auto liquidationPercent = 100.0 / aData.leverage;
	assert(aData.activationPercent >= 0.0 && aData.activationPercent <= liquidationPercent);
	assert(aData.stopLossPercent >= aData.activationPercent && aData.stopLossPercent <= liquidationPercent);
	assert(aData.minimumProfitPercent > 2 * algorithmData::tax);


}