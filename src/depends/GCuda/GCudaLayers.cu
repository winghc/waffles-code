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

#include "GCudaLayers.h"
#include "../../GClasses/GNeuralNet.h"
#include "../../GClasses/GVec.h"

namespace GClasses {

GCudaLayer::GCudaLayer(GDomNode* pNode, GCudaEngine& engine)
: GNeuralNetLayer(), m_engine(engine)
{
	throw Ex("Sorry, GCudaLayer does not support serialization");
}

GDomNode* GCudaLayer::serialize(GDom* pDoc)
{
	throw Ex("Sorry, GNeuralNetLayerCuda does not support serialization");
	//return NULL;
}











GLayerClassicCuda::GLayerClassicCuda(GCudaEngine& engine, size_t inputs, size_t outputs)
: GCudaLayer(engine), m_pOutgoing(NULL)
{
	resize(inputs, outputs, NULL);
}

GLayerClassicCuda::~GLayerClassicCuda()
{
	delete[] m_pOutgoing;
}

void GLayerClassicCuda::resize(size_t inputCount, size_t outputCount, GRand* pRand, double deviation)
{
	if(inputCount == inputs() && outputCount == outputs())
		return;
	if(pRand)
		throw Ex("Sorry, GLayerClassicCuda does not support preserving resizes");

	m_weights.resize(inputCount, outputCount);
	m_bias.resize(outputCount);
	m_activation.resize(outputCount);
	m_error.resize(outputCount);
	delete[] m_pOutgoing;
	m_pOutgoing = NULL;
}

// virtual
void GLayerClassicCuda::resetWeights(GRand& rand)
{
	size_t inputCount = inputs();
	size_t outputCount = outputs();
	GMatrix mTmp(inputCount, outputCount);
	double mag = 1.0 / inputCount;
	for(size_t i = 0; i < inputCount; i++)
	{
		double* pW = mTmp[i];
		for(size_t j = 0; j < outputCount; j++)
			*(pW++) = rand.normal() * mag;
	}
	m_weights.upload(mTmp);
	GVec vTmp(outputCount);
	double* pB = vTmp.v;
	for(size_t i = 0; i < outputCount; i++)
		*(pB++) = rand.normal() * mag;
	m_bias.upload(vTmp.v, outputCount);
}

// virtual
void GLayerClassicCuda::perturbWeights(GRand& rand, double deviation, size_t start, size_t count)
{
	// Perturb weights
	GMatrix m;
	m_weights.download(m);
	size_t n = std::min(outputs() - start, count);
	for(size_t j = 0; j < m_weights.rows(); j++)
		GVec::perturb(m[j] + start, deviation, n, rand);
	m_weights.upload(m);

	// Perturb biases
	GVec v(outputs());
	m_bias.download(v.v);
	GVec::perturb(v.v + start, deviation, n, rand);
	m_bias.upload(v.v, outputs());
}

// virtual
double* GLayerClassicCuda::activation()
{
	if(!m_pOutgoing)
		m_pOutgoing = new double[outputs()];
	m_activation.download(m_pOutgoing);
	return m_pOutgoing;
}

// virtual
double* GLayerClassicCuda::error()
{
	if(!m_pOutgoing)
		m_pOutgoing = new double[outputs()];
	m_error.download(m_pOutgoing);
	return m_pOutgoing;
}

// virtual
void GLayerClassicCuda::copyBiasToNet()
{
	m_activation.copy(m_engine, m_bias);
	m_engine.sync();
}

// virtual
void GLayerClassicCuda::feedIn(const double* pIn, size_t inputStart, size_t inputCount)
{
	m_incoming.upload(pIn, inputs());
	m_weights.feedIn(m_engine, m_incoming, m_activation, inputStart);
	m_engine.sync();
}

// virtual
void GLayerClassicCuda::feedIn(GNeuralNetLayer* pUpStreamLayer, size_t inputStart)
{
	if(pUpStreamLayer->usesGPU())
	{
		m_weights.feedIn(m_engine, ((GCudaLayer*)pUpStreamLayer)->deviceActivation(), m_activation, inputStart);
		m_engine.sync();
	}
	else
		feedIn(pUpStreamLayer->activation(), inputStart, pUpStreamLayer->outputs());
}

// virtual
void GLayerClassicCuda::activate()
{
	m_activation.activateTanh(m_engine);
	m_engine.sync();
}

// virtual
void GLayerClassicCuda::dropOut(GRand& rand, double probOfDrop)
{
	throw Ex("sorry, not implemented yet");
}

// virtual
void GLayerClassicCuda::dropConnect(GRand& rand, double probOfDrop)
{
	throw Ex("sorry, not implemented yet");
}

void GLayerClassicCuda::computeError(const double* pTarget)
{
	m_error.upload(pTarget, outputs());
	m_error.add(m_engine, m_activation, -1.0);
	m_engine.sync();
}

void GLayerClassicCuda::deactivateError()
{
	m_error.deactivateTanh(m_engine, m_activation);
	m_engine.sync();
}

void GLayerClassicCuda::backPropError(GNeuralNetLayer* pUpStreamLayer, size_t inputStart)
{
	if(pUpStreamLayer->usesGPU())
	{
		m_weights.backPropError(m_engine, m_error, ((GCudaLayer*)pUpStreamLayer)->deviceError(), inputStart);
		m_engine.sync();
	}
	else
	{
		if(m_incoming.size() != inputs())
			m_incoming.resize(inputs());
		m_weights.backPropError(m_engine, m_error, m_incoming, inputStart);
		m_engine.sync();
		m_incoming.download(pUpStreamLayer->error());
	}
}

// virtual
void GLayerClassicCuda::updateBias(double learningRate, double momentum)
{
	m_bias.add(m_engine, m_error, learningRate);
	m_engine.sync();
}

// virtual
void GLayerClassicCuda::updateWeights(const double* pUpStreamActivation, size_t inputStart, size_t inputCount, double learningRate, double momentum)
{
	// Assume that the input was already uploaded into m_incoming when feedForward was called
	if(inputStart != 0 || inputCount != m_weights.rows())
		throw Ex("Sorry, partial weight updates are not yet supported in GNeuralNetLayerCuda");
	m_weights.updateWeights(m_engine, m_incoming, inputStart, m_error, learningRate);
}

// virtual
void GLayerClassicCuda::updateWeights(GNeuralNetLayer* pUpStreamLayer, size_t inputStart, double learningRate, double momentum)
{
	if(pUpStreamLayer->usesGPU())
	{
		m_weights.updateWeights(m_engine, ((GCudaLayer*)pUpStreamLayer)->deviceActivation(), inputStart, m_error, learningRate);
	}
	else
	{
		// Assume that the input was already uploaded into m_incoming when feedForward was called
		if(inputStart != 0)
			throw Ex("Sorry, partial weight updates are not yet supported in GNeuralNetLayerCuda");
		m_weights.updateWeights(m_engine, m_incoming, inputStart, m_error, learningRate);
	}
}

// virtual
void GLayerClassicCuda::updateWeightsAndRestoreDroppedOnes(const double* pUpStreamActivation, size_t inputStart, size_t inputCount, double learningRate, double momentum)
{
	throw Ex("Sorry, not implemented yet");
}

// virtual
void GLayerClassicCuda::updateWeightsAndRestoreDroppedOnes(GNeuralNetLayer* pUpStreamLayer, size_t inputStart, double learningRate, double momentum)
{
	throw Ex("Sorry, not implemented yet");
}

void GLayerClassicCuda::scaleWeights(double factor, bool scaleBiases)
{
	m_weights.scale(m_engine, factor);
	if(scaleBiases)
		m_bias.scale(m_engine, factor);
	m_engine.sync();
}

void GLayerClassicCuda::diminishWeights(double amount, bool diminishBiases)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::diminishWeights is not yet implemented");
}

// virtual
void GLayerClassicCuda::maxNorm(double max)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::maxNorm is not yet implemented");
}

// virtual
double GLayerClassicCuda::unitIncomingWeightsL1Norm(size_t unit)
{
	return m_weights.colSumAbs(m_engine, unit);
}

// virtual
double GLayerClassicCuda::unitIncomingWeightsL2Norm(size_t unit)
{
	return m_weights.colSumSquare(m_engine, unit);
}

// virtual
double GLayerClassicCuda::unitOutgoingWeightsL1Norm(size_t input)
{
	return m_weights.rowSumAbs(m_engine, input);
}

// virtual
double GLayerClassicCuda::unitOutgoingWeightsL2Norm(size_t input)
{
	return m_weights.rowSumSquare(m_engine, input);
}

// virtual
void GLayerClassicCuda::scaleUnitIncomingWeights(size_t unit, double scalar)
{
	m_weights.scaleCol(m_engine, unit, scalar);
}

// virtual
void GLayerClassicCuda::scaleUnitOutgoingWeights(size_t input, double scalar)
{
	m_weights.scaleRow(m_engine, input, scalar);
}

// virtual
size_t GLayerClassicCuda::countWeights()
{
	throw Ex("Sorry, GNeuralNetLayerCuda::countWeights is not yet implemented");
	//return 0;
}

// virtual
size_t GLayerClassicCuda::weightsToVector(double* pOutVector)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::weightsToVector is not yet implemented");
	//return 0;
}

// virtual
size_t GLayerClassicCuda::vectorToWeights(const double* pVector)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::vectorToWeights is not yet implemented");
	//return 0;
}

// virtual
void GLayerClassicCuda::copyWeights(GNeuralNetLayer* pSource)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::copyWeights is not yet implemented");
}

// virtual
void GLayerClassicCuda::renormalizeInput(size_t input, double oldMin, double oldMax, double newMin, double newMax)
{
	throw Ex("Sorry, GNeuralNetLayerCuda::renormalizeInput is not yet implemented");
}

void GLayerClassicCuda::upload(GLayerClassic& source)
{
	m_weights.upload(source.weights());
	m_bias.upload(source.bias(), source.outputs());
}

void GLayerClassicCuda::download(GLayerClassic& dest)
{
	m_weights.download(dest.weights());
	m_bias.download(dest.bias());
}


} // namespace GClasses

