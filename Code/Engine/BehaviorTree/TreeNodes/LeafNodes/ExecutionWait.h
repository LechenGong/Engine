#pragma once

#include "Engine/BehaviorTree/TreeNodes/LeafNodes/LeafNode.h"

class ExecutionWait : public LeafNode
{
public:
	ExecutionWait( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
	void ParseDataFromXml( XmlElement* xmlElement ) override;
	void ExportAttributeToXml( XmlElement* xmlElement ) override;

	void InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	NodeStatus InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	void InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext ) override {};

	std::string GetTypeName() const override;

protected:
	float m_duration = 0;
	float m_timeStarts = 0;
};