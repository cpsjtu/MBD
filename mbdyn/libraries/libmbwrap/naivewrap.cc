/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2005
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

/*
 * The Naive Solver is copyright (C) 2004 by
 * Paolo Mantegazza <mantegazza@aero.polimi.it>
 */

#ifdef HAVE_CONFIG_H
#include <mbconfig.h>           /* This goes first in every *.c,*.cc file */
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "spmh.h"
#include "spmapmh.h"
#include "naivewrap.h"
#include "mthrdslv.h"

/* NaiveSolver - begin */
NaiveSolver::NaiveSolver(const integer &size, const doublereal& dMP,
		NaiveMatrixHandler *const a)
: LinearSolver(0),
iSize(size),
dMinPiv(dMP),
piv(size),
A(a)
{
	NO_OP;
}

NaiveSolver::~NaiveSolver(void)
{
	NO_OP;
}

void
NaiveSolver::SetMat(NaiveMatrixHandler *const a)
{
	A = a;
}

void
NaiveSolver::Reset(void)
{
	bHasBeenReset = true;
}

void
NaiveSolver::Solve(void) const
{
	if (bHasBeenReset) {
      		((NaiveSolver *)this)->Factor();
      		bHasBeenReset = false;
	}

	naivslv(A->ppdRows, iSize, A->piNzc, A->ppiCols,
			LinearSolver::pdRhs, LinearSolver::pdSol, &piv[0]);
}

void
NaiveSolver::Factor(void)
{
	int		rc;

#if 0
	integer i = 0;
	for (integer j = 0; j < iSize; j++) {
		i += A->piNzr[j];
	}
	std::cerr << "prima: " << i << std::endl;
#endif
	
	rc = naivfct(A->ppdRows, iSize,
			A->piNzr, A->ppiRows, 
			A->piNzc, A->ppiCols,
			A->ppnonzero, 
			&piv[0], dMinPiv);

#if 0
	i = 0;
	for (integer j = 0; j < iSize; j++) {
		i += A->piNzr[j];
	}
	std::cerr << "dopo: " << i << std::endl;
#endif
	
	if (rc) {
		if (rc & ENULCOL) {
			silent_cerr("NaiveSolver: ENULCOL("
					<< (rc & ~ENULCOL) << ")" << std::endl);
			throw ErrGeneric();
		}

		if (rc & ENOPIV) {
			silent_cerr("NaiveSolver: ENOPIV("
					<< (rc & ~ENOPIV) << ")" << std::endl);
			throw ErrGeneric();
		}

		/* default */
		throw ErrGeneric();
	}
}

/* NaiveSolver - end */

/* NaiveSparseSolutionManager - begin */

NaiveSparseSolutionManager::NaiveSparseSolutionManager(const integer Dim,
		const doublereal dMP)
: A(0),
VH(Dim),
XH(Dim)
{
	SAFENEWWITHCONSTRUCTOR(A, NaiveMatrixHandler, NaiveMatrixHandler(Dim));
	SAFENEWWITHCONSTRUCTOR(pLS, NaiveSolver, 
		NaiveSolver(Dim, dMP, A));

	pLS->ChangeResPoint(VH.pdGetVec());
	pLS->ChangeSolPoint(XH.pdGetVec());

	pLS->SetSolutionManager(this);
}

NaiveSparseSolutionManager::~NaiveSparseSolutionManager(void) 
{
	if (A != 0) {
		SAFEDELETE(A);
		A = 0;
	}
}

void
NaiveSparseSolutionManager::MatrReset()
{
	pLS->Reset();
}

/* Risolve il sistema  Fattorizzazione + Bacward Substitution*/
void
NaiveSparseSolutionManager::Solve(void)
{
	pLS->Solve();
}

/* Rende disponibile l'handler per la matrice */
MatrixHandler*
NaiveSparseSolutionManager::pMatHdl(void) const
{
	return A;
}

/* Rende disponibile l'handler per il termine noto */
MyVectorHandler*
NaiveSparseSolutionManager::pResHdl(void) const
{
	return &VH;
}

/* Rende disponibile l'handler per la soluzione */
MyVectorHandler*
NaiveSparseSolutionManager::pSolHdl(void) const
{
	return &XH;
}

/* NaiveSparseSolutionManager - end */

/* NaivePermSparseSolutionManager - begin */

extern "C" {
#include "colamd.h"
}

NaiveSparsePermSolutionManager::NaiveSparsePermSolutionManager(const integer Dim, 
	const doublereal dMP)
: NaiveSparseSolutionManager(Dim, dMP),
dMinPiv(dMP),
ePermState(PERM_NO)
{
	perm.resize(Dim, 0);
	invperm.resize(Dim, 0);

	SAFEDELETE(A);
	A = 0;
	SAFENEWWITHCONSTRUCTOR(A, NaivePermMatrixHandler, NaivePermMatrixHandler(Dim, &perm[0]));

	dynamic_cast<NaiveSolver *>(pLS)->SetMat(A);

	MatrInitialize();
}

NaiveSparsePermSolutionManager::~NaiveSparsePermSolutionManager(void) 
{
	NO_OP;
}

void
NaiveSparsePermSolutionManager::MatrReset(void)
{
	if (ePermState == PERM_INTERMEDIATE) {
		ePermState = PERM_READY;

		pLS->ChangeResPoint(VH.pdGetVec());
		pLS->ChangeSolPoint(XH.pdGetVec());

		pLS->SetSolutionManager(this);
	}

	pLS->Reset();
}

void
NaiveSparsePermSolutionManager::ComputePermutation(void)
{
	std::vector<integer> Ai;
	A->MakeCCStructure(Ai, invperm);
	doublereal knobs[COLAMD_KNOBS];
	integer stats[COLAMD_STATS];
	integer Alen = colamd_recommended (Ai.size(), A->iGetNumRows(), A->iGetNumCols());
	Ai.resize(Alen);
	colamd_set_defaults(knobs);
	if (!colamd(A->iGetNumRows(), A->iGetNumCols(), Alen,
		&(Ai[0]), &invperm[0], knobs, stats)) {
		silent_cerr("colamd permutation failed" << std::endl);
		throw ErrGeneric();
	}
	for (integer i = 0; i < A->iGetNumRows(); i++) {
		perm[invperm[i]] = i;
	}
	ePermState = PERM_INTERMEDIATE;
}

void
NaiveSparsePermSolutionManager::BackPerm(void)
{
	for (integer i = 0; i < A->iGetNumCols(); i++) {
		XH.PutCoef(invperm[i] + 1, VH.dGetCoef(i + 1));
	}
}


/* Risolve il sistema  Fattorizzazione + Bacward Substitution*/
void
NaiveSparsePermSolutionManager::Solve(void)
{
	if (ePermState == PERM_NO) {
		ComputePermutation();

	} else {
		pLS->ChangeSolPoint(VH.pdGetVec());
	}

	pLS->Solve();

	if (ePermState == PERM_READY) {
		BackPerm();
		pLS->ChangeSolPoint(XH.pdGetVec());
	}
}

/* Inizializzatore "speciale" */
void
NaiveSparsePermSolutionManager::MatrInitialize()
{
	ePermState = PERM_NO;
	for (integer i = 0; i < A->iGetNumRows(); i++) {
		perm[i] = i;
		invperm[i] = i;
	}

	MatrReset();
}
	
/* NaivePermSparseSolutionManager - end */

