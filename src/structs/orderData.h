#pragma once
#include "../../third_party/json.hpp"
#include <string>

namespace algorithm {
	class moneyMaker;
	enum class eState : unsigned char;
}

class orderData {
public:
	static void initOrderDataFromJson(orderData& aData, const Json& aJson);

	orderData();
	bool operator==(const orderData& aOther);
	void openOrder(const algorithm::moneyMaker& aMM, double aPrice);
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
	void updateStopLoss(double aStopLoss) { stopLoss = aStopLoss; }
	void setFullCheck(bool aFullCheck) { fullCheck = aFullCheck; }
private:
	double calculateLiquidationPrice() const;
	double calculateStopLoss(const algorithm::moneyMaker& aMM) const;
	double calculateMinimumProfit(const algorithm::moneyMaker& aMM) const;

	std::string time;
	double price = 0.0;
	double stopLoss = 0.0;
	double minimumProfit = 0.0;
	double margin = 0.0;
	double notionalValue = 0.0;
	double quantity = 0.0;
	algorithm::eState state;
	bool fullCheck = false;
};