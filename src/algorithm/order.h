#pragma once
#include "json/json.hpp"
#include <string>

namespace algorithm {
	class algorithmDataBase;
}

namespace algorithm {
	enum class eOrderState {
		NONE = 0,
		LONG = 1,
		SHORT = 2
	};
	double getMinLiquidationPercent();
	class order {
	public:
		order(const algorithm::algorithmDataBase& data);
		void initFromJson(const Json& aJson);
		bool operator==(const order& aOther) const;
		bool openOrder(eOrderState aState, double aPrice, double aCash, const std::string& aTime);
		double closeOrder();
		void reset();
		std::string toString() const;

		const std::string& getTime() const { return time; }
		double getPrice() const { return price; }
		double getStopLoss() const { return stopLoss; }
		double getMinimumProfit() const { return minimumProfit; }
		double getMargin() const { return margin; }
		double getNotionalValue() const { return notionalValue; }
		double getQuantity() const { return quantity; }
		double getProfit() const;
		eOrderState getState() const { return state; }
		void updateStopLoss(double aStopLoss);
	private:
		double calculateStopLoss() const;
		double calculateMinimumProfit() const;

		const algorithm::algorithmDataBase& mData;
		std::string time;
		double price = 0.0;
		double stopLoss = 0.0;
		double initStopLoss = 0.0;
		double minimumProfit = 0.0;
		double margin = 0.0;
		double notionalValue = 0.0;
		double quantity = 0.0;
		eOrderState state;
		bool fullCheck = false;
	};
}