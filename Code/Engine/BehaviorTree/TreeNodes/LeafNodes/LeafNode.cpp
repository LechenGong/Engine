#include "Engine/BehaviorTree/TreeNodes/LeafNodes/LeafNode.h"

LeafNode::LeafNode( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: TreeNode( bt, selfIndex, parentIndex, location )
{

}

