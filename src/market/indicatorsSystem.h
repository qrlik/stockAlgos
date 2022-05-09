#pragma once
#include "candle.h"
#include <vector>
#include <deque>

namespace algorithm {
	class algorithmDataBase;
}
namespace market {
	class indicatorsData;
	class indicatorsSystem {
	public:
		indicatorsSystem(const algorithm::algorithmDataBase& aData);
		void processCandle(candle& aCandle);
		bool isInited() const { return inited; }
	private:
		double calculateTrueRangeWMA() const;
		double calculateTrueRangeEMA(double aAlpha);
		double calculateTrueRangeMA();
		bool calculateRangeAtr(candle& aCandle);
		void calculateSuperTrend(candle& aCandle);
		bool checkSkip();

		const indicatorsData& data;

		std::deque<double> trList;
		candle prevCandle;

		double lastUpperBand = 0.0;
		double lastLowerBand = 0.0;
		double lastTrend = 0.0;
		double lastClose = 0.0;
		int lastCandleTime = 0;
		int candlesCounter = 0;
		bool inited = false;
	};
}
