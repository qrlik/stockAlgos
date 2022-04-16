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
	data.atrType = market::eAtrType::EMA;
	data.atrSize = 50;
	data.stFactor = 3.5;

	data.dealPercent = 5.0;
	data.leverage = 100;

	data.startCash = 75'000.0;
	data.maxLossPercent = 35.0;
	data.maxLossCash = 50'000.0;
	data.orderSize = 2500.0;

	data.liquidationOffsetPercent = 0.05;
	data.minimumProfitPercent = 0.25;

	data.dynamicSLPercent = 32.0;
	data.dynamicSLTrendMode = false;

	data.touchOpenerActivationWaitMode = false;

	data.breakOpenerEnabled = true;
	data.breakOpenerActivationWaitMode = false;
	data.alwaysUseNewTrend = true;

	data.activationWaiterResetAllowed = false;
	data.activationWaiterRange = -1;
	data.activationWaiterFullCandleCheck = false;

	data.stopLossWaiterEnabled = true;
	data.stopLossWaiterResetAllowed = false;
	data.stopLossWaiterRange = 0;
	data.stopLossWaiterFullCandleCheck = false;

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