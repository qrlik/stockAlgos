#pragma once
#include "json/json.hpp"
#include "market/candle.h"

namespace calculation {
	class MaxLossBalancer {
	public:
		MaxLossBalancer(const std::string& ticker, market::eCandleInterval interval, const Json& data, double maxLossPercent);

		void calculate(const std::string& algoType);
		double getMaxLossPercent() const { return mActualMaxLossPercent; }
		double getDealPercent() const { return mActualDealPercent; }
	private:
		bool isValid() const;
		bool isReadyForBalance() const;

		void terminate();
		void onSuccess();
		void onOverhead();
		void onBalanced();
		void increaseValues(bool success);

		template<typename algorithmType>
		void iterate() {
			auto data = algorithmType::algorithmDataType{mTicker};
			mData["dealPercent"] = mLastDealPercent;
			if (!data.initFromJson(mData)) {
				utils::logError("MaxLossBalancer::iterate wrong alrorithm data");
				terminate();
				return;
			}
			auto algorithm = algorithmType(data, mInterval);
			auto result = algorithm.calculate(lastCandles);
			mLastMaxLossPercent = algorithm.getJsonData()["stats"]["maxLossPercent"].get<double>();
			if (result) {
				onSuccess();
			}
			else {
				onOverhead();
			}
		}

		Json mData;
		std::string mTicker;

		const double mMaxLossPercentCeil = 0.0;
		const double mDealPercentPrecision = 0.01;

		double mIncreaseFactor = 10.0;
		double mLastMaxLossPercent = 0.0;
		double mLastDealPercent = 0.0;

		double mActualMaxLossPercent = 0.0;
		double mActualDealPercent = 0.0;

		double mMinFailedPercent = std::numeric_limits<double>::max();

		market::eCandleInterval mInterval = market::eCandleInterval::NONE;
		bool mStepsStarted = false;
		bool mBalanced = false;
		bool mTerminated = false;
	};
}