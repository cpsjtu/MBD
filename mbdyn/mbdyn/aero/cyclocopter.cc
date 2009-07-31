/* $Header$ */
/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2009
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

/* Elementi di rotore */

#ifdef HAVE_CONFIG_H
#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */
#endif /* HAVE_CONFIG_H */

#include <limits>
#include <cmath>

#include "cyclocopter.h"
#include "dataman.h"

/* CyclocopterNoInflow - begin */

CyclocopterNoInflow::CyclocopterNoInflow(unsigned int uL, const DofOwner* pDO,
	const StructNode* pC, const Mat3x3& rrot,
	const StructNode* pR, ResForceSet **ppres, 
	flag fOut)
: Elem(uL, fOut),
InducedVelocity(uL, pDO, pC, ppres, fOut),
pRotor(pR),
RRot(rrot)
{
	NO_OP;	
}

CyclocopterNoInflow::~CyclocopterNoInflow(void)
{
	NO_OP;
}

InducedVelocity::Type
CyclocopterNoInflow::GetInducedVelocityType(void) const
{
	return InducedVelocity::CYCLOCOPTER;
}

void
CyclocopterNoInflow::AfterConvergence(const VectorHandler& X, const VectorHandler& XP)
{
	NO_OP;	
}

void
CyclocopterNoInflow::Output(OutputHandler& OH) const
{
	if (fToBeOutput()) {

                OH.Rotors()
                        << std::setw(8) << GetLabel()   /* 1 */
                        << " " << RRotTranspose*Res.Force()     /* 2-4 */
                        << " " << RRotTranspose*Res.Couple()    /* 5-7 */
                        << " " << "0."                	 /* 8 */
                        << " " << "0."                	 /* 9 */
                        << " " << "0."                	 /* 10 */
                        << " " << "0."                	 /* 11 */
                        << " " << "0."                	 /* 12 */
                        << " " << "0."                	 /* 13 */
                        << " " << "0."                	 /* 14 */
                        << " " << "0."   		 /* 15 */
                        << " " << "0."           	 /* 16 */
                        << std::endl;

                /* FIXME: check for parallel stuff ... */
                for (int i = 0; ppRes && ppRes[i]; i++) {
                        OH.Rotors()
                                << std::setw(8) << GetLabel()
                                << ":" << ppRes[i]->GetLabel()
                                << " " << ppRes[i]->pRes->Force()
                                << " " << ppRes[i]->pRes->Couple()
                                << std::endl;
                }
	}


}

std::ostream&
CyclocopterNoInflow::Restart(std::ostream& out) const
{
	return out << "# cyclocopter: not implemented yet" << std::endl;
}

void
CyclocopterNoInflow::SetInitialValue(VectorHandler& X)
{
	NO_OP;
}

SubVectorHandler&
CyclocopterNoInflow::AssRes(SubVectorHandler& WorkVec,
	doublereal dCoef,
	const VectorHandler& XCurr,
	const VectorHandler& XPrimeCurr)
{
	if (fToBeOutput() ){
		RRotTranspose = pCraft->GetRCurr()*RRot;
		RRotTranspose = RRotTranspose.Transpose();
	}
	
	ResetForce();
	WorkVec.Resize(0);	

	return WorkVec;
}

void
CyclocopterNoInflow::AddForce(unsigned int uL, const Vec3& F, const Vec3& M, const Vec3& X)
{
	/* Sole se deve fare l'output calcola anche il momento */
	if (fToBeOutput()) {
		Res.AddForces(F, M, X);
		InducedVelocity::AddForce(uL, F, M, X);
	}
}

Vec3
CyclocopterNoInflow::GetInducedVelocity(const Vec3& X) const
{
	
	return Zero3;
}

void
CyclocopterNoInflow::GetConnectedNodes(std::vector<const Node *>& connectedNodes) const
{
	connectedNodes.resize(1);
	connectedNodes[0] = pCraft;
}

/* CyclocopterNoInflow - end */

/* CyclocopterUniform1D - begin */

CyclocopterUniform1D::CyclocopterUniform1D(unsigned int uL, const DofOwner* pDO,
	const StructNode* pC, const Mat3x3& rrot,
	const StructNode* pR, ResForceSet **ppres, 
	const doublereal& dOR, const doublereal& dR,
	const doublereal& dL, const doublereal& dOmegaFilter,
	const doublereal& dDeltaT, DriveCaller *pdW, 
	flag fOut)
: Elem(uL, fOut),
InducedVelocity(uL, pDO, pC, ppres, fOut),
pRotor(pR),
RRot(rrot)
{
	ASSERT(dOR > 0.);	
	ASSERT(dR > 0.);	
	ASSERT(dL > 0.);	
	ASSERT(pdW != 0);	

	dOmegaRef = dOR;
	dRadius = dR;
	dSpan = dL;
	dArea = 2*dRadius*dSpan;

	Weight.Set(pdW);
	dWeight = 0.;
	
	dUind = 0.;
	dUindPrev = 0.;
	
	/* Butterworth discrete low-pass filter coefficients */
	if( dDeltaT > 0 && dOmegaFilter > 0 ) {
		doublereal dTmp = 4. + 2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter;
		a1 = (-8.+2*dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;
		a2 = (4. -2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;

		dTmp = dOmegaFilter*dOmegaFilter*dDeltaT*dDeltaT/dTmp;
		b0 = dTmp;
		b1 = 2*dTmp;
		b2 = dTmp;
	} else {
		a1 = 0.;
		a2 = 0.;
		b0 = 1.;
		b1 = 0.;
		b2 = 0.;
	}
	
	/* ingresso del filtro */
	Uk = 0.;
	Uk_1 = 0.;
	Uk_2 = 0.;
	/* uscita del filtro */
	Yk = 0.;
	Yk_1 = 0.;
	Yk_2 = 0.;

}

CyclocopterUniform1D::~CyclocopterUniform1D(void)
{
	NO_OP;
}

InducedVelocity::Type
CyclocopterUniform1D::GetInducedVelocityType(void) const
{
	return InducedVelocity::CYCLOCOPTER;
}

void
CyclocopterUniform1D::AfterConvergence(const VectorHandler& X, const VectorHandler& XP)
{

	/* aggiorno ingressi e uscite del filtro */
	Yk_2 = Yk_1;
	Yk_1 = Yk;
	Uk_2 = Uk_1;
	Uk_1 = Uk;
		
	dUindPrev = dUind;

	if (Weight.pGetDriveCaller() != 0) {
		dWeight = Weight.dGet();
		if (dWeight < 0.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay < 0.0; using 0.0" << std::endl);
			dWeight = 0.;
		} else if (dWeight > 1.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay > 1.0; using 1.0" << std::endl);
			dWeight = 1.;
		}
	}
			
	InducedVelocity::AfterConvergence(X, XP);
}

void
CyclocopterUniform1D::Output(OutputHandler& OH) const
{
	if (fToBeOutput()) {

                OH.Rotors()
                        << std::setw(8) << GetLabel()   /* 1 */
                        << " " << RRotTranspose*Res.Force()     /* 2-4 */
                        << " " << RRotTranspose*Res.Couple()    /* 5-7 */
                        << " " << dUind                	 /* 8 */
                        << " " << "0."                	 /* 9 */
                        << " " << "0."                	 /* 10 */
                        << " " << "0."                	 /* 11 */
                        << " " << "0."                	 /* 12 */
                        << " " << "0."                	 /* 13 */
                        << " " << "0."                	 /* 14 */
                        << " " << "0."   		 /* 15 */
                        << " " << "0."           	 /* 16 */
                        << std::endl;

                /* FIXME: check for parallel stuff ... */
                for (int i = 0; ppRes && ppRes[i]; i++) {
                        OH.Rotors()
                                << std::setw(8) << GetLabel()
                                << ":" << ppRes[i]->GetLabel()
                                << " " << ppRes[i]->pRes->Force()
                                << " " << ppRes[i]->pRes->Couple()
                                << std::endl;
                }
	}


}

std::ostream&
CyclocopterUniform1D::Restart(std::ostream& out) const
{
	return out << "# cyclocopter: not implemented yet" << std::endl;
}

void
CyclocopterUniform1D::SetInitialValue(VectorHandler& X)
{
	NO_OP;
}

SubVectorHandler&
CyclocopterUniform1D::AssRes(SubVectorHandler& WorkVec,
	doublereal dCoef,
	const VectorHandler& XCurr,
	const VectorHandler& XPrimeCurr)
{
	/* UNIFORM induced velocity (Moble version)*/
	/* Trasporta della matrice di rotazione del rotore */
	RRotTranspose = pCraft->GetRCurr()*RRot;
	RRot3 = RRotTranspose.GetVec(3);
	RRotTranspose = RRotTranspose.Transpose();
	doublereal dTz= RRot3*Res.Force();
	/* filtro le forze */
	Uk = dTz;
	Yk = -Yk_1*a1 - Yk_2*a2 + Uk*b0 + Uk_1*b1 + Uk_2*b2;
	dTz = Yk;	
	
	doublereal dRho = dGetAirDensity(GetXCurr());
	dUind = copysign(std::sqrt(std::abs(dTz)/(2*dRho*dArea)), dTz);

	dUind = (1 - dWeight)*dUind + dWeight*dUindPrev;
	
	ResetForce();
	WorkVec.Resize(0);	

	return WorkVec;
}

void
CyclocopterUniform1D::AddForce(unsigned int uL, const Vec3& F, const Vec3& M, const Vec3& X)
{
	/* Sole se deve fare l'output calcola anche il momento */
	if (fToBeOutput()) {
		Res.AddForces(F, M, X);
		InducedVelocity::AddForce(uL, F, M, X);
	} else {
		Res.AddForce(F);
	}
}

Vec3
CyclocopterUniform1D::GetInducedVelocity(const Vec3& X) const
{
	
	return RRot3*dUind;
}

void
CyclocopterUniform1D::GetConnectedNodes(std::vector<const Node *>& connectedNodes) const
{
	connectedNodes.resize(1);
	connectedNodes[0] = pCraft;
}

/* CyclocopterUniform1D - end */

/* CyclocopterUniform2D - begin */

CyclocopterUniform2D::CyclocopterUniform2D(unsigned int uL, const DofOwner* pDO,
	const StructNode* pC, const Mat3x3& rrot,
	const StructNode* pR, ResForceSet **ppres, 
	const doublereal& dOR, const doublereal& dR,
	const doublereal& dL, const doublereal& dOmegaFilter,
	const doublereal& dDeltaT, DriveCaller *pdW, 
	flag fOut)
: Elem(uL, fOut),
InducedVelocity(uL, pDO, pC, ppres, fOut),
pRotor(pR),
RRot(rrot)
{
	ASSERT(dOR > 0.);	
	ASSERT(dR > 0.);	
	ASSERT(dL > 0.);	
	ASSERT(pdW != 0);	

	dOmegaRef = dOR;
	dRadius = dR;
	dSpan = dL;
	dArea = 2*dRadius*dSpan;

	Weight.Set(pdW);
	dWeight = 0.;
	
	dUind = 0.;
	dUindPrev = 0.;
	dUindMagnitude = 0.;
	
	/* Butterworth discrete low-pass filter coefficients */
	if( dDeltaT > 0 && dOmegaFilter > 0 ) {
		doublereal dTmp = 4. + 2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter;
		a1 = (-8.+2*dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;
		a2 = (4. -2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;

		dTmp = dOmegaFilter*dOmegaFilter*dDeltaT*dDeltaT/dTmp;
		b0 = dTmp;
		b1 = 2*dTmp;
		b2 = dTmp;
	} else {
		a1 = 0.;
		a2 = 0.;
		b0 = 1.;
		b1 = 0.;
		b2 = 0.;
	}

	/* ingresso del filtro */
	Uk = 0.;
	Uk_1 = 0.;
	Uk_2 = 0.;
	/* uscita del filtro */
	Yk = 0.;
	Yk_1 = 0.;
	Yk_2 = 0.;

}

CyclocopterUniform2D::~CyclocopterUniform2D(void)
{
	NO_OP;
}

InducedVelocity::Type
CyclocopterUniform2D::GetInducedVelocityType(void) const
{
	return InducedVelocity::CYCLOCOPTER;
}

void
CyclocopterUniform2D::AfterConvergence(const VectorHandler& X, const VectorHandler& XP)
{

	dUindPrev = dUind;

	/* aggiorno ingressi e uscite del filtro */
	Yk_2 = Yk_1;
	Yk_1 = Yk;
	Uk_2 = Uk_1;
	Uk_1 = Uk;

	if (Weight.pGetDriveCaller() != 0) {
		dWeight = Weight.dGet();
		if (dWeight < 0.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay < 0.0; using 0.0" << std::endl);
			dWeight = 0.;
		} else if (dWeight > 1.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay > 1.0; using 1.0" << std::endl);
			dWeight = 1.;
		}
	}
			
	InducedVelocity::AfterConvergence(X, XP);
}

void
CyclocopterUniform2D::Output(OutputHandler& OH) const
{
	if (fToBeOutput()) {

                OH.Rotors()
                        << std::setw(8) << GetLabel()   /* 1 */
                        << " " << RRotor.MulTV(Res.Force())     /* 2-4 */
                        << " " << RRotor.MulTV(Res.Couple())    /* 5-7 */
                        << " " << dUindMagnitude               	 /* 8 */
                        << " " << dUind                	 /* 9 -11*/
                        << " " << "0."                	 /* 12 */
                        << " " << "0."                	 /* 13 */
                        << " " << "0."                	 /* 14 */
                        << " " << "0."   		 /* 15 */
                        << " " << "0."           	 /* 16 */
                        << std::endl;

                /* FIXME: check for parallel stuff ... */
                for (int i = 0; ppRes && ppRes[i]; i++) {
                        OH.Rotors()
                                << std::setw(8) << GetLabel()
                                << ":" << ppRes[i]->GetLabel()
                                << " " << ppRes[i]->pRes->Force()
                                << " " << ppRes[i]->pRes->Couple()
                                << std::endl;
                }
	}


}

std::ostream&
CyclocopterUniform2D::Restart(std::ostream& out) const
{
	return out << "# cyclocopter: not implemented yet" << std::endl;
}

void
CyclocopterUniform2D::SetInitialValue(VectorHandler& X)
{
	NO_OP;
}

SubVectorHandler&
CyclocopterUniform2D::AssRes(SubVectorHandler& WorkVec,
	doublereal dCoef,
	const VectorHandler& XCurr,
	const VectorHandler& XPrimeCurr)
{
	/* UNIFORM induced velocity */
	/* Trasporta della matrice di rotazione del rotore */
	RRotor = pCraft->GetRCurr()*RRot;
	RRotorTranspose = RRotor.Transpose();
	/* Forze nel sistema rotore */
	Vec3 F = RRotorTranspose*Res.Force();
	/* filtro le forze */
	Uk = F;
	Yk = -Yk_1*a1 - Yk_2*a2 + Uk*b0 + Uk_1*b1 + Uk_2*b2;
	F = Yk;	
	/* Forza nel piano normale all'asse di rotazione */
	doublereal dT= sqrt(F(2)*F(2) + F(3)*F(3));
	/* Velocità indotta: calcolata in base alla dT */
	doublereal dRho = dGetAirDensity(GetXCurr());
	dUindMagnitude = sqrt(dT/(2*dRho*dArea));
	/* Componenti della velocità indotta nel sistema 
	 * rotore */
	dUind = 0.;
	if (dT > std::numeric_limits<doublereal>::epsilon()) {
		dUind(2) = dUindMagnitude*F(2)/dT;
		dUind(3) = dUindMagnitude*F(3)/dT;
	}
	dUind(1) = (1 - dWeight)*dUind(1) + dWeight*dUindPrev(1);
	dUind(2) = (1 - dWeight)*dUind(2) + dWeight*dUindPrev(2);
	dUind(3) = (1 - dWeight)*dUind(3) + dWeight*dUindPrev(3);

	ResetForce();
	WorkVec.Resize(0);

	return WorkVec;
}

void
CyclocopterUniform2D::AddForce(unsigned int uL, const Vec3& F, const Vec3& M, const Vec3& X)
{
	/* Sole se deve fare l'output calcola anche il momento */
	if (fToBeOutput()) {
		Res.AddForces(F, M, X);
		InducedVelocity::AddForce(uL, F, M, X);

	} else {
		Res.AddForce(F);
	}
}

Vec3
CyclocopterUniform2D::GetInducedVelocity(const Vec3& X) const
{
	return RRotor*dUind;
}

void
CyclocopterUniform2D::GetConnectedNodes(std::vector<const Node *>& connectedNodes) const
{
	connectedNodes.resize(1);
	connectedNodes[0] = pCraft;
}

/* CyclocopterUniform2D - end */

/* CyclocopterMasarati - begin */

CyclocopterMasarati::CyclocopterMasarati(unsigned int uL, const DofOwner* pDO,
	const StructNode* pC, const Mat3x3& rrot,
	const StructNode* pR, ResForceSet **ppres, 
	const doublereal& dOR, const doublereal& dR,
	const doublereal& dL, const doublereal& dOmegaFilter,
	const doublereal& dDeltaT, DriveCaller *pdW, 
	flag fOut)
: Elem(uL, fOut),
InducedVelocity(uL, pDO, pC, ppres, fOut),
pRotor(pR),
RRot(rrot)
{
	ASSERT(dOR > 0.);	
	ASSERT(dR > 0.);	
	ASSERT(dL > 0.);	
	ASSERT(pdW != 0);	

	dOmegaRef = dOR;
	dRadius = dR;
	dSpan = dL;
	dArea = 2*dRadius*dSpan;

	Weight.Set(pdW);
	dWeight = 0.;
	
	dUindMean = 0.;
	dUindMeanPrev = 0.;
	dUindMeanMagnitude = 0.;
	
	/* Butterworth discrete low-pass filter coefficients */
	if( dDeltaT > 0 && dOmegaFilter > 0 ) {
		doublereal dTmp = 4. + 2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter;
		a1 = (-8.+2*dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;
		a2 = (4. -2*sqrt(2)*dOmegaFilter*dDeltaT + dDeltaT*dDeltaT*dOmegaFilter*dOmegaFilter)/dTmp;

		dTmp = dOmegaFilter*dOmegaFilter*dDeltaT*dDeltaT/dTmp;
		b0 = dTmp;
		b1 = 2*dTmp;
		b2 = dTmp;
	} else {
		a1 = 0.;
		a2 = 0.;
		b0 = 1.;
		b1 = 0.;
		b2 = 0.;
	}

	/* Butterworth discrete low-pass filter (dt = 1e-4; fcut = 3.3Hz); */
	//a1 = -1.997067699357293e+00;
	//a2 = 9.970719922613875e-01;
	//b0 = 1.073226023551310e-06;
	//b1 = 2.146452047102620e-06;
	//b2 = 1.073226023551310e-06;
	
	/* ingresso del filtro */
	Uk = 0.;
	Uk_1 = 0.;
	Uk_2 = 0.;
	/* uscita del filtro */
	Yk = 0.;
	Yk_1 = 0.;
	Yk_2 = 0.;

	
}

CyclocopterMasarati::~CyclocopterMasarati(void)
{
	NO_OP;
}

InducedVelocity::Type
CyclocopterMasarati::GetInducedVelocityType(void) const
{
	return InducedVelocity::CYCLOCOPTER;
}

void
CyclocopterMasarati::AfterConvergence(const VectorHandler& X, const VectorHandler& XP)
{

	dUindMeanPrev = dUindMean;

	/* aggiorno ingressi e uscite del filtro */
	Yk_2 = Yk_1;
	Yk_1 = Yk;
	Uk_2 = Uk_1;
	Uk_1 = Uk;

	if (Weight.pGetDriveCaller() != 0) {
		dWeight = Weight.dGet();
		if (dWeight < 0.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay < 0.0; using 0.0" << std::endl);
			dWeight = 0.;
		} else if (dWeight > 1.) {
			silent_cout("Rotor(" << GetLabel() << "): "
				"delay > 1.0; using 1.0" << std::endl);
			dWeight = 1.;
		}
	}
			
	InducedVelocity::AfterConvergence(X, XP);
}

void
CyclocopterMasarati::Output(OutputHandler& OH) const
{
	if (fToBeOutput()) {

                OH.Rotors()
                        << std::setw(8) << GetLabel()   /* 1 */
                        << " " << RRotorTranspose*Res.Force()     /* 2-4 */
                        << " " << RRotorTranspose*Res.Couple()    /* 5-7 */
                        << " " << dUindMeanMagnitude               	 /* 8 */
                        << " " << dUindMean                	 /* 9 -11*/
                        << " " << dXi                	 /* 12 */
                        << " " << "0."               	 /* 13 */
                        << " " << "0."                	 /* 14 */
                        << " " << "0."   		 /* 15 */
                        << " " << "0."           	 /* 16 */
                        << std::endl;

                /* FIXME: check for parallel stuff ... */
                for (int i = 0; ppRes && ppRes[i]; i++) {
                        OH.Rotors()
                                << std::setw(8) << GetLabel()
                                << ":" << ppRes[i]->GetLabel()
                                << " " << ppRes[i]->pRes->Force()
                                << " " << ppRes[i]->pRes->Couple()
                                << std::endl;
                }
	}


}

std::ostream&
CyclocopterMasarati::Restart(std::ostream& out) const
{
	return out << "# cyclocopter: not implemented yet" << std::endl;
}

void
CyclocopterMasarati::SetInitialValue(VectorHandler& X)
{
	NO_OP;
}

SubVectorHandler&
CyclocopterMasarati::AssRes(SubVectorHandler& WorkVec,
	doublereal dCoef,
	const VectorHandler& XCurr,
	const VectorHandler& XPrimeCurr)
{
	/* UNIFORM induced velocity */
	/* Trasporta della matrice di rotazione del rotore */
	RRotor = pCraft->GetRCurr()*RRot;
	RRotorTranspose = RRotor.Transpose();
	/* Forze nel sistema rotore */
	Vec3 F = RRotorTranspose*Res.Force();
	/* filtro le forze */
	Uk = F;
	Yk = -Yk_1*a1 - Yk_2*a2 + Uk*b0 + Uk_1*b1 + Uk_2*b2;
	F = Yk;	
	/* Forza nel piano normale all'asse di rotazione */
	doublereal dT= sqrt( F(2)*F(2)+F(3)*F(3) );
	/* angolo di cui è ruotata la trazione */
	dXi = atan2(F(3), F(2)) - M_PI/2.;
	/* Velocità indotta: calcolata in base alla dT */
	doublereal dRho = dGetAirDensity(GetXCurr());
	dUindMeanMagnitude = sqrt( dT/(2*dRho*dArea) );
	/* Componenti della velocità indotta nel sistema 
	 * rotore */
	dUindMean = 0.;
	if (dT > std::numeric_limits<doublereal>::epsilon()) {
		dUindMean(2) = dUindMeanMagnitude*F(2)/dT;
		dUindMean(3) = dUindMeanMagnitude*F(3)/dT;
	}
	dUindMean(1) = (1-dWeight)*dUindMean(1)+dWeight*dUindMeanPrev(1);
	dUindMean(2) = (1-dWeight)*dUindMean(2)+dWeight*dUindMeanPrev(2);
	dUindMean(3) = (1-dWeight)*dUindMean(3)+dWeight*dUindMeanPrev(3);

	
	ResetForce();
	WorkVec.Resize(0);	

	return WorkVec;
}

void
CyclocopterMasarati::AddForce(unsigned int uL, const Vec3& F, const Vec3& M, const Vec3& X)
{
	/* Sole se deve fare l'output calcola anche il momento */
	if (fToBeOutput()) {
		Res.AddForces(F,M,X);
		InducedVelocity::AddForce(uL, F, M, X);
	} else {
		Res.AddForce(F);
	}
}

Vec3
CyclocopterMasarati::GetInducedVelocity(const Vec3& X) const
{

	//Vec3 XRel(RRotorTranspose*(X-Res.Pole()));
	Vec3 XRel(RRotorTranspose*(X-pRotor->GetXCurr()));

	doublereal d1 = XRel.dGet(2);
	doublereal d2 = XRel.dGet(3);

	/* dPsi0 non serve a nulla perchè uso l'angolo
	 * relativo: (dp-dXi)!!! */
	doublereal dp = atan2(d2, d1);

	doublereal r = sqrt(d1*d1+d2*d2)*cos(dp-dXi);
	
	Vec3 dUind = 0.;
	dUind(1) = dUindMean(1)*(M_PI/2.)*cos((M_PI/2.)*(r/dRadius));
	dUind(2) = dUindMean(2)*(M_PI/2.)*cos((M_PI/2.)*(r/dRadius));
	dUind(3) = dUindMean(3)*(M_PI/2.)*cos((M_PI/2.)*(r/dRadius));

	return RRotor*dUind;
}

void
CyclocopterMasarati::GetConnectedNodes(std::vector<const Node *>& connectedNodes) const
{
	connectedNodes.resize(1);
	connectedNodes[0] = pCraft;
}

/* CyclocopterMasarati - end */

/* CyclocopterKARI - begin */

CyclocopterKARI::CyclocopterKARI(unsigned int uL, const DofOwner* pDO,
	const StructNode* pC, const Mat3x3& rrot,
	const StructNode* pR, ResForceSet **ppres, flag fOut)
: Elem(uL, fOut),
InducedVelocity(uL, pDO, pC, ppres, fOut),
pRotor(pR),
RRot(rrot)
{	
	NO_OP;
}

CyclocopterKARI::~CyclocopterKARI(void)
{
	NO_OP;
}

InducedVelocity::Type
CyclocopterKARI::GetInducedVelocityType(void) const
{
	return InducedVelocity::CYCLOCOPTER;
}

void
CyclocopterKARI::AfterConvergence(const VectorHandler& X, const VectorHandler& XP)
{
	NO_OP;
}

void
CyclocopterKARI::Output(OutputHandler& OH) const
{
	NO_OP;
}

std::ostream&
CyclocopterKARI::Restart(std::ostream& out) const
{
	return out << "# cyclocopter: not implemented yet" << std::endl;
}

void
CyclocopterKARI::SetInitialValue(VectorHandler& X)
{
	NO_OP;
}

SubVectorHandler&
CyclocopterKARI::AssRes(SubVectorHandler& WorkVec,
	doublereal dCoef,
	const VectorHandler& XCurr,
	const VectorHandler& XPrimeCurr)
{	
	return WorkVec;
}

#if 0
void
CyclocopterKARI::AddForce(unsigned int uL, const Vec3& F, const Vec3& M, const Vec3& X)
{
	NO_OP;
}
#endif

Vec3
CyclocopterKARI::GetInducedVelocity(const Vec3& X) const
{
	return Zero3;
}

void
CyclocopterKARI::GetConnectedNodes(std::vector<const Node *>& connectedNodes) const
{
	connectedNodes.resize(1);
	connectedNodes[0] = pCraft;
}

/* CyclocopterKARI - end */

Elem*
ReadCyclocopter(DataManager* pDM,
	MBDynParser& HP,
	const DofOwner* pDO, 
	unsigned int uLabel,
	const StructNode* pCraft,
	const Mat3x3& rrot,
	const StructNode* pRotor)
{
	Elem *pEl = 0;

	const char* sKeyWords[] = {
		"type",
		"no",
		"uniform1D",
		"uniform2D",
		"masarati",
		"KARI",
		NULL
	};

	enum KeyWords {
		UNKNOWN = -1,
		type = 0,
		NO,
		uniform1D,
		uniform2D,
		masarati,
		KARI,

		LASTKEYWORD
	};

	KeyTable K(HP, sKeyWords);

	KeyWords CyclocopterInducedType = NO;
	if (HP.IsArg() && HP.IsKeyWord("type")) {
        	CyclocopterInducedType = KeyWords(HP.GetWord());
	}

	switch( CyclocopterInducedType ) {
	case NO: {
		ResForceSet **ppres = ReadResSets(pDM, HP);

	 	flag fOut = pDM->fReadOutput(HP, Elem::INDUCEDVELOCITY);
		pEl = new CyclocopterNoInflow(uLabel, pDO,
			pCraft, rrot, pRotor,
  			ppres, fOut);
 
		break;
	}
	case uniform1D:	{
		doublereal dOR = HP.GetReal();
		if (dOR <= 0.) {
			silent_cerr("Illegal null or negative "
				"reference speed for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dR = HP.GetReal();
		if (dR <= 0.) {
			silent_cerr("Illegal null or negative radius"
				"for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dL = HP.GetReal();
		if (dL <= 0.) {
			silent_cerr("Illegal null or negative blade"
				"length for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	
		DriveCaller *pdW = 0;
		if (HP.IsKeyWord("delay")) {
			pdW = HP.GetDriveCaller();
		} else {
			SAFENEW( pdW, NullDriveCaller);
		}

		doublereal dOmegaFilter = 0.;
		if (HP.IsKeyWord("omegacut")) {
			dOmegaFilter = HP.GetReal();
			if (dOmegaFilter <= 0){
				silent_cerr("Illegal null or negative filter"
					"cut frequency for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dOmegaFilter = 0.;
		}

		doublereal dDeltaT = 0.;
		if (HP.IsKeyWord("timestep")) {
			dDeltaT = HP.GetReal();
			if (dDeltaT <= 0){
				silent_cerr("Illegal null or negative time"
					"step for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dDeltaT = 0.;
		}

     		ResForceSet **ppres = ReadResSets(pDM, HP);

	 	flag fOut = pDM->fReadOutput(HP, Elem::INDUCEDVELOCITY);
		pEl = new CyclocopterUniform1D(uLabel, pDO,
			pCraft, rrot, pRotor,
  			ppres, dOR, dR, dL,
			dOmegaFilter, dDeltaT, pdW, fOut);
		
		break;
	}
	case uniform2D:	{
		doublereal dOR = HP.GetReal();
		if (dOR <= 0.) {
			silent_cerr("Illegal null or negative "
				"reference speed for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dR = HP.GetReal();
		if (dR <= 0.) {
			silent_cerr("Illegal null or negative radius"
				"for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dL = HP.GetReal();
		if (dL <= 0.) {
			silent_cerr("Illegal null or negative blade"
				"length for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	
		DriveCaller *pdW = 0;
		if (HP.IsKeyWord("delay")) {
			pdW = HP.GetDriveCaller();
		} else {
			SAFENEW( pdW, NullDriveCaller);
		}

		doublereal dOmegaFilter = 0.;
		if (HP.IsKeyWord("omegacut")) {
			dOmegaFilter = HP.GetReal();

			if (dOmegaFilter <= 0){
				silent_cerr("Illegal null or negative filter"
					"cut frequency for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dOmegaFilter = 0.;
		}

		doublereal dDeltaT = 0.;
		if (HP.IsKeyWord("timestep")) {
			dDeltaT = HP.GetReal();
			if (dDeltaT <= 0){
				silent_cerr("Illegal null or negative time"
					"step for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dDeltaT = 0.;
		}

     		ResForceSet **ppres = ReadResSets(pDM, HP);

	 	flag fOut = pDM->fReadOutput(HP, Elem::INDUCEDVELOCITY);
		pEl = new CyclocopterUniform2D(uLabel, pDO,
			pCraft, rrot, pRotor,
  			ppres, dOR, dR, dL,
			dOmegaFilter, dDeltaT, pdW, fOut);
		
		break;
	}
	case masarati:	{
		doublereal dOR = HP.GetReal();
		if (dOR <= 0.) {
			silent_cerr("Illegal null or negative "
				"reference speed for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dR = HP.GetReal();
		if (dR <= 0.) {
			silent_cerr("Illegal null or negative radius"
				"for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		doublereal dL = HP.GetReal();
		if (dL <= 0.) {
			silent_cerr("Illegal null or negative blade"
				"length for rotor" << uLabel
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	
		DriveCaller *pdW = 0;
		if (HP.IsKeyWord("delay")) {
			pdW = HP.GetDriveCaller();
		} else {
			SAFENEW( pdW, NullDriveCaller);
		}

		doublereal dOmegaFilter = 0.;
		if (HP.IsKeyWord("omegacut")) {
			dOmegaFilter = HP.GetReal();
			if (dOmegaFilter <= 0){
				silent_cerr("Illegal null or negative filter"
					"cut frequency for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dOmegaFilter = 0.;
		}

		doublereal dDeltaT = 0.;
		if (HP.IsKeyWord("timestep")) {
			dDeltaT = HP.GetReal();
			if (dDeltaT <= 0){
				silent_cerr("Illegal null or negative time"
					"step for rotor" << uLabel
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw DataManager::ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		} else {
			dDeltaT = 0.;
		}

     		ResForceSet **ppres = ReadResSets(pDM, HP);

	 	flag fOut = pDM->fReadOutput(HP, Elem::INDUCEDVELOCITY);
		pEl = new CyclocopterMasarati(uLabel, pDO,
			pCraft, rrot, pRotor,
  			ppres, dOR, dR, dL,
			dOmegaFilter, dDeltaT, pdW, fOut);
		
		break;
	}
	case KARI: {
     		ResForceSet **ppres = ReadResSets(pDM, HP);

	 	flag fOut = pDM->fReadOutput(HP, Elem::INDUCEDVELOCITY);
		pEl = new CyclocopterKARI(uLabel, pDO,
			pCraft, rrot, pRotor,
  			ppres, fOut);
		break;
	}
	default:
		silent_cerr("Rotor(" << uLabel << "): "
			"unknown cyclocopter inflow model "
			"at line " << HP.GetLineData()
			<< std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	return pEl;
}
