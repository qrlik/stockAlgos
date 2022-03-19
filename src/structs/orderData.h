#pragma once
#include "../../third_party/json.hpp"
#include <string>

struct orderData {
	static void initOrderDataFromJson(orderData& aData, const Json& aJson);

	bool operator==(const orderData& aOther);
	double getLiquidationPrice() const;
	std::string toString() const;

	std::string time;
	double price = 0.0;
	double stopLoss = 0.0;
	double minimumProfit = 0.0;
	double margin = 0.0;
	double notionalValue = 0.0;
	double quantity = 0.0;
	bool fullCheck = false;
};