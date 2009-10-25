#pragma once

// DT = Data Type
// CT = Coordinates Type

/**
 * \brief Common interface class for scattering data interpolation
 */
template<class DataType, class CoordType>
class Approx
{
public:
	typedef typename DataType DT;
	typedef typename CoordType CT;

	Approx();
	virtual ~Approx();

	/**
	 * @1 Size of known data
	 * @2 Known sites
	 * @3 Known function values
	 * @4 Lookup position
	 * @5 Interpolated value
	 */
	virtual void GetInterpolatedValue(const size_t, const int, const CT*, const DT*, const CT&, DT&) = 0;
};

/**
 * \brief Metaball approximation kernel
 */
template<class DataType, class CoordType>
class WyvillApprox : public Approx<DataType,CoordType>
{
public:
	WyvillApprox(const float);
	~WyvillApprox();

	void GetInterpolatedValue(const size_t, const int, const CT*, const DT*, const CT&, DT&);
private:
	float mR;
};

/**
 * \brief RBF approximation kernel
*/
template<class DataType, class CoordType>
class RBFApprox : public Approx<DataType,CoordType>
{
public:
	RBFApprox(const float);
	~RBFApprox();
	
	void GetInterpolatedValue(const size_t, const int, const CT*, const DT*, const CT&, DT&);
private:
	float mConst;
};
