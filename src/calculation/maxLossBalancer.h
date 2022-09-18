#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace calculation {
	class MaxLossBalancer {
	public:
		MaxLossBalancer(const std::string ticker, market::eCandleInterval interval, const Json& data, double maxLossPercent);

		void calculate();

		// getters

	private:
		bool isValid() const;
		bool isReadyForBalance() const;
		void iterate();

		Json mData;
		std::string mTicker;

		const double mMaxLossPercentCeil = 0.0;
		double mCurMaxLossPercent = 0.0;
		double mCurDealPercent = 0.0;

		market::eCandleInterval mInterval = market::eCandleInterval::NONE;
		bool mBalanced = false;
	};
}