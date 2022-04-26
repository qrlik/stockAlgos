#pragma once

namespace algorithm {
	template<typename dataType>
	class algorithmBase {
	public:
		algorithmBase(const dataType& aData) : data(aData) {}
		const dataType& getData() const { return data; }
	private:
		const dataType data;
	};
}