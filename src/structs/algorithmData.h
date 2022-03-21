#pragma once
#include "../market/indicators.h"

struct algorithmData {
	static const double tax;
	static algorithmData initAlgorithmDataFromJson(const Json& aData);

	market::eAtrType atrType = market::eAtrType::NONE;
	int atrSize = -1;
	double stFactor = -1.0;

	double dealPercent = -1.0;
	int leverage = -1;

	double startCash = 0.0;
	double stopCash = 0.0;

	double liquidationOffsetPercent = -1.0;
	double minimumProfitPercent = -1.0;

	double dynamicSLPercent = -1.0;
	bool dynamicSLTrendMode = false;

	bool touchOpenerActivationWaitMode = false;

	bool breakOpenerEnabled = false;
	bool breakOpenerActivationWaitMode = false;
	bool alwaysUseNewTrend = false;

	bool activationWaiterResetAllowed = false;
	int activationWaiterRange = -1;
	bool activationWaiterFullCandleCheck = false;

	bool stopLossWaiterEnabled = false;
	bool stopLossWaiterResetAllowed = false;
	int stopLossWaiterRange = -1;
	bool stopLossWaiterFullCandleCheck = false;

	bool fullCheck = false;
};