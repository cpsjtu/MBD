/* $Header$ */
/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2007
 *
 * Pierangelo Masarati	<masarati@aero.polimi.it>
 * Paolo Mantegazza	<mantegazza@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 * 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef CONSTLAW_ANN_H
#define CONSTLAW_ANN_H

#include "dataman.h"
#include "constltp.h"
#include "ann.h"

template <class T, class Tder>
class AnnElasticConstitutiveLaw
: public ConstitutiveLaw<T, Tder> {
private:

public:
	AnnElasticConstitutiveLaw(void)
	{
		NO_OP;
	};

	virtual ~AnnElasticConstitutiveLaw(void) {
		NO_OP;
	};

	ConstLawType::Type GetConstLawType(void) const {
		return ConstLawType::ELASTIC;
	};

	virtual ConstitutiveLaw<T, Tder>* pCopy(void) const {
		ConstitutiveLaw<T, Tder>* pCL = NULL;

		typedef AnnElasticConstitutiveLaw<T, Tder> cl;
		SAFENEWWITHCONSTRUCTOR(pCL, cl, cl());
		return pCL;
	};

	virtual std::ostream& Restart(std::ostream& out) const {
		return out << " /* ann not implemented yet */,";
	};

	virtual void Update(const T& Eps, const T& /* EpsPrime */  = 0.) {
		ConstitutiveLaw<T, Tder>::Epsilon = Eps;

		T E = ConstitutiveLaw<T, Tder>::Epsilon;

		ConstitutiveLaw<T, Tder>::F = 0.;
		ConstitutiveLaw<T, Tder>::FDE = 0.;
	};

	virtual void IncrementalUpdate(const T& DeltaEps, const T& /* EpsPrime */ = 0.) {
		Update(ConstitutiveLaw<T, Tder>::Epsilon + DeltaEps);
	};
};

template <class T, class Tder>
class AnnViscoElasticConstitutiveLaw
: public ConstitutiveLaw<T, Tder> {
private:

public:
	AnnViscoElasticConstitutiveLaw(void)
	{
		NO_OP;
	};

	virtual ~AnnViscoElasticConstitutiveLaw(void) {
		NO_OP;
	};

	ConstLawType::Type GetConstLawType(void) const {
		return ConstLawType::VISCOELASTIC;
	};

	virtual ConstitutiveLaw<T, Tder>* pCopy(void) const {
		ConstitutiveLaw<T, Tder>* pCL = NULL;

		typedef AnnViscoElasticConstitutiveLaw<T, Tder> cl;
		SAFENEWWITHCONSTRUCTOR(pCL, cl, cl());
		return pCL;
	};

	virtual std::ostream& Restart(std::ostream& out) const {
		return out << " /* ann not implemented yet */,";
	};

	virtual void Update(const T& Eps, const T& EpsPrime = 0.) {
		ConstitutiveLaw<T, Tder>::Epsilon = Eps;
		ConstitutiveLaw<T, Tder>::EpsilonPrime = EpsPrime;

		T E = ConstitutiveLaw<T, Tder>::Epsilon;

		ConstitutiveLaw<T, Tder>::F = 0.;
		ConstitutiveLaw<T, Tder>::FDE = 0.;
	};

	virtual void IncrementalUpdate(const T& DeltaEps, const T& EpsPrime = 0.) {
		Update(ConstitutiveLaw<T, Tder>::Epsilon + DeltaEps, EpsPrime);
	};
};

/* specific functional object(s) */
template <class T, class Tder>
struct AnnElasticCLR : public ConstitutiveLawRead<T, Tder> {
	virtual ConstitutiveLaw<T, Tder> *
	Read(const DataManager* pDM, MBDynParser& HP, ConstLawType::Type& CLType) {
		ConstitutiveLaw<T, Tder>* pCL = 0;

		CLType = ConstLawType::VISCOELASTIC;

		typedef AnnElasticConstitutiveLaw<T, Tder> L;
		SAFENEWWITHCONSTRUCTOR(pCL, L, L());

		return pCL;
	};
};

template <class T, class Tder>
struct AnnViscoElasticCLR : public ConstitutiveLawRead<T, Tder> {
	virtual ConstitutiveLaw<T, Tder> *
	Read(const DataManager* pDM, MBDynParser& HP, ConstLawType::Type& CLType) {
		ConstitutiveLaw<T, Tder>* pCL = 0;

		CLType = ConstLawType::VISCOELASTIC;

		typedef AnnViscoElasticConstitutiveLaw<T, Tder> L;
		SAFENEWWITHCONSTRUCTOR(pCL, L, L());

		return pCL;
	};
};

#endif // CONSTLAW_ANN_H
