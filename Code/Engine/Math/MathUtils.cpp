#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

unsigned int RoundToPrevPower2( unsigned int base )
{
	base--;
	base |= base >> 1;
	base |= base >> 2;
	base |= base >> 4;
	base |= base >> 8;
	base |= base >> 16;
	base = base >> 1;
	base++;
	return base;
}

unsigned int RoundToNextPower2( unsigned int base )
{
	base--;
	base |= base >> 1;
	base |= base >> 2;
	base |= base >> 4;
	base |= base >> 8;
	base |= base >> 16;
	base++;
	return base;
}

float Power( float base, unsigned int exp )
{
	float result = 1.f;
	for (;;)
	{
		if (exp & 1)
		{
			result *= base;
		}
		exp >>= 1;

		if (!exp)
			break;

		base *= base;
	}

	return result;
}

float GetUnitDegree( float degrees )
{
	while (degrees > 180.f)
		degrees -= 360.f;
	while (degrees < -180.f)
		degrees += 360.f;
	return degrees;
}

float ConvertDegreesToRadians( float degrees )
{
	return degrees * PI / 180.f;
}

float ConvertRadiansToDegrees( float radians )
{
	return radians * 180.f / PI;
}

float CosDegrees( float degrees )
{
	return cosf( ConvertDegreesToRadians( degrees ) );
}

float SinDegrees( float degrees )
{
	return sinf( ConvertDegreesToRadians( degrees ) );
}

float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}

float GetShortestAngularDispDegrees( float startDegrees, float endDegrees )
{
	float rawDisp = endDegrees - startDegrees;
	return GetUnitDegree( rawDisp );
}

float GetTurnedTowardDegrees( float currentDegrees, float goalDegrees, float maxDeltaDegrees )
{
	float targetDisp = GetShortestAngularDispDegrees( currentDegrees, goalDegrees );
	if (targetDisp < 0)
		return GetUnitDegree( currentDegrees - MIN( maxDeltaDegrees, -targetDisp ) );
	return GetUnitDegree( currentDegrees + MIN( maxDeltaDegrees, targetDisp ) );
}

float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b )
{
	float dotP2D = DotProduct2D( a, b );
	float thetaRadians = acosf( dotP2D / (a.GetLength() * b.GetLength()) );
	return ConvertRadiansToDegrees( thetaRadians );
}

float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB )
{
	return sqrtf( GetDistanceSquared2D( positionA, positionB ) );
}

float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	return deltaX * deltaX + deltaY * deltaY;
}

float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( GetDistanceSquared3D( positionA, positionB ) );
}

float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	float deltaZ = positionA.z - positionB.z;
	return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}

float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB )
{
	return sqrtf( GetDistanceXYSquared3D( positionA, positionB ) );
}

float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB )
{
	float deltaX = positionA.x - positionB.x;
	float deltaY = positionA.y - positionB.y;
	return deltaX * deltaX + deltaY * deltaY;
}

int GetTaxicabDistance2D( IntVec2 const& pointA, IntVec2 const& pointB )
{
	return abs( pointA.x - pointB.x ) + abs( pointA.y - pointB.y );
 }

float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto )
{
	return DotProduct2D( vectorToProject, vecToProjectOnto ) / vecToProjectOnto.GetLength();
}

Vec2 const GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vecToProjectOnto )
{
	return vecToProjectOnto.GetNormalized() * GetProjectedLength2D( vectorToProject, vecToProjectOnto );
}

float GetProjectedLength3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto )
{
	return DotProduct3D( vectorToProject, vecToProjectOnto ) / vecToProjectOnto.GetLength();
}

Vec3 const GetProjectedOnto3D( Vec3 const& vectorToProject, Vec3 const& vecToProjectOnto )
{
	return vecToProjectOnto.GetNormalized() * GetProjectedLength3D( vectorToProject, vecToProjectOnto );
}

bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius )
{
	float toCenterVectorLengthSquared = (point - discCenter).GetLengthSquared();
	return toCenterVectorLengthSquared <= discRadius * discRadius;
}

bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box )
{
	if (point.x >= box.m_maxs.x)
		return false;
	if (point.x <= box.m_mins.x)
		return false;
	if (point.y >= box.m_maxs.y)
		return false;
	if (point.y <= box.m_mins.y)
		return false;
	return true;
}

bool IsPointInsideCapsule2D( Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	Vec2 boneVector = boneEnd - boneStart;

	Vec2 startToPointVector = point - boneStart;
	if (DotProduct2D( boneVector, startToPointVector ) <= 0)
		return (startToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec2 endToPointVector = point - boneEnd;
	if (DotProduct2D( boneVector, endToPointVector ) >= 0)
		return (endToPointVector.GetLengthSquared() >= radius * radius) ? false : true;

	Vec2 boneVectorNormal = boneVector.GetNormalized();
	float distToLine = DotProduct2D( startToPointVector, Vec2( -boneVectorNormal.y, boneVectorNormal.x ) );
	return (distToLine >= radius || distToLine <= -radius) ? false : true;
}

bool IsPointInsideOBB2D( Vec2 const& point, OBB2 const& box )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );

	Vec2 centerToPoint = point - box.m_center;
	float xLocal = DotProduct2D( centerToPoint, iBasis );
	if (xLocal >= box.m_halfDimensions.x || xLocal <= -box.m_halfDimensions.x)
		return false;
	float yLocal = DotProduct2D( centerToPoint, jBasis );
	if (yLocal >= box.m_halfDimensions.y || yLocal <= -box.m_halfDimensions.y)
		return false;
	return true;
}

bool IsPointInsideOBB3D( Vec3 const& point, OBB3 const& box )
{
	Vec3 localPos = box.WorldPosToLocalPos( point );

	return IsPointInsideAABB3D( localPos, box.GetLocalAABB3() );
}

bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 tipToPointVector = point - sectorTip;
	float tipToPointLengthSquared = tipToPointVector.GetLengthSquared();
	if (tipToPointLengthSquared > sectorRadius * sectorRadius)
		return false;
	float tipToPointDegree = tipToPointVector.GetOrientationDegrees();
	float shortestDegreeToForward = GetShortestAngularDispDegrees( sectorForwardDegrees, tipToPointDegree );
	return shortestDegreeToForward <= sectorApertureDegrees * 0.5f && shortestDegreeToForward >= -sectorApertureDegrees * 0.5f;
}

bool IsPointInsideDirectedSector2D( Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	Vec2 tipToPointVector = point - sectorTip;
	float tipToPointLengthSquared = tipToPointVector.GetLengthSquared();
	if (tipToPointLengthSquared > sectorRadius * sectorRadius)
		return false;
	Vec2 tipToPointNormal = tipToPointVector.GetNormalized();
	return ConvertRadiansToDegrees( acosf( DotProduct2D( tipToPointNormal, sectorForwardNormal ) ) ) < sectorApertureDegrees * 0.5f;
}

bool IsPointInsideSphere3D( Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 centerToPointVec = point - sphereCenter;
	return centerToPointVec.GetLength() < sphereRadius;
}

bool IsPointInsideAABB3D( Vec3 const& point, AABB3 const& box )
{
	if (point.x >= box.m_maxs.x)
		return false;
	if (point.x <= box.m_mins.x)
		return false;
	if (point.y >= box.m_maxs.y)
		return false;
	if (point.y <= box.m_mins.y)
		return false;
	if (point.z >= box.m_maxs.z)
		return false;
	if (point.z <= box.m_mins.z)
		return false;
	return true;
}

bool DoDiscsOverlap2D( Vec2 const& centerA, float const& radiusA, Vec2 const& centerB, float const& radiusB )
{
	return GetDistanceSquared2D( centerA, centerB ) < ((radiusA + radiusB) * (radiusA + radiusB));
}

bool DoAABBsOverlap2D( AABB2 const& aabbA, AABB2 const& aabbB )
{
	Vec2 AX = Vec2( aabbA.m_mins.x, aabbA.m_maxs.x );
	Vec2 AY = Vec2( aabbA.m_mins.y, aabbA.m_maxs.y );
	Vec2 BX = Vec2( aabbB.m_mins.x, aabbB.m_maxs.x );
	Vec2 BY = Vec2( aabbB.m_mins.y, aabbB.m_maxs.y );

	float a = MAX( AX.x, BX.x );
	float b = MIN( AX.y, BX.y );
	bool xOverlap = (a < b);
	a = MAX( AY.x, BY.x );
	b = MIN( AY.y, BY.y );
	bool yOverlap = (a < b);
	return xOverlap && yOverlap;
}

bool DoDiscOverlapAABB2D( AABB2 const& aabb, Vec2 const& center, float const& radius )
{
	if (IsPointInsideAABB2D( center, aabb ))
		return true;

	Vec2 point = GetNearestPointOnAABB2D( center, aabb );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoDiscOverlapOBB2D( Vec2 const& discCenter, float discRadius, OBB2 const& box )
{
	OBB2 fatOBB = box;
	fatOBB.m_halfDimensions += Vec2( discRadius, discRadius );
	return IsPointInsideOBB2D( discCenter, fatOBB );
}

bool DoDiscOverlapCapsule2D( Vec2 const& discCenter, float discRadius, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	return IsPointInsideCapsule2D( discCenter, boneStart, boneEnd, radius + discRadius );
}

bool DoDiscOverlapDirectedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius )
{
	return IsPointInsideOrientedSector2D( discCenter, sectorTip, sectorForwardDegrees, sectorApertureDegrees, sectorRadius + discRadius );
}

bool DoDiscOverlapOrientedSector2D( Vec2 const& discCenter, float discRadius, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius )
{
	return IsPointInsideDirectedSector2D( discCenter, sectorTip, sectorForwardNormal, sectorApertureDegrees, sectorRadius + discRadius );
}

bool DoSpheresOverlap3D( Vec3 const& centerA, float const& radiusA, Vec3 const& centerB, float const& radiusB )
{
	return GetDistanceSquared3D( centerA, centerB ) < ((radiusA + radiusB) * (radiusA + radiusB));
}

bool DoAABBsOverlap3D( AABB3 const& aabbA, AABB3 const& aabbB )
{
	Vec2 AX = Vec2( aabbA.m_mins.x, aabbA.m_maxs.x );
	Vec2 AY = Vec2( aabbA.m_mins.y, aabbA.m_maxs.y );
	Vec2 AZ = Vec2( aabbA.m_mins.z, aabbA.m_maxs.z );
	Vec2 BX = Vec2( aabbB.m_mins.x, aabbB.m_maxs.x );
	Vec2 BY = Vec2( aabbB.m_mins.y, aabbB.m_maxs.y );
	Vec2 BZ = Vec2( aabbB.m_mins.z, aabbB.m_maxs.z );

	float a = MAX( AX.x, BX.x );
	float b = MIN( AX.y, BX.y );
	bool xOverlap = (a < b);
	a = MAX( AY.x, BY.x );
	b = MIN( AY.y, BY.y );
	bool yOverlap = (a < b);
	a = MAX( AZ.x, BZ.x );
	b = MIN( AZ.y, BZ.y );
	bool ZOverlap = (a < b);
	return xOverlap && yOverlap && ZOverlap;
}

bool DoZCylindersOverlap3D( Vec3 const& cylinderAStart, float cylinderAHeight, float cylinderARadius, Vec3 const& cylinderBStart, float cylinderBHeight, float cylinderBRadius )
{
	float actorABottom = cylinderAStart.z;
	float actorATop = actorABottom + cylinderAHeight;
	float actorBBottom = cylinderBStart.z;
	float actorBTop = actorBBottom + cylinderBHeight;

	Vec2 actorACenter = cylinderAStart.GetXY();
	Vec2 actorBCenter = cylinderBStart.GetXY();

	return actorATop > actorBBottom &&
		actorABottom < actorBTop&&
		DoDiscsOverlap2D( actorACenter, cylinderARadius, actorBCenter, cylinderBRadius );
}

bool DoSphereAndAABBOverlap3D( Vec3 const& center, float const& radius, AABB3 const& aabb )
{
	Vec3 point = GetNearestPointOnAABB3D( center, aabb );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoZCylinderAndAABBOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, AABB3 const& aabb )
{
	Vec2 discPos = cylinderStart.GetXY();
	FloatRange cylinderZRange( cylinderStart.z, cylinderStart.z + cylinderHeight );
	FloatRange aabbZRange( aabb.m_mins.z, aabb.m_maxs.z );
	AABB2 aabb2( aabb.m_mins.GetXY(), aabb.m_maxs.GetXY() );
	return DoFloatRangeOverlap( cylinderZRange, aabbZRange ) && DoDiscOverlapAABB2D( aabb2, discPos, cylinderRadius );
}

bool DoZCylinderAndSphereOverlap3D( Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius, Vec3 const& center, float const& radius )
{
	Vec3 point = GetNearestPointOnZCylinder3D( center, cylinderStart, cylinderHeight, cylinderRadius );
	return (point - center).GetLengthSquared() < radius * radius;
}

bool DoPlaneOverlapZCylinder3D( Plane const& plane, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius )
{
	Vec3 planeNormalXY = Vec3( plane.m_normal.GetXY().GetNormalized() );

	std::vector<Vec3>intercepts;
	intercepts.resize( 4 );
	intercepts[0] = planeNormalXY * cylinderRadius + cylinderStart;
	intercepts[1] = -planeNormalXY * cylinderRadius + cylinderStart;
	intercepts[2] = planeNormalXY * cylinderRadius + cylinderStart + Vec3( 0.f, 0.f, cylinderHeight );
	intercepts[3] = -planeNormalXY * cylinderRadius + cylinderStart + Vec3( 0.f, 0.f, cylinderHeight );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 4; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool DoPlaneOverlapAABB3D( Plane const& plane, AABB3 const& aabb )
{
	std::vector<Vec3>intercepts;
	intercepts.resize( 8 );
	intercepts[0] = Vec3( aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z );
	intercepts[1] = Vec3( aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z );
	intercepts[2] = Vec3( aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z );
	intercepts[3] = Vec3( aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z );
	intercepts[4] = Vec3( aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z );
	intercepts[5] = Vec3( aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z );
	intercepts[6] = Vec3( aabb.m_mins.x, aabb.m_mins.y, aabb.m_mins.z );
	intercepts[7] = Vec3( aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 8; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

bool DoPlaneOverlapSphere3D( Plane const& plane, Vec3 const& center, float radius )
{
	return abs( plane.GetDistanceOfPoint( center ) ) < radius;
}

bool DoPlaneOverlapOBB3D( Plane const& plane, OBB3 const& obb )
{
	std::vector<Vec3>intercepts;
	intercepts.resize( 8 );
	intercepts[0] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[1] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[2] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[3] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y + obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[4] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[5] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x + obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[6] = Vec3 ( obb.m_center - obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	intercepts[7] = Vec3 ( obb.m_center + obb.m_iBasisNormal * obb.m_halfDimensions.x - obb.m_jBasisNormal * obb.m_halfDimensions.y - obb.m_kBasisNormal * obb.m_halfDimensions.z );
	float dist = plane.GetDistanceOfPoint( intercepts[0] );
	for (int i = 1; i < 8; i++)
	{
		float thisDist = plane.GetDistanceOfPoint( intercepts[i] );
		if (dist > 0.f && thisDist < 0.f ||
			dist < 0.f && thisDist > 0.f)
		{
			return true;
		}
	}
	return false;
}

Vec2 const GetNearestPointOnDisc2D( Vec2 const& referencePos, Vec2 const& discCenter, float discRadius )
{
	Vec2 centerToPointVector = referencePos - discCenter;
	Vec2 centerToPointNormal = centerToPointVector.GetNormalized();
	float centerToPointLength = centerToPointVector.GetLength();
	if (centerToPointLength <= discRadius)
		return referencePos;
	return discCenter + centerToPointNormal * Clamp( centerToPointLength, 0.f, discRadius );
}

Vec2 const GetNearestPointOnAABB2D( Vec2 const& referencePos, AABB2 const& box )
{
	return Vec2(
		Clamp( referencePos.x, box.m_mins.x, box.m_maxs.x ),
		Clamp( referencePos.y, box.m_mins.y, box.m_maxs.y )
	);
}

Vec2 const GetNearestPointOnInfiniteLine2D( Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine )
{
	Vec2 lineVector = anotherPointOnLine - pointOnLine;
	Vec2 startToPointVector = referencePos - pointOnLine;
	Vec2 lineVectorNormal = lineVector.GetNormalized();
	return pointOnLine + lineVectorNormal * DotProduct2D( startToPointVector, lineVectorNormal );
}

Vec2 const GetNearestPointOnLineSegment2D( Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd )
{
	Vec2 lineVector = lineSegEnd - lineSegStart;

	Vec2 startToPointVector = referencePos - lineSegStart;

	if (DotProduct2D( lineVector, startToPointVector ) <= 0)
		return lineSegStart;

	Vec2 endToPointVector = referencePos - lineSegEnd;
	if (DotProduct2D( lineVector, endToPointVector ) >= 0)
		return lineSegEnd;

	Vec2 lineVectorNormal = lineVector.GetNormalized();
	return lineSegStart + lineVectorNormal * DotProduct2D( startToPointVector, lineVectorNormal );
}

Vec2 const GetNearestPointOnCapsule2D( Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius )
{
	Vec2 boneVector = boneEnd - boneStart;

	Vec2 startToPointVector = referencePos - boneStart;
	if (DotProduct2D( boneVector, startToPointVector ) <= 0)
		return boneStart + startToPointVector.GetClamped( radius );

	Vec2 endToPointVector = referencePos - boneEnd;
	if (DotProduct2D( boneVector, endToPointVector ) >= 0)
		return boneEnd + endToPointVector.GetClamped( radius );

	Vec2 boneVectorNormal = boneVector.GetNormalized();
	Vec2 nearestPointOnLine = boneStart + boneVectorNormal * DotProduct2D( startToPointVector, boneVectorNormal );
	float distToLine = DotProduct2D( startToPointVector, Vec2( -boneVectorNormal.y, boneVectorNormal.x ) );
	return nearestPointOnLine + Clamp( distToLine, -radius, radius ) * Vec2( -boneVectorNormal.y, boneVectorNormal.x );
}

Vec2 const GetNearestPointOnOBB2D( Vec2 const& referencePos, OBB2 const& box )
{
	Vec2 iBasis = box.m_iBasisNormal;
	Vec2 jBasis = Vec2( -iBasis.y, iBasis.x );

	Vec2 centerToPoint = referencePos - box.m_center;
	float xLocal = DotProduct2D( centerToPoint, iBasis );
	float yLocal = DotProduct2D( centerToPoint, jBasis );
	return box.m_center + Clamp( xLocal, -box.m_halfDimensions.x, box.m_halfDimensions.x ) * iBasis + Clamp( yLocal, -box.m_halfDimensions.y, box.m_halfDimensions.y ) * jBasis;
}

Vec3 const GetNearestPointOnSphere3D( Vec3 const& referencePos, Vec3 const& sphereCenter, float sphereRadius )
{
	Vec3 centerToPointVector = referencePos - sphereCenter;
	float centerToPointLength = centerToPointVector.GetLength();

	if (centerToPointLength <= sphereRadius)
	{
		return referencePos;
	}

	Vec3 centerToPointNormal = centerToPointVector / centerToPointLength;
	return sphereCenter + centerToPointNormal * sphereRadius;
}

Vec3 const GetNearestPointOnAABB3D( Vec3 const& referencePos, AABB3 const& box )
{
	return Vec3(
		Clamp( referencePos.x, box.m_mins.x, box.m_maxs.x ),
		Clamp( referencePos.y, box.m_mins.y, box.m_maxs.y ),
		Clamp( referencePos.z, box.m_mins.z, box.m_maxs.z )
	);
}

Vec3 const GetNearestPointOnZCylinder3D( Vec3 const& referencePos, Vec3 const& cylinderStart, float cylinderHeight, float cylinderRadius )
{
	Vec2 nearestDisc2D = GetNearestPointOnDisc2D( referencePos.GetXY(), cylinderStart.GetXY(), cylinderRadius );
	float nearestZ = Clamp( referencePos.z, cylinderStart.z, cylinderStart.z + cylinderHeight );
	return Vec3( nearestDisc2D.x, nearestDisc2D.y, nearestZ );
}

Mat44 const GetLookAtMatrix( Vec3 forwardVec )
{
	Vec3 iBasis = forwardVec.GetNormalized();
	Vec3 jBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 kBasis = Vec3( 0.f, 0.f, 1.f );
	if (iBasis != Vec3( 0.f, 0.f, 1.f ))
	{
		jBasis = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), iBasis ).GetNormalized();
		kBasis = CrossProduct3D( iBasis, jBasis );
	}
	return Mat44( iBasis, jBasis, kBasis, Vec3::ZERO );
}

Mat44 const GetBillboardMatrix( BillboardType billboardType, Mat44 const& cameraMatrix, const Vec3& billboardPosition, const Vec2& billboardScale )
{
	Mat44 transformMat;
	if (billboardType == BillboardType::FULL_FACING)
	{
		Vec3 cameraPos = cameraMatrix.GetTranslation3D();
		Vec3 textToCameraVec = cameraPos - billboardPosition;
		transformMat = GetLookAtMatrix( textToCameraVec );
	}
	else if (billboardType == BillboardType::FULL_OPPOSING)
	{
		transformMat.SetIJK3D( -cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D(), cameraMatrix.GetKBasis3D() );
	}
	else if (billboardType == BillboardType::WORLD_UP_FACING)
	{
		Vec3 cameraPos = cameraMatrix.GetTranslation3D();
		Vec3 textToCameraVec = cameraPos - billboardPosition;
		transformMat = GetLookAtMatrix( textToCameraVec );
		transformMat.SetIJK3D( transformMat.GetIBasis3D(), transformMat.GetJBasis3D(), Vec3( 0.f, 0.f, 1.f ) );
	}
	else if (billboardType == BillboardType::WORLD_UP_OPPOSING)
	{
		transformMat.SetIJK3D( -cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D(), Vec3( 0.f, 0.f, 1.f ) );
	}
	else if (billboardType == BillboardType::NONE)
	{

	}
	transformMat.SetTranslation3D( billboardPosition );
	transformMat.AppendScaleNonUniform2D( billboardScale );
	return transformMat;
}

bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedpoint )
{
	Vec2 pointToCenterVector = mobileDiscCenter - fixedpoint;
	float pointToCenterLength = pointToCenterVector.GetLength();
	if (pointToCenterLength <= discRadius)
	{
		Vec2 pointToCenterNormal = pointToCenterVector.GetNormalized();
		mobileDiscCenter += pointToCenterNormal * (discRadius - pointToCenterLength);
		return true;
	}
	return false;
}

bool PushDiscOutOfFixedDisc2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius )
{
	Vec2 fixedToMobileVector = mobileDiscCenter - fixedDiscCenter;
	float fixedToMobileLength = fixedToMobileVector.GetLength();
	if (fixedToMobileLength <= mobileDiscRadius + fixedDiscRadius)
	{
		Vec2 pointToCenterNormal = fixedToMobileVector.GetNormalized();
		mobileDiscCenter += pointToCenterNormal * (mobileDiscRadius + fixedDiscRadius - fixedToMobileLength);
		return true;
	}
	return false;
}

bool PushDiscsOutOfEachOther2D( Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius )
{
	Vec2 fixedToMobileVector = aCenter - bCenter;
	float fixedToMobileLength = fixedToMobileVector.GetLength();
	if (fixedToMobileLength <= aRadius + bRadius)
	{
		Vec2 pointToCenterNormal = fixedToMobileVector.GetNormalized();
		float moveDistTotal = aRadius + bRadius - fixedToMobileLength;
		aCenter += pointToCenterNormal * (moveDistTotal * 0.5f);
		bCenter += -pointToCenterNormal * (moveDistTotal * 0.5f);
		return true;
	}
	return false;
}

bool PushDiscOutOfFixedAABB2D( Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox )
{
	Vec2 centerToNearestPointOnAABBVector = mobileDiscCenter - fixedBox.GetNearestPoint( mobileDiscCenter );
	float centerToNearestPointOnAABBLength = centerToNearestPointOnAABBVector.GetLength();
	if (centerToNearestPointOnAABBLength <= discRadius)
	{
		Vec2 centerToNearestPointOnAABBNormal = centerToNearestPointOnAABBVector.GetNormalized();
		mobileDiscCenter += centerToNearestPointOnAABBNormal * discRadius - centerToNearestPointOnAABBVector;
		return true;
	}
	return false;
}

bool BounceDiscsOffPoint( Vec2& center, float radius, Vec2& velocity, Vec2 const& point, float elasticity )
{
	if (!PushDiscOutOfFixedPoint2D( center, radius, point ))
	{
		return false;
	}
	Vec2 discToPointVec = (point - center).GetNormalized();
	Vec2 Vn = DotProduct2D( velocity, discToPointVec ) * discToPointVec;
	Vec2 Vt = velocity - Vn;
	velocity = Vt - Vn * elasticity;
	return true;
}

bool BounceDiscsOffEachOther( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticity )
{
	if (!PushDiscsOutOfEachOther2D( centerA, radiusA, centerB, radiusB ))
	{
		return false;
	}
	
	Vec2 aToBVec = centerB - centerA;
	Vec2 aToBVecNormal = aToBVec.GetNormalized();
	float dotVnA = DotProduct2D( velocityA, aToBVecNormal );
	Vec2 VnA = dotVnA * aToBVecNormal;
	Vec2 VtA = velocityA - VnA;
	float dotVnB = DotProduct2D( velocityB, aToBVecNormal );
	Vec2 VnB = dotVnB * aToBVecNormal;
	Vec2 VtB = velocityB - VnB;

	if (dotVnB < dotVnA)
	{
		velocityA = VtA + VnB * elasticity;
		velocityB = VtB + VnA * elasticity;
		return true;
	}

	return false;
}

bool BounceDiscOffDisc( Vec2& centerA, float radiusA, Vec2& velocityA, Vec2 const& centerB, float radiusB, float elasticity )
{
	//Vec2 closestOnDisc = GetNearestPointOnDisc2D( centerA, centerB, radiusB );
	return BounceDiscsOffPoint( centerA, radiusA + radiusB, velocityA, centerB, elasticity );
}

bool BounceDiscOffCapsule( Vec2& center, float radius, Vec2& velocity, Vec2 const& boneStart, Vec2 const& boneEnd, float radiusCapsule, float elasticity )
{
	//Vec2 closestOnCapsule = GetNearestPointOnCapsule2D( center, boneStart, boneEnd, radiusCapsule );
	Vec2 closestOnCapsule = GetNearestPointOnLineSegment2D( center, boneStart, boneEnd );
	return BounceDiscsOffPoint( center, radius + radiusCapsule, velocity, closestOnCapsule, elasticity );
	//return BounceDiscsOffPoint( center, radius, velocity, closestOnCapsule, elasticity );
}

bool BounceDiscOffOBB( Vec2& center, float radius, Vec2& velocity, OBB2 const& box, float elasticity )
{
	Vec2 closestOnOBB = GetNearestPointOnOBB2D( center, box );
	return BounceDiscsOffPoint( center, radius, velocity, closestOnOBB, elasticity );
}

void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation )
{
	if (uniformScale != 1.f)
	{
		posToTransform *= uniformScale;
	}
	if (rotationDegrees != 0.f)
	{
		posToTransform.RotateDegrees( rotationDegrees );
;	}
	if (translation != Vec2( 0.f, 0.f ))
	{
		posToTransform += translation;
	}
}

void TransformPositionXY3D( Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY )
{
	if (scaleXY != 1.f)
	{
		posToTransform = Vec3( posToTransform.x * scaleXY, posToTransform.y * scaleXY, posToTransform.z );
	}
	if (zRotationDegrees != 0.f)
	{
		posToTransform = posToTransform.GetRotatedAboutZDegrees( zRotationDegrees );
	}
	if (translationXY != Vec2( 0.f, 0.f ))
	{
		posToTransform += Vec3( translationXY.x, translationXY.y, 0.f );
	}
}

void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation )
{
	posToTransform = Vec2(
		posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translation.x,
		posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translation.y
	);
}

void TransformPositionXY3D( Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY )
{
	posToTransform = Vec3(
		posToTransform.x * iBasis.x + posToTransform.y * jBasis.x + translationXY.x,
		posToTransform.x * iBasis.y + posToTransform.y * jBasis.y + translationXY.y,
		posToTransform.z
	);
}

float DotProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.x + a.y * b.y;
}

float DotProduct3D( Vec3 const& a, Vec3 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D( Vec4 const& a, Vec4 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float CrossProduct2D( Vec2 const& a, Vec2 const& b )
{
	return a.x * b.y - a.y * b.x;
}

Vec3 CrossProduct3D( Vec3 const& a, Vec3 const& b)
{
	return Vec3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

float Clamp( float value, float minValue, float maxValue )
{
	if (value < minValue)
		return minValue;
	else if (value > maxValue)
		return maxValue;
	return value;
}

int ClampInt( int value, int minValue, int maxValue )
{
	if (value < minValue)
		return minValue;
	else if (value > maxValue)
		return maxValue;
	return value;
}

float ClampZeroToOne( float value )
{
	return Clamp( value, 0.f, 1.f );
}

float Interpolate( float start, float end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

Vec2 Interpolate( Vec2 start, Vec2 end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

Vec3 Interpolate( Vec3 start, Vec3 end, float fractionTowardEnd )
{
	return (end - start) * fractionTowardEnd + start;
}

float GetFractionWithinRange( float value, float rangeStart, float rangeEnd )
{
	return (value - rangeStart) / (rangeEnd - rangeStart);
}

float RangeMap( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	return Interpolate( outStart, outEnd, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

float RangeMapZeroToOne( float inValue, float inStart, float inEnd )
{
	return Interpolate( 0.f, 1.0f, GetFractionWithinRange( inValue, inStart, inEnd ) );
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	return Interpolate( outStart, outEnd, GetFractionWithinRange( Clamp( inValue, inStart, inEnd ), inStart, inEnd ) );
}

int RoundDownToInt( float value )
{
	return (int)floorf( value );
}

float NormalizeByte( unsigned char targetByte )
{
	float rawFloat = static_cast<float>(targetByte);
	return RangeMap( rawFloat, 0.f, 255.f, 0.f, 1.f );
}

unsigned char DenormalizeByte( float targetFloat )
{
	return static_cast<unsigned char>(Clamp( targetFloat * 256.f, 0.f, 255.f ));
}

float ComputeCubicBezier1D( float A, float B, float C, float D, float t )
{
	float AB = Interpolate( A, B, t );
	float BC = Interpolate( B, C, t );
	float CD = Interpolate( C, D, t );

	float ABC = Interpolate( AB, BC, t );
	float BCD = Interpolate( BC, CD, t );

	float ABCD = Interpolate( ABC, BCD, t );
	
	return ABCD;
}

float ComputeQuinticBezier1D( float A, float B, float C, float D, float E, float F, float t )
{
	float AB = Interpolate( A, B, t );
	float BC = Interpolate( B, C, t );
	float CD = Interpolate( C, D, t );
	float DE = Interpolate( D, E, t );
	float EF = Interpolate( E, F, t );

	float ABC = Interpolate( AB, BC, t );
	float BCD = Interpolate( BC, CD, t );
	float CDE = Interpolate( CD, DE, t );
	float DEF = Interpolate( DE, EF, t );

	float ABCD = Interpolate( ABC, BCD, t );
	float BCDE = Interpolate( BCD, CDE, t );
	float CDEF = Interpolate( CDE, DEF, t );

	float ABCDE = Interpolate( ABCD, BCDE, t );
	float BCDEF = Interpolate( BCDE, CDEF, t );

	float ABCDEF = Interpolate( ABCDE, BCDEF, t );

	return ABCDEF;
}

float SmoothStart2( float t )
{
	return t * t;
}

float SmoothStart3( float t )
{
	return t * t * t;
}

float SmoothStart4( float t )
{
	return t * t * t * t;
}

float SmoothStart5( float t )
{
	return t * t * t * t * t;
}

float SmoothStart6( float t )
{
	return t * t * t * t * t * t;
}

float SmoothStop2( float t )
{
	return 1.f - (1.f - t) * (1.f - t);
}

float SmoothStop3( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop4( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop5( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStop6( float t )
{
	return 1.f - (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t) * (1.f - t);
}

float SmoothStep3( float t )
{
	return Interpolate( SmoothStart3( t ), SmoothStop3( t ), t );
}

float SmoothStep5( float t )
{
	return Interpolate( SmoothStart5( t ), SmoothStop5( t ), t );
}

float Hesitate3( float t )
{
	return ComputeCubicBezier1D( 0.f, 0.f, 1.f, 1.f, t );
}

float Hesitate5( float t )
{
	return (1.f - t) * SmoothStop4( t ) + SmoothStart4( t );
	//return ComputeQuinticBezier1D( 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t );
}

float CustomFunkyEasingFunction( float t )
{
	return Hesitate5( t ) * Hesitate5( t );
}
