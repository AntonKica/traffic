#include "SimulationAreaObjectStatic.h"
#include <limits>
#include <type_traits>

template<typename NumType> bool numberIsApproximitelyEqual(NumType num, NumType approximitation, NumType maxDifferrence = std::numeric_limits<NumType>::epsilon())
{
	if (!std::is_arithmetic<NumType>::value)
		static_assert("NumType is not a number");

	return std::abs(num) - std::abs(approximitation) <= maxDifferrence;
}

template<typename NumType> bool numberIsApproximitelyGreater(NumType num, NumType greaterThan, NumType maxDifferrence = std::numeric_limits<NumType>::epsilon())
{
	if (!std::is_arithmetic<NumType>::value)
		static_assert("NumType is not a number");

	return (std::abs(num) - std::abs(maxDifferrence) > std::abs(greaterThan)) || std::abs(num) + std::abs(maxDifferrence) > std::abs(greaterThan);
}

template<typename NumType> bool numberIsApproximitelyLower(NumType num, NumType greaterThan, NumType maxDifferrence = std::numeric_limits<NumType>::epsilon())
{
	if (!std::is_arithmetic<NumType>::value)
		static_assert("NumType is not a number");

	return (std::abs(num) - std::abs(maxDifferrence) < std::abs(greaterThan)) || std::abs(num) + std::abs(maxDifferrence) < std::abs(greaterThan);
}

StaticAdjacencyFlags SimulationAreaObjectStatic::getObjectAdjacency(const SimulationAreaObjectStatic& other) const
{
	const auto pos1 = getPosition();
	const auto pos2 = other.getPosition();

	const auto posDiff = glm::abs(pos1 - pos2);
	const auto diffLength = glm::length(posDiff);

	StaticAdjacencyFlags flags{};
	if (numberIsApproximitelyEqual(diffLength, 1.0f))
		flags |= StaticAdjacencyBits::Neighbour;
	else
		flags |= StaticAdjacencyBits::Distant;

	// we suppose that pos1 != pos2
	if (posDiff.x != 0 && posDiff.y == 0 && posDiff.z == 0)
	{
		flags |= StaticAdjacencyBits::AxialX |
			StaticAdjacencyBits::Axial;
	}
	if (posDiff.x == 0 && posDiff.y != 0 && posDiff.z == 0)
	{
		flags |= StaticAdjacencyBits::AxialY |
			StaticAdjacencyBits::Axial;
	}
	if (posDiff.x == 0 && posDiff.y == 0 && posDiff.z != 0)
	{
		flags |= StaticAdjacencyBits::AxialZ |
			StaticAdjacencyBits::Axial;
	}

	return flags;
}