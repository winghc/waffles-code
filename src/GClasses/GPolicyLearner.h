/*
  The contents of this file are dedicated by all of its authors, including

    Michael S. Gashler,
    Eric Moyer,
    anonymous contributors,

  to the public domain (http://creativecommons.org/publicdomain/zero/1.0/).

  Note that some moral obligations still exist in the absence of legal ones.
  For example, it would still be dishonest to deliberately misrepresent the
  origin of a work. Although we impose no legal requirements to obtain a
  license, it is beseeming for those who build on the works of others to
  give back useful improvements, or find a way to pay it forward. If
  you would like to cite us, a published paper about Waffles can be found
  at http://jmlr.org/papers/volume12/gashler11a/gashler11a.pdf. If you find
  our code to be useful, the Waffles team would love to hear how you use it.
*/

#ifndef __GPOLICYLEARNER_H__
#define __GPOLICYLEARNER_H__

#include "GMatrix.h"
#include <vector>

namespace GClasses {

class GRand;
class GIncrementalLearner;


/// Iterates through all the actions that are valid in the current state.
/// If actions are continuous or very numerous, this should sample valid
/// actions in a random order. The caller may decide that it has sampled
/// enough at any time.
class GAgentActionIterator
{
public:
	GAgentActionIterator()
	{
	}

	virtual ~GAgentActionIterator()
	{
	}

	/// Returns the number of actions. If actions are continuous,
	/// this should return 0x7fffffff.
	/// (If actions are state-dependent, it should return the
	/// total number of possible unique actions unioned over
	/// all states.)
	virtual int actionCount() = 0;

	/// Returns the number of dimensions in the action vector
	virtual int actionDims() = 0;

	/// Produces a random action
	virtual void randomAction(double* pOutAction, GRand* pRand) = 0;

	/// Prepares to begin iterating over actions. (Note that this
	/// is not called by the constructor, so you should even call
	/// Reset the first time you iterate over the actions.) If the
	/// set of available actions is not state-dependent, this should
	/// simply ignore the pState parameter.
	virtual void reset(const double* pState) = 0;

	/// Returns false if there are no more available actions.
	virtual bool nextAction(double* pOutAction) = 0;
};



/// This is a simple and common action iterator that can be
/// used when there is a discrete set of possible actions
class GDiscreteActionIterator : public GAgentActionIterator
{
protected:
	int m_action;
	int m_count;

public:
	GDiscreteActionIterator(int actionCount) : GAgentActionIterator(), m_count(actionCount) {}
	virtual ~GDiscreteActionIterator() {}

	/// Returns the total number of action values
	virtual int actionCount() { return m_count; }

	/// Returns 1
	virtual int actionDims() { return 1; }

	/// Returns a random action
	virtual void randomAction(double* pOutAction, GRand* pRand);

	/// Resets the iterator
	virtual void reset(const double* pState);
	
	/// Iterates to the next action. Returns false if there are no more.
	virtual bool nextAction(double* pOutAction);
};




/// This is the base class for algorithms that learn a policy.
class GPolicyLearner
{
protected:
	GRelation* m_pRelation;
	int m_senseDims;
	int m_actionDims;
	bool m_teleported;
	bool m_explore;

public:
	/// actionDims specifies how many dimensions are in the action vector. (For example, if your
	/// agent has a discrete set of ten possible actions, then actionDims should be 1, because it
	/// only takes one value to represent one of ten discrete actions. If your agent has four legs
	/// that can move independently to continuous locations relative to the position of the
	/// agent in 3D space, then actionDims should be 12, because it takes three values to represent
	/// the force or offset vector for each leg.)
	/// The number of sense dimensions is the number of attributes in pRelation minus actionDims.
	/// pRelation specifies the type of each element in the sense and action vectors (whether they
	/// are continuous or discrete). The first attributes refer to the senses, and the last actionDims
	/// attributes refer to the actions.
	GPolicyLearner(const GRelation& relation, int actionDims);
	GPolicyLearner(GDomNode* pNode);

	virtual ~GPolicyLearner();

	/// This method tells the agent to learn from the current senses,
	/// and select a new action vector. (This method should also
	/// set m_teleported to false.)
	virtual void refinePolicyAndChooseNextAction(const double* pSenses, double* pOutActions) = 0;

	/// If an external force changes the state of pSenses, you should
	/// call this method to inform the agent that the change is not a
	/// consequence of its most recent action. The agent should refrain
	/// from "learning" when refinePolicyAndChooseNextAction is next called.
	void onTeleport()
	{
		m_teleported = true;
	}

	/// If b is false, then the agent will only exploit (and no learning
	/// will occur). If b is true, (which is the default) then the agent
	/// will seek some balance between exploration and exploitation.
	void setExplore(bool b)
	{
		m_explore = b;
	}

protected:
	/// If a child class has a serialize method, it
	/// should use this method to serialize the base-class stuff.
	GDomNode* baseDomNode(GDom* pDoc);
};


} // namespace GClasses

#endif // __GPOLICYLEARNER_H__
