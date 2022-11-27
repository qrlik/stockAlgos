#pragma once
#include "algorithm/order.h"

namespace algorithm {
	class stMAlgorithm;
	class closerModule {
		friend class stMAlgorithm;
	public:
		closerModule(stMAlgorithm& aAlgorithm);
		bool check();

	private:
		const order& getOrder() const;

		stMAlgorithm& algorithm;
	};
}
