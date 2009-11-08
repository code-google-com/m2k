#include "PR.h"

NAMESPACE_PR_BEGIN

/**
 * \brief "Surface"
 *
 * For polygon model we suggest <a href="http://usa.autodesk.com/adsk/servlet/pc/index?siteID=123112&id=6837478">FBX</a> now.
 * It will, 
 * - Apply(or not) subdivision on mesh.
 * - Sample primitives to generate a dense distribution to represent the geometry.
 */
class SurfaceParticleResolver : public ParticleResolver
{
public:
	SurfaceParticleResolver();
	~SurfaceParticleResolver();
	
	void SetPath(const char*);
	void SetSubdLevel(const int&);
	
	/**
	 * \todo
	 * Have not been implemented yet.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	int mSubdLevel;
	std::string mPath;
};

NAMESPACE_PR_END
