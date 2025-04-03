#pragma once

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/CompositeNode.h"
#include "Engine/BehaviorTree/BehaviorTree.h"

class ExecutionSelector : public CompositeNode
{
public:
	ExecutionSelector( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );

	NodeStatus InternalTick( const BehaviorTree::Context* btContext ) override;

	std::string GetTypeName() const override;
};