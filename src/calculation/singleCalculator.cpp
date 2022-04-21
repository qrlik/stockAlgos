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
	data.atrSize = 50;
	data.stFactor = 14.0;

	data.dealPercent = 5.0;
	data.leverage = 50;

	data.startCash = 75'000.0;
	data.maxLossPercent = 35.0;
	data.maxLossCash = 25'000.0;
	data.orderSize = 2500.0;

	data.liquidationOffsetPercent = 0.05;
	data.minimumProfitPercent = 0.1;

	data.dynamicSLPercent = 0.5;
	data.dynamicSLTrendMode = false;

	data.touchOpenerActivationWaitMode = true;

	data.breakOpenerEnabled = true;
	data.breakOpenerActivationWaitMode = false;
	data.alwaysUseNewTrend = false;

	data.activationWaiterResetAllowed = false;
	data.activationWaiterRange = 4;
	data.activationWaiterFullCandleCheck = true;

	data.stopLossWaiterEnabled = true;
	data.stopLossWaiterResetAllowed = false;
	data.stopLossWaiterRange = 2;
	data.stopLossWaiterFullCandleCheck = false;

	tests::checkAlgorithmData(data);
	auto json = utils::readFromJson("assets/candles/BTCUSDT/15m");
	auto candles = utils::parseCandles(json);

	auto indicators = market::indicatorSystem(data.atrType, data.atrSize, data.stFactor);
	indicators.getProcessedCandles(candles, static_cast<int>(candles.size()) - 1000);

	auto mm = algorithm::moneyMaker(data);
	mm.setWithLogs(true);
	mm.calculate(candles);
	std::cout << mm.getFullCash() << '\n';
}