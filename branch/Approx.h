#pragma once

#include <iostream>
#include <math.h>

#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

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

	Approx()
	{
	}
	virtual ~Approx()
	{
	}

	/**
	 * @1 Size of known data
	 * @2 Known sites
	 * @3 Known function values
	 * @4 Lookup position
	 * @5 Interpolated value
	 */
	virtual DT GetInterpolatedValue(const size_t, const int, const CT*, const DT*, const CT&) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Linear
{
	float operator() (float r, float c) const
	{
		return r;
	}
};

struct TPSpline
{
	float operator() (float r, float c) const
	{
		return r*r*log(r);
	}
};

struct Gaussian
{
	float operator() (float r, float c) const
	{
		return exp(-c*r*r);
	}
};

struct Multiquad
{
	float operator() (float r, float c) const
	{
		return sqrt( r*r + c*c );
	}
};

typedef  boost::function<float(float r,float c)> RBF;

/**
 * \brief RBF approximation kernel
*/
template<class DataType, class CoordType>
class RBFApprox : public Approx<DataType,CoordType>
{
public:
	RBFApprox(const float Const) : mConst(Const)
	{
	}
	~RBFApprox()
	{
	}
	
	DT GetInterpolatedValue(const size_t Len, const int D, const CT* Coords, const DT* Values, const CT& LookupPos)
	{
		const float Const = mConst;

		gsl_vector* f = gsl_vector_alloc(Len);
		gsl_vector* c = gsl_vector_alloc(Len);
		gsl_matrix* A = gsl_matrix_alloc(Len,Len);
		gsl_matrix* A_i = gsl_matrix_alloc(Len,Len);

		// Fill the value f
		for(size_t i=0; i<Len; ++i)
			gsl_vector_set(f,i,Values[i]);
		
		//Fill the matrix A_{ij}
		RBF TheBasisFunction = TPSpline();
		for( size_t i=0; i<Len; ++i )
		{
			for( size_t j=0; j<Len; ++j )
			{
				float3 V;
				V.x = Coords[i].x - LookupPos.x;
				V.y = Coords[i].y - LookupPos.y;
				V.z = Coords[i].z - LookupPos.z;
				float Len = sqrtf( V.x*V.x + V.y*V.y + V.z*V.z );
				
				double Phi = TheBasisFunction(Len,Const);
				gsl_matrix_set(A,i,j,Phi);
			}
		}

		gsl_permutation* A_perm = gsl_permutation_alloc(Len);
		boost::scoped_array<int> A_perm_sign( new int[Len] );
		gsl_linalg_LU_decomp(A,A_perm,A_perm_sign.get());
		gsl_linalg_LU_invert(A,A_perm,A_i);

		gsl_blas_dgemv(CblasNoTrans,1.0,A_i,f,0.0,c);

		DT R = (DT)0;
		for( size_t a = 0; a<Len; ++a )
		{
			float3 V;
			V.x = Coords[a].x - LookupPos.x;
			V.y = Coords[a].y - LookupPos.y;
			V.z = Coords[a].z - LookupPos.z;
			float Len = sqrtf( V.x*V.x + V.y*V.y + V.z*V.z );

			R += gsl_vector_get(c,a)*TheBasisFunction(Len,Const);
		}
		return R;
	}
private:
	float mConst;
};
