#pragma once
#include "algorithm/algorithmDataBase.h"

namespace algorithm {
	class stMAlgorithmData final : public algorithmDataBase {
	public:

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

	};
}