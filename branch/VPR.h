#ifndef VPR_H
#define VPR_H

#include "PR.h"

NAMESPACE_PR_BEGIN
/**
 * \brief "Volumetric"
 * 
 * It will
 * - Read an external point cloud.
 * - Convert into level set signed distance function
 * - Refine coarse point cloud.
 * 
 * Go to <a href="http://software.primefocusworld.com/software/support/krakatoa/render_geometry_volumes.php">CONVERTING GEOMETRY VOLUMES TO PARTICLE CLOUDS</a> for more details.
 */
class VolumetricParticleSolver : public ParticleResolver
{
public:
	VolumetricParticleSolver();
	~VolumetricParticleSolver();

	void SetRes(const RtInt&, const RtInt&, const RtInt&);
	
	/**
	 * \todo
	 * Have not been implemented yet.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	RtInt mRes[3];
};

NAMESPACE_PR_END

#endif
