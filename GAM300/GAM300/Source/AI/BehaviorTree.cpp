#include "Precompiled.h"
#include "BehaviorTree.h"

void BehaviorNode::setResult(BehaviorResult result)
{
	mResult = result;
}

void BehaviorNode::setStatus(BehaviorStatus status)
{
	mStatus = status;
}

const BehaviorStatus BehaviorNode::getStatus()
{
	return mStatus;
}

const BehaviorResult BehaviorNode::getResult()
{
	return mResult;
}

void BehaviorNode::addChild(BehaviorNode* mChild)
{
	mChildren.push_back(mChild);
}

std::vector<BehaviorNode*> BehaviorNode::getChildren()
{
	return mChildren;
}

void BehaviorNode::onLeafEnter()
{
	setStatus(BehaviorStatus::RUNNING);
	setResult(BehaviorResult::IN_PROGRESS);
}

void BehaviorNode::onSuccess()
{
	setStatus(BehaviorStatus::EXITING);
	setResult(BehaviorResult::SUCCESS);
}

void BehaviorNode::onFailure()
{
	setStatus(BehaviorStatus::EXITING);
	setResult(BehaviorResult::FAILURE);
}

void BehaviorTree::RunTree(float dt)
{
	mRootNode->Tick(dt);
}

std::string BehaviorTree::GetTreeName() const
{
	return mTreeName;
}
