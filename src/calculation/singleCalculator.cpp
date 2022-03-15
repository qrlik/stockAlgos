#include "singleCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../market/indicators.h"
#include "../utils/utils.h"
#include <iostream>

using namespace calculation;

void calculation::singleCalculation() {
	algorithmData data{};
    data.atrType = market::eAtrType::SMA;
    data.atrSize = 15;
    data.stFactor = 3.0;

    data.dealPercent = 5.0;
    data.leverage = 50;

    data.activationPercent = 2.0;
    data.stopLossPercent = -1;
    data.minimumProfitPercent = 0.5;

    data.dynamicSLPercent = 0.25;
    data.dynamicSLTrendMode = false;

    data.touchOpenerActivationWaitMode = false;

    data.breakOpenerEnabled = true;
    data.breakOpenerActivationWaitMode = true;
    data.alwaysUseNewTrend = true;

    data.activationWaiterResetAllowed = false;
    data.activationWaiterRange = 2;
    data.activationWaiterFullCandleCheck = false;

    data.stopLossWaiterEnabled = false;
    data.stopLossWaiterResetAllowed = false;
    data.stopLossWaiterRange = -1;
    data.stopLossWaiterFullCandleCheck = false;

    auto json = utils::readFromJson("assets/candles/3year/1h");
    auto candles = utils::parseCandles(json);
    market::getProcessedCandles(candles, data.atrType, data.atrSize, data.stFactor, candles.size() - 1000);

    auto mm = algorithm::moneyMaker(data, 100.0);
    mm.setWithLogs(true);
    mm.calculate(candles);
    std::cout << mm.getFullCash() << '\n';
}