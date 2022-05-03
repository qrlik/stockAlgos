#pragma once
#include "json/json.hpp"
#include <fstream>

namespace calculation {
	void addStats(Json& aStats, const Json& aData, double aWeight);
	void saveStats(Json& aStats, const std::string& aFileName);
	void addHeadlines(std::ofstream& aOutput, const Json& aStats, const Json& aExample);
	void addData(std::ofstream& aOutput, const Json& aStats, const Json& aData);
}
