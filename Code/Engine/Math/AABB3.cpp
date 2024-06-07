#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/EngineCommon.hpp"

AABB3::AABB3( AABB3 const& copyForm )
	: m_mins( copyForm.m_mins )
	, m_maxs( copyForm.m_maxs )
{
}

AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
{
	m_mins = Vec3( minX, minY, minZ );
	m_maxs = Vec3( maxX, maxY, maxZ );
}

AABB3::AABB3( Vec3 const& mins, Vec3 const& maxs )
	: m_mins( mins )
	, m_maxs( maxs )
{
}

AABB3 const AABB3::GetMoved( Vec3 const& delta ) const
{
	return AABB3( m_mins + delta, m_maxs + delta );
}
