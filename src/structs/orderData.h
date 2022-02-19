#pragma once
#include <string>

struct orderData {
	std::string time;
	double price = 0.0;
	double stopLoss = 0.0;
	double minimumProfit = 0.0;
	double amount = 0.0;
	bool fullCheck = false;
};