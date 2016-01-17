/*
 * Copyright (c) The Shogun Machine Learning Toolbox
 * Written (w) 2015 Wu Lin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Shogun Development Team.
 *
 */

#include <shogun/optimization/RmsPropUpdater.h>
#include <shogun/lib/config.h>
using namespace shogun;

RmsPropUpdater::RmsPropUpdater()
	:DescendUpdaterWithCorrection()
{
	init();
}

RmsPropUpdater::RmsPropUpdater(float64_t learning_rate, float64_t epsilon, float64_t decay_factor)
	:DescendUpdaterWithCorrection()
{
  	init();
	set_learning_rate(learning_rate);
	set_epsilon(epsilon);
	set_decay_factor(decay_factor);
}

void RmsPropUpdater::set_learning_rate(float64_t learning_rate)
{
	REQUIRE(learning_rate>0,"Learning_rate (%f) must be positive\n",
		learning_rate);
	m_build_in_learning_rate=learning_rate;
}

void RmsPropUpdater::set_epsilon(float64_t epsilon)
{
	REQUIRE(epsilon>=0,"Epsilon (%f) must be non-negative\n",
		epsilon);
	m_epsilon=epsilon;
}

void RmsPropUpdater::set_decay_factor(float64_t decay_factor)
{
	REQUIRE(decay_factor>=0.0 && decay_factor<1.0,
		"decay factor (%f) must in [0,1)\n",
		decay_factor);
	m_decay_factor=decay_factor;
}

RmsPropUpdater::~RmsPropUpdater()
{}

void RmsPropUpdater::init()
{
	m_decay_factor=0.9;
	m_epsilon=1e-6;
	m_build_in_learning_rate=1.0;
	m_gradient_accuracy=SGVector<float64_t>();
}

void RmsPropUpdater::update_context(CMinimizerContext* context)
{
	DescendUpdaterWithCorrection::update_context(context);
	REQUIRE(context, "Context must set\n");
	SGVector<float64_t> value(m_gradient_accuracy.vlen);
	std::copy(m_gradient_accuracy.vector,
		m_gradient_accuracy.vector+m_gradient_accuracy.vlen,
		value.vector);
	std::string key="RmsPropUpdater::m_gradient_accuracy";
	context->save_data(key, value);
}

void RmsPropUpdater::load_from_context(CMinimizerContext* context)
{
	DescendUpdaterWithCorrection::load_from_context(context);
	REQUIRE(context, "Context must set\n");
	std::string key="RmsPropUpdater::m_gradient_accuracy";
	SGVector<float64_t> value=context->get_data_sgvector_float64(key);
	m_gradient_accuracy=SGVector<float64_t>(value.vlen);
	std::copy(value.vector, value.vector+value.vlen,
		m_gradient_accuracy.vector);
}

float64_t RmsPropUpdater::get_negative_descend_direction(float64_t variable,
	float64_t gradient, index_t idx, float64_t learning_rate)
{
	REQUIRE(idx>=0 && idx<m_gradient_accuracy.vlen,
		"Index (%d) is invalid\n", idx);
	float64_t scale=m_decay_factor*m_gradient_accuracy[idx]+
		(1.0-m_decay_factor)*gradient*gradient;
	m_gradient_accuracy[idx]=scale;
	float64_t res=m_build_in_learning_rate*gradient/CMath::sqrt(scale+m_epsilon);
	return res;
}

void RmsPropUpdater::update_variable(SGVector<float64_t> variable_reference,
	SGVector<float64_t> raw_negative_descend_direction, float64_t learning_rate)
{
	REQUIRE(variable_reference.vlen>0,"variable_reference must set\n");
	REQUIRE(variable_reference.vlen==raw_negative_descend_direction.vlen,
		"The length of variable_reference (%d) and the length of gradient (%d) do not match\n",
		variable_reference.vlen,raw_negative_descend_direction.vlen);
	if(m_gradient_accuracy.vlen==0)
	{
		m_gradient_accuracy=SGVector<float64_t>(variable_reference.vlen);
		m_gradient_accuracy.set_const(0.0);
	}
	DescendUpdaterWithCorrection::update_variable(variable_reference, raw_negative_descend_direction, learning_rate);
}
