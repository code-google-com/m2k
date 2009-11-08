#include "PR.h"

NAMESPACE_PR_BEGIN

/**
 * \brief "External"
 *
 * It would load an external procedural primitive DSO to do more.
 */
class ExternalParticleResolver : public ParticleResolver
{
public:
	ExternalParticleResolver();
	~ExternalParticleResolver();

	void SetPath(const char*);
	void SetBound(const RtBound&);
	/**
	 * \todo
	 * Have not been implemented yet.
	 */
	RtVoid DoIt(RtInt, RtInt, RtToken [], RtPointer []);
private:
	std::string mPath;
	RtBound mB;
};

NAMESPACE_PR_END
