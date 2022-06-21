#pragma once
#include "json/json.hpp"
#include <string>
#include <vector>

namespace calculation {
	struct predicate {
		std::string field;
		Json value;
		std::string operand;
	};

	struct predicatesData {
		std::vector<predicate> keepPredicates;
		std::vector<predicate> deletePredicates;
	};

	void processFilter();
}
