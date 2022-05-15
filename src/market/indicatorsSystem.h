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
		std::pair<double, bool> getSuperTrend() const { return { superTrend, trendIsUp }; }
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

		double atr = 0.f;

		double lastUpperBand = 0.0;
		double lastLowerBand = 0.0;
		double superTrend = 0.f;
		bool trendIsUp = false;

		int candlesCounter = 0;
		bool inited = false;
	};
}
