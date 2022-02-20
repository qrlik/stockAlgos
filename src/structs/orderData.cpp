#include "orderData.h"
#include <sstream>
#include <iomanip>

std::string orderData::toString() const {
	std::ostringstream ostr;
	ostr << std::setw(6) << "p: " << std::setw(12) << std::to_string(price)
		<< std::setw(6) << "s: " << std::setw(12) << std::to_string(stopLoss)
		<< std::setw(6) << "a: " << std::setw(12) << std::to_string(amount);
	return ostr.str();
}