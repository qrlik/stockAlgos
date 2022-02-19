#pragma once
#include "../indicators/indicators.h"

struct algorithmData {
    indicators::eAtrType atrType = indicators::eAtrType::NONE;
    size_t atrSize = 0;
    double stFactor = 0.0;

    double dealPercent = 0.0;
    size_t leverage = 0;

    double activationPercent = 0.0;
    double stopLossPercent = 0.0;
    double minimumProfitPercent = 0.0;

    double dynamicSLPercent = 0.0;
    bool dynamicSLTrendMode = false;

    bool touchOpenerActivationWaitMode = false;

    bool breakOpenerEnabled = false;
    bool breakOpenerActivationWaitMode = false;
    bool alwaysUseNewTrend = false;

    bool activationWaiterResetAllowed = false;
    size_t activationWaiterRange = 0;
    bool activationWaiterFullCandleCheck = false;

    bool stopLossWaiterEnabled = false;
    bool stopLossWaiterResetAllowed = false;
    size_t stopLossWaiterRange = 0;
    bool stopLossWaiterFullCandleCheck = false;

    bool fullCheck = false;
};