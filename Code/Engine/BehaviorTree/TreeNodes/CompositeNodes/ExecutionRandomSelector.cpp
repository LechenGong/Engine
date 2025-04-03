#include <algorithm>
#include <random>

#include "Engine/BehaviorTree/TreeNodes/CompositeNodes/ExecutionRandomSelector.h"

ExecutionRandomSelector::ExecutionRandomSelector( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: CompositeNode( bt, selfIndex, parentIndex, location )
{

}

void ExecutionRandomSelector::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	static std::random_device rd;
	static std::mt19937 rng( rd() );
	std::shuffle( m_children.begin(), m_children.end(), rng );
}

NodeStatus ExecutionRandomSelector::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
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
		else if (status == NodeStatus::SUCCESS)
		{
			return m_status = NodeStatus::SUCCESS;
		}
		else if (status == NodeStatus::BREAKING)
		{
			return m_status = (m_status == NodeStatus::INVALID) ? NodeStatus::BREAKING : m_status;
		}
	}
	return m_status = NodeStatus::FAILURE;
}

std::string ExecutionRandomSelector::GetTypeName() const
{
	return "RandomSelector";
}

