#pragma once

namespace algorithm {
	class stMAlgorithm;
	class maDirectionModule {
		friend class stMAlgorithm;
	public:
		maDirectionModule(stMAlgorithm& aAlgorithm);
		bool update();
	private:
		stMAlgorithm& algorithm;
	};
}
