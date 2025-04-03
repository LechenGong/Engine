#pragma once

#include "Engine/BehaviorTree/TreeNodes/TreeNode.h"

class LeafNode : public TreeNode
{
public:
	LeafNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
};
