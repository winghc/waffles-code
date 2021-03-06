/*
  The contents of this file are dedicated by all of its authors, including

    Michael S. Gashler,
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

#ifndef __GKALMAN_H__
#define __GKALMAN_H__

#include <cstring>

namespace GClasses {

class GMatrix;

/// This is an implementation of the Extended Kalman Filter. This class
/// is used by alternately calling advance and correct.
class GExtendedKalmanFilter
{
protected:
	size_t m_stateDims, m_obsDims;
	double* m_x; // estimated state
	double* m_z; // observation buffer
	double* m_zz; // state correction buffer
	GMatrix* m_pP; // estimated covariance of state

public:
	GExtendedKalmanFilter(int stateDims, int obsDims, int controlDims);
	~GExtendedKalmanFilter();

	/// Use this to get and/or set the estimated state
	double* state() { return m_x; }

	/// Use this to get and/or set the estimated covariance of the state
	GMatrix* stateCovariance() { return m_pP; }

	/// Advance to the next time step. pControl is the control vector
	/// to apply to the system at the current time step. pA is the
	/// Jacobian of the transition function at the current state, and
	/// with this control vector. Each column in pA represents the
	/// change in f(x,u) over the change in one of the dimensions of
	/// x, where f is the transition function, x is the state, and u
	/// is the control vector.
	void advance(const double* pControl, GMatrix* pA);

	/// Correct the estimates of state and covariance based on the
	/// observations made in the new state (and assuming the observation function
	/// is locally linear around the current state). pH is the Jacobian of
	/// the observation function at the current
	/// state. Each column in pH represents the change in h(x) over the change
	/// in one of the dimensions of x, where h is the observation function,
	/// and x is the state.
	void correct(const double* pObservation, GMatrix* pH);

	/// Computes the transition function. (Adjust the values in pInOutState
	/// as directed by pControl.)
	virtual void transition(double* pInOutState, const double* pControl) = 0;

	/// Computes the observation function. Put results in pOutObs.
	virtual void observation(double* pOutObs, const double* pState) = 0;

	/// pInOutCov is a nxn matrix, where n is the number of state dims.
	/// This method should add the covariance of the transition noise to pInOutCov.
	virtual void addTransitionNoise(GMatrix* pInOutCov) = 0;

	/// pInOutCov is a mxm matrix, where m is the number of observation dims.
	/// This method should add the covariance of the observation noise to pInOutCov.
	virtual void addObservationNoise(GMatrix* pInOutCov) = 0;
};

} // namespace GClasses

#endif // __GKALMAN_H__
