#include <algorithm>
#include <random>

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionRandomSequence.h"

ExecutionRandomSequence::ExecutionRandomSequence( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: CompositeNode( bt, selfIndex, parentIndex, location )
{

}

void ExecutionRandomSequence::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	static std::random_device rd;
	static std::mt19937 rng( rd() );
	std::shuffle( m_children.begin(), m_children.end(), rng );
}

NodeStatus ExecutionRandomSequence::InternalTick(const BehaviorTree::Context* btContext)
{
	int runningChildIndex = 0;
	for (int i = 0; i < m_children.size(); i++)
	{
		if (m_children[i]->GetStatus() == NodeStatus::RUNNING)
		{
			runningChildIndex = i;
			break;
		}
	}

	for (int i = runningChildIndex; i < m_children.size(); i++)
	{
		TreeNode* child = m_children[i];
		NodeStatus status = child->Execute( btContext );

		if (status == NodeStatus::RUNNING)
		{
			return m_status = NodeStatus::RUNNING;
		}
		else if (status == NodeStatus::FAILURE)
		{
			return m_status = NodeStatus::FAILURE;
		}
		else if (status == NodeStatus::BREAKING)
		{
			return m_status = (m_status == NodeStatus::INVALID) ? NodeStatus::BREAKING : m_status;
		}
	}
	return m_status = NodeStatus::SUCCESS;
}

std::string ExecutionRandomSequence::GetTypeName() const
{
	return "RandomSequence";
}

