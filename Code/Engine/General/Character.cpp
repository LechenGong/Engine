#include <cstdarg>

#include "Engine/Core/EngineCommon.hpp"

#include "Engine/General/Character.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Animation/AnimationController.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/General/Controller.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"

Character::Character()
	: Actor()
{
	g_eventSystem->SubscribeEventCallBackFunc( "toggleCollision", &ToggleCollision );
}

Character::Character( std::string name, SkeletalMesh* skeletalMesh )
	: Actor()
{
	if (!GetSkeletalMeshComponent() && !GetSkeletalMesh())
	{
		m_name = name;
		SetSkeletalMesh( skeletalMesh );
		GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	}

	m_boundingCollision = new CapsuleComponent();
	m_boundingCollision->SetCollisionChannel( CollisionChannel::PAWN );
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.f ) );
}

Character::Character( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures )
	: Actor()
{
	UNUSED( textures );
	m_name = name;
	SetSkeletalMesh( new SkeletalMesh( meshes, skeleton ) );
	GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );

	m_boundingCollision = new CapsuleComponent();
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.f ) );
}

Character::~Character()
{
	delete m_skeletalMeshComponent;
}

void Character::SetVelocity( Vec3 const& newVelocity )
{
	m_velocity = newVelocity;
}

void Character::AddVelocity( Vec3 const& additionVelocity )
{
	m_velocity += additionVelocity;
}

Vec3 Character::GetVelocity() const
{
	return m_velocity;
}

void Character::SetIsGrounded( bool flag )
{
	m_isGrounded = flag;
}

bool Character::GetIsGrounded() const
{
	return m_isGrounded;
}

void Character::Update( float deltaSeconds )
{
	if (m_controller)
	{
		m_controller->Update( deltaSeconds );
	}

	if (m_animController)
	{
		m_animController->Update( deltaSeconds );
	}

	if (!m_controller)
	{
		if (GetSkeletalMesh())
		{
			if (m_animController)
			{
				AnimationSequence* currentAnimation = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetAnimation();
				float currentAnimationClock = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();

				if (currentAnimation->m_rootTranslation.size() > 0)
				{
					Vec3 rootTranslation = currentAnimation->GetRootTranslationAtTime( currentAnimationClock, deltaSeconds );
					
					Vec3 position = GetActorLocalPosition() + GetActorLocalTransform().TransformVectorQuantity3D( rootTranslation * GetSkeletalMeshComponent()->GetLocalScale() );
					SetActorLocalPosition( position );
				}
				if (currentAnimation->m_rootRotation.size() > 0)
				{
					Quat rootRotation = currentAnimation->GetRootRotationAtTime( currentAnimationClock, deltaSeconds );

					SetActorLocalOrientation( GetActorLocalOrientation() * rootRotation );
				}
				GetSkeletalMesh()->Update();
				AnimationStateMachine* animStateMachine = m_animController->GetStateMachine();
				float currentTimeSeconds = animStateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();
				
				float previousTimeSecond = animStateMachine->GetOngoingAnimation( 0 ).GetPreviousAnimationPlaybackTime();
				AnimationSequence* previousAnimaiton = (animStateMachine->GetOngoingAnimation( 0 ).GetPreviousState()) ? animStateMachine->GetOngoingAnimation( 0 ).GetPreviousState()->GetAnimation() : nullptr;
				float crossfadeAlpha = animStateMachine->GetOngoingAnimation( 0 ).GetCrossfadeAlpha();
				GetSkeletalMesh()->UpdateJoints( GetSkeletalMesh()->m_skeleton.m_joints, GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), currentTimeSeconds, currentAnimation, previousTimeSecond, previousAnimaiton, crossfadeAlpha );
				ComponentCollisionCheck();
			}
		}
		return;
	}

	m_physicsTimer += deltaSeconds;

	Vec3 currentInputDirection = m_controller->GetMoveDirection();

	if (currentInputDirection != Vec3::ZERO && !m_controller->IsMovementHindered())
	{
		SetActorLocalOrientation( Quat( Vec3::UP, Atan2Degrees( currentInputDirection.y, currentInputDirection.x ) ) );
	}

	if (!m_animController)
		return;

	AnimationSequence* currentAnimation = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetAnimation();
	float currentAnimationClock = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();
//  	DebugAddMessage( m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetStateName(), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN );
//  	if (m_animController->GetStateMachine()->GetOngoingAnimation( 1 ).GetCurrentState())
//  		DebugAddMessage( m_animController->GetStateMachine()->GetOngoingAnimation( 1 ).GetCurrentState()->GetStateName() + "   " + std::to_string(m_animController->GetStateMachine()->GetOngoingAnimation(1).blendAlpha), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN);
	if (currentAnimation->m_rootTranslation.size() > 0)
	{
		Vec3 rootTranslation = currentAnimation->GetRootTranslationAtTime( currentAnimationClock, deltaSeconds );
		//DebugAddMessage( rootTranslation.ToString(), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN );
		Vec3 position = GetActorLocalPosition() + GetActorLocalTransform().TransformVectorQuantity3D( rootTranslation * GetSkeletalMeshComponent()->GetLocalScale() );
		//Vec3 position = GetActorLocalPosition() + rootTranslation * GetSkeletalMeshComponent()->GetLocalScale();
		SetActorLocalPosition( position );
	}
	if (currentAnimation->m_rootRotation.size() > 0)
	{
		Quat rootRotation = currentAnimation->GetRootRotationAtTime( currentAnimationClock, deltaSeconds );

		SetActorLocalOrientation( GetActorLocalOrientation() * rootRotation );
	}

	if (GetSkeletalMesh())
	{
		GetSkeletalMesh()->Update();
		GetSkeletalMesh()->UpdateJoints( GetSkeletalMesh()->m_skeleton.m_joints, GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), m_animController->GetStateMachine() );
	}

	SetIsGrounded( false );

	ComponentCollisionCheck();

	if (!GetIsGrounded())
	{
		SetIsGrounded( GetAboveGroundHeight() <= 0.1f );
	}

	if (GetIsGrounded())
	{
		m_velocity.z = 0.f;
		m_offGroundTimer = 0.f;
	}
	else
	{
		m_offGroundTimer += deltaSeconds;
	}

	while (m_physicsTimer >= fixedPhysicsDeltaTime)
	{
		if (m_enableGravity)
		{
			m_velocity += Vec3( 0.f, 0.f, -10.f ) * fixedPhysicsDeltaTime;
			//m_velocity.z = Clamp( m_velocity.z, -50.f, 99999.f );
			SetActorWorldPosition( GetActorWorldPosition() + m_velocity * fixedPhysicsDeltaTime );
			ComponentCollisionCheck();
		}

		m_physicsTimer -= fixedPhysicsDeltaTime;
	}

	CameraArmCollisionCheck();	
}

void Character::Render() const
{
	g_theRenderer->SetModelConstants( GetSkeletalMeshComponent()->GetWorldTransform() );
	g_theRenderer->SetJointConstants( GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), GetSkeletalMesh()->GetSkeleton().m_joints );
	GetSkeletalMeshComponent()->Render();
}

void Character::InitializeAllCollisions()
{
	InitializeCollisionComponents();
	auto collisionList = GetAllCollisionInfo();
	m_collisionsEnabled.resize( collisionList.size() );
	for (int i = 0; i < collisionList.size(); i++)
	{
		if (collisionList[i].use == CollisionUsage::BODY ||
			collisionList[i].use == CollisionUsage::PHYSICS ||
			collisionList[i].use == CollisionUsage::INTERACTION)
		{
			m_collisionsEnabled[i] = true;
		}
		else
		{
			m_collisionsEnabled[i] = false;
		}
	}
}

std::vector<CollisionInfo>const& Character::GetAllCollisionInfo()
{
	throw std::logic_error("GetCollisionsInfo() called on base class.");
}

bool Character::DoCharactersOverlap( Character* charaA, Character* charaB, Vec3& out_mtv_XY_A, Vec3& out_mtv_XY_B )
{
	if (!ShapeComponent::DoShapesOverlap( charaA->m_boundingCollision, charaB->m_boundingCollision, out_mtv_XY_A ))
		return false;

	auto collisionInfoA = charaA->GetAllCollisionInfo();
	auto collisionInfoB = charaB->GetAllCollisionInfo();

	Vec3 resultMTV = Vec3::ZERO;
	float maxPenetration = 0.f;
	for (int i = 0; i < collisionInfoA.size(); i++)
	{
		if (!charaA->m_collisionsEnabled[i])
			continue;
		if (collisionInfoA[i].use != CollisionUsage::BODY)
			continue;

		for (int j = 0; j < collisionInfoB.size(); j++)
		{
			if (!charaB->m_collisionsEnabled[j])
				continue;
			if (collisionInfoB[j].use != CollisionUsage::BODY)
				continue;

			Vec3 mtv;
			if (!ShapeComponent::DoShapesOverlap( collisionInfoA[i], charaA, collisionInfoB[j], charaB, mtv ))
				continue;

			resultMTV += mtv;
			maxPenetration = MAX( maxPenetration, mtv.GetLength() );
		}
	}
	if (resultMTV == Vec3::ZERO)
		return false;

	float magitudeA = charaA->m_mass / (charaA->m_mass + charaB->m_mass);
	float magitudeB = charaB->m_mass / (charaA->m_mass + charaB->m_mass);
	resultMTV = resultMTV.GetNormalized();
	Vec3 directionXY = Vec3( resultMTV.x, resultMTV.y, 0.f );
	out_mtv_XY_A = directionXY * magitudeA * maxPenetration;
	out_mtv_XY_B = directionXY * -magitudeB * maxPenetration;
	return true;
}

bool Character::DoesAttackHits( Character* attacker, Character* receiver )
{
	Vec3 placeholder;

	auto attackerCollisions = attacker->GetAllCollisionInfo();
	auto receiverCollisions = receiver->GetAllCollisionInfo();

	for (int i = 0; i < attackerCollisions.size(); i++)
	{
		if (attackerCollisions[i].use != CollisionUsage::ATTACK)
			continue;
		if (!attacker->m_collisionsEnabled[i])
			continue;

		ShapeComponent* attackerCollision = nullptr;
		if (attackerCollisions[i].shape == CollisionShape::CAPSULE)
		{
			attackerCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}
		else if (attackerCollisions[i].shape == CollisionShape::SPHERE)
		{
			attackerCollision = new SphereComponent( SphereComponent::CreateSphereComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}
		attackerCollision->SetCollisionChannel( PAWN );

		if (!ShapeComponent::DoShapesOverlap( attackerCollision, receiver->m_boundingCollision, placeholder ))
			continue;

		for (int j = 0; j < receiverCollisions.size(); j++)
		{
			if (receiverCollisions[j].use != CollisionUsage::BODY)
				continue;
			if (!receiver->m_collisionsEnabled[j])
				continue;

			ShapeComponent* receiverCollision = nullptr;
			if (receiverCollisions[j].shape == CollisionShape::CAPSULE)
			{
				receiverCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}
			else if (receiverCollisions[j].shape == CollisionShape::SPHERE)
			{
				receiverCollision = new SphereComponent( SphereComponent::CreateSphereComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}

			if (ShapeComponent::DoShapesOverlap( attackerCollision, receiverCollision, placeholder ))
			{
				delete attackerCollision;
				delete receiverCollision;
				return true;
			}
			delete receiverCollision;
		}
		delete attackerCollision;
	}
	return false;
}

void Character::OnPossessed( Controller* controller )
{
	if (m_controller)
	{
		m_controller->Unpossess();
	}
	m_controller = controller;
	m_controller->m_possessedChara = this;
}

void Character::OnUnpossessed()
{
	if (m_controller)
	{
		m_controller->m_possessedChara = nullptr;
		m_controller = nullptr;
	}
}

void Character::SetMovementSpeed( float newSpeed )
{
	m_movementSpeed = newSpeed;
}

float Character::GetMovementSpeed() const
{
	return m_movementSpeed;
}

void Character::SetSprintParam( float newSprintParam )
{
	m_sprintParam = newSprintParam;
}
float Character::GetSprintParam() const
{
	return m_sprintParam;
}

void Character::SetSkeletalMesh( SkeletalMesh* skeletalMesh )
{
	if (!m_skeletalMeshComponent)
		m_skeletalMeshComponent = new SkeletalMeshComponent;
	m_skeletalMeshComponent->SetSkeletalMesh( skeletalMesh );
}

SkeletalMesh* Character::GetSkeletalMesh() const
{
	if (!m_skeletalMeshComponent)
		return nullptr;
	return m_skeletalMeshComponent->GetSkeletalMesh();
}

SkeletalMeshComponent* Character::GetSkeletalMeshComponent() const
{
	return m_skeletalMeshComponent;
}

Mat44 const& Character::GetJointGlobalTransformByIndex( int jointIndex ) const
{
	return GetSkeletalMeshComponent()->GetSkeletonGlobalTransform()[jointIndex];
}

Mat44 const& Character::GetJointGlobalTransformByName( std::string jointName ) const
{
	return GetSkeletalMeshComponent()->GetSkeletonGlobalTransform()[GetSkeletalMesh()->GetJointIndexByName( jointName )];
}

bool Character::ToggleCollision( Character* character, int collisionIndex, bool flag )
{
	character->SetCollisionEnabled( collisionIndex, flag );
	return false;
}

bool Character::SetCollisionEnabled( int collisionIndex, bool flag )
{
	if (collisionIndex >= m_collisionsEnabled.size() ||
		collisionIndex < 0)
		return false;

	m_collisionsEnabled[collisionIndex] = flag;
	return true;
}

void Character::CameraArmCollisionCheck()
{
	m_camera.m_position = GetActorWorldPosition() + m_cameraArmPivotPos;
	Vec3 cameraArmVectorNormal = m_camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
	Vec3 cameraArmVectorNegateNormal = cameraArmVectorNormal * -1.f;

	m_camera.m_position += cameraArmVectorNegateNormal * m_cameraArmLength;
}

void Character::ComponentCollisionCheck()
{
}
