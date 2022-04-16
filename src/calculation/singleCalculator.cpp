#include "singleCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../market/indicators.h"
#include "../tests/checkers.h"
#include "../utils/utils.h"
#include <iostream>

using namespace calculation;

void calculation::singleCalculation() {
	algorithmData data{};
	data.atrType = market::eAtrType::RMA;
	data.atrSize = 15;
	data.stFactor = 18.0;

	data.dealPercent = 5.0;
	data.leverage = 100;

	data.startCash = 50000.0;
	data.maxLossPercent = 60.0;
	data.orderSize = 2500.0;

	data.liquidationOffsetPercent = 0.05;
	data.minimumProfitPercent = 0.5;

	data.dynamicSLPercent = -1.0;
	data.dynamicSLTrendMode = true;

	data.touchOpenerActivationWaitMode = false;

	data.breakOpenerEnabled = true;
	data.breakOpenerActivationWaitMode = false;
	data.alwaysUseNewTrend = true;

	data.activationWaiterResetAllowed = false;
	data.activationWaiterRange = -1;
	data.activationWaiterFullCandleCheck = false;

	data.stopLossWaiterEnabled = true;
	data.stopLossWaiterResetAllowed = true;
	data.stopLossWaiterRange = 3;
	data.stopLossWaiterFullCandleCheck = true;

	tests::checkAlgorithmData(data);
	auto json = utils::readFromJson("assets/candles/3year/2h");
	auto candles = utils::parseCandles(json);

	auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
	indicators.getProcessedCandles(candles, static_cast<int>(candles.size()) - 1000);

	auto mm = algorithm::moneyMaker(data);
	mm.setWithLogs(true);
	mm.calculate(candles);
	std::cout << mm.getFullCash() << '\n';
}