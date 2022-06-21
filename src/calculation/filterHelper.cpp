#include "filterHelper.h"
#include "utils/utils.h"

using namespace calculation;

namespace {
	bool checkOperand(const std::string& aOperand) {
		if (aOperand == "=="
			|| aOperand == "!="
			|| aOperand == ">"
			|| aOperand == ">="
			|| aOperand == "<"
			|| aOperand == "<=")
			return true;
		return false;
	}

	bool checkPredicates(const Json& aPredicates) {
		bool result = true;
		result &= aPredicates.is_array();
		for (const auto& predicate : aPredicates) {
			result &= predicate.is_object();
			result &= predicate.contains("field") && predicate["field"].is_string();
			result &= predicate.contains("value") && !predicate["value"].is_null();
			result &= predicate.contains("operand") && predicate["operand"].is_string()
				&& checkOperand(predicate["operand"].get<std::string>());
		}
		return result;
	}

	bool checkSettingsJson(const Json& aSettings) {
		bool result = true;
		result &= aSettings.is_object();
		result &= aSettings.contains("keepPredicates") || aSettings.contains("deletePredicates");

		if (aSettings.contains("keepPredicates")) {
			result &= checkPredicates(aSettings["keepPredicates"]);
		}
		if (aSettings.contains("deletePredicates")) {
			result &= checkPredicates(aSettings["deletePredicates"]);
		}
		return result;
	}

	predicatesData parsePredicates(const Json& aData) {
		predicatesData result;
		const auto fillPredicates = [](std::vector<predicate>& aContainer, const Json& aValue) {
			for (const auto& pred : aValue) {
				predicate p;
				p.field = pred["field"].get<std::string>();
				p.value = pred["value"];
				p.operand = pred["operand"].get<std::string>();
				aContainer.push_back(std::move(p));
			}
		};
		fillPredicates(result.keepPredicates, aData["keepPredicates"]);
		fillPredicates(result.deletePredicates, aData["deletePredicates"]);
		return result;
	}
}

void calculation::processFilter() {
	auto settings = utils::readFromJson("input/filterSettings");
	if (!checkSettingsJson(settings)) {
		utils::logError("calculation::processFilter settings parse error");
		return;
	}
	const auto predicates = parsePredicates(settings);
	const auto fullData = utils::readFromJson("input/jsonDataAll");
}