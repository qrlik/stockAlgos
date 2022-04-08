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
	data.atrType = market::eAtrType::SMA;
	data.atrSize = 15;
	data.stFactor = 10.0;

	data.dealPercent = 5.0;
	data.leverage = 100;

	data.startCash = 50000.0;
	data.stopCash = 20000.0;
	data.orderSize = 2500.0;

	data.liquidationOffsetPercent = 0.05;
	data.minimumProfitPercent = 2.5;

	data.dynamicSLPercent = -1.0;
	data.dynamicSLTrendMode = true;

	data.touchOpenerActivationWaitMode = false;

	data.breakOpenerEnabled = true;
	data.breakOpenerActivationWaitMode = true;
	data.alwaysUseNewTrend = true;

	data.activationWaiterResetAllowed = true;
	data.activationWaiterRange = 4;
	data.activationWaiterFullCandleCheck = false;

	data.stopLossWaiterEnabled = true;
	data.stopLossWaiterResetAllowed = false;
	data.stopLossWaiterRange = 3;
	data.stopLossWaiterFullCandleCheck = true;

	tests::checkAlgorithmData(data);
	auto json = utils::readFromJson("assets/candles/3year/1h");
	auto candles = utils::parseCandles(json);

	auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
	indicators.getProcessedCandles(candles, static_cast<int>(candles.size()) - 1000);

	auto mm = algorithm::moneyMaker(data);
	mm.setWithLogs(true);
	mm.calculate(candles);
	std::cout << mm.getFullCash() << '\n';
}