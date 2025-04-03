#pragma once

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/CompositeNode.h"
#include "Engine/BehaviorTree/BehaviorTree.h"

class ExecutionRandomSequence : public CompositeNode
{
public:
	ExecutionRandomSequence( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );

	void InternalSpawn( const BehaviorTree::Context* btContext ) override;
	NodeStatus InternalTick( const BehaviorTree::Context* btContext ) override;

	std::string GetTypeName() const override;
};
