/*
* Copyright (c) 1998, Bo Zhou <Bo.Schwarzstein@gmail.com>
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Bo Zhou, nor the names of its contributors
*       may be used to endorse or promote products derived from this
*       software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PR_H
#define PR_H

#include <memory>
#include <string>
#include <RifPlugin.h>

#define NAMESPACE_PR_BEGIN namespace PR {
#define NAMESPACE_PR_END }

/**
 * \mainpage PR
 *
 * \author Bo Zhou
 * \date 2009
 *
 * \section Introduction
 * The "ParticleSolver"(PR) is a RenderMan procedural primitive framework DSO for particle-based
 * effect production. It extents the existing Maya particle function to do more. 
 *
 * \section Changelog
 * \date 2009/10/31
 * - Renamed "PolygonParticleSolver" to "SurfaceParticleSolver".
 * - "Diffusion" supports user defined random seed now.
 */

NAMESPACE_PR_BEGIN

/**
 * \brief Simple vector class
 * \warning there is already a class float3 in CUDA
 */
struct Float3
{
	Float3() : x(0.0f), y(0.0f), z(0.0f)
	{
	}
	~Float3()
	{
	}
	float x,y,z;
};

/**
 * \brief Interface for variable implemenation
 */
class ParticleResolver
{
public:
	ParticleResolver()
	{
	}
	virtual ~ParticleResolver()
	{
	}
	/**
	 * Pure virtual function for further implementation. It's the same as \b RiPointsV.
	 * You can get more information from <a href="https://renderman.pixar.com/products/rispec/rispec_pdf/RISpec3_2.pdf">RenderMan Interface Specification</a>.
	 */
	virtual RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []) = 0;
};

/**
 * \brief Instanced by RenderMan
 */
class ParticleResolverPlugin : public RifPlugin
{
public:
	static std::auto_ptr<ParticleResolver> Instance;

	static RtVoid AttributeV(RtToken, RtInt, RtToken [], RtPointer []);
	static RtVoid PointsV(RtInt, RtInt, RtToken [], RtPointer []);
	
	ParticleResolverPlugin(int, char**);
	virtual ~ParticleResolverPlugin();

	RifFilter& GetFilter();
private:
	RifFilter mRF;
};

NAMESPACE_PR_END

#endif
