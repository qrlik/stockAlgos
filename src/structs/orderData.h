#pragma once
#include "../../third_party/json.hpp"
#include <string>

struct orderData {
	static void initOrderDataFromJson(orderData& aData, const Json& aJson);

	bool operator==(const orderData& aOther);
	std::string toString() const;
	std::string time;
	double price = 0.0;
	double stopLoss = 0.0;
	double minimumProfit = 0.0;
	double amount = 0.0;
	bool fullCheck = false;
};