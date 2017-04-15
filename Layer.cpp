﻿#include "Layer.h"
#include <math.h>
#include <assert.h>
#include <iostream>

using namespace std;

Layer::Layer(unsigned inputSize, unsigned outputSize)
{
	m_inputSize = inputSize;
	m_outputSize = outputSize;
	m_outputs.resize(outputSize);
	m_rawOutputs.resize(outputSize);
	m_delta.resize(outputSize);
	m_gradients.resize(outputSize);

	// Initialise outputs of each neuron to 0s
	for (unsigned n = 0; n < outputSize; n++) {
		m_outputs[n] = 0.0;
		m_rawOutputs[n] = 0.0;
	}
}

Layer::Layer()
{
}


Layer::~Layer()
{

}

void Layer::initialiseWeights()
{
	m_weights.randn(m_inputSize, m_outputSize);
}

void Layer::updateWeights(Layer &prevLayer)
{
	for (unsigned i = 0; i < prevLayer.getOutputSize(); i++) {
		for (unsigned j = 0; j < m_outputSize; j++) {
			// (w_ij)^l ← (w_ij)^l - η ((x_i)^(l-1)) (δ_j)^l
			// New weight = old weight - (eta * x from previous layer * delta for current layer)
			double oldWeight = prevLayer.getWeight(i, j);

			double newWeight = oldWeight - ETA * prevLayer.getOutput(i) * m_gradients[j];

			prevLayer.setWeight(i,j, newWeight);
		}
	}

	/*
	for (unsigned row = 0; row < m_weights.n_rows; row++) {
		for (unsigned col = 0; col < m_weights.n_cols; col++) {
			
			m_weights(row,col) -= (ETA * prevLayer.getOutput(row) * m_delta[col]);
			//cout << m_weights(row, col);
		}
	}
	*/
}

vector<double> Layer::forwardPropagate(Layer &prevLayer) {
	// (x_j)^l = θ(sum of{(w_ij)^l * ((x_i)^(l-1))})
	//Next x = ThresholdOf(Sum of(Current weight * x from the previous layer))

	// For each neuron in this layer
	for (unsigned j = 0; j < m_outputSize; j++)
	{
		double sum = 0.0;

		// For each neuron in the previous layer
		for (unsigned i = 0; i < prevLayer.getOutputSize(); i++)
		{
			// Add the previous layer's output * the weight between (x_i)^(l-1) and (x_j)^l
			sum += prevLayer.getOutput(i) * prevLayer.getWeight(i, j);
		}

		sum += 1.0; // +1 for bias neuron

		m_rawOutputs[j] = sum;
		m_outputs[j] = sigmoid(sum);
	}

	return m_outputs;
}

double Layer::calculateError(double target)
{
	// For final layer: (δ_1)^L = ∂e(w) / ∂(s_1)^L
	// delta = 2(1-tanh^2((s_1)^2))(x_1 - y_n)

	double error = 0.0;

	for (unsigned n = 0; n < m_outputSize; n++) {
		double delta = target - m_outputs[n];
		m_delta[n] = delta;
		error += delta * delta;
	}
	
	error /= m_outputSize; // Average error squared
	error = sqrt(error); // RMS

	return error;

	/*
	double tanhS = sigmoid(m_rawOutputs[0] * m_rawOutputs[0]);

	m_delta[0] = 2 * (1 - tanhS*tanhS) * (m_outputs[0] - target);
	*/
}

double Layer::sumDerivativeOfWeights(Layer &nextLayer) {
	double sum = 0.0;

	// For each neuron in the current layer
	for (unsigned i = 0; i < m_outputSize; i++) {
		// For each neuron in the next layer
		for (unsigned j = 0; j < nextLayer.getOutputSize(); j++) {
			// Add the weight w_ij in the current layer * the gradient of 
			// x_j in the next layer to the sum
			sum += m_weights.at(i, j) * nextLayer.getGradient(j);
		}
	}

	return sum;
}

void Layer::calcHiddenGradients(Layer &nextLayer) {
	double dow = sumDerivativeOfWeights(nextLayer);
	for (unsigned n = 0; n < m_outputSize; n++) {
		m_gradients[n] = dow * sigmoidDerivative(m_outputs[n]);
	}	
}

void Layer::calcOutputGradients(double target) {
	for (unsigned n = 0; n < m_outputSize; n++) {
		double delta = target - m_outputs[n];
		m_gradients[n] = delta * sigmoidDerivative(m_outputs[n]);
	}	
}

vector<double> Layer::backPropagate(vector<double> input, Layer &prevLayer)
{
	// (δ_i)^(l-1) = (1 - (((x_i)^(l-1))^2)(sum of{((w_ij)^l)((δ_j)^l)}
	// Delta for prev layer = (1 - (x from prev layer squared * (sum of (weights from current layer * current deltas))))
	
	double sum;
	vector<double> output;
	output.resize(m_inputSize);

	for (unsigned i = 0; i < m_weights.n_cols; i++) {
		sum = 0;

		for (unsigned j = 0; j < m_weights.n_rows; j++) {
			sum += (m_weights(j, i) * m_delta[j]);
		}

		double prevX = prevLayer.getOutput(i);

		prevLayer.setDelta(i, 1 - prevX * prevX * sum);
	}

	return output;
}