/* $Header$ */
/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2012
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
  *
  * Copyright (C) 2003-2012
  * Giuseppe Quaranta	<quaranta@aero.polimi.it>
  *
  * classi che implementano la risoluzione del sistema nonlineare 
  */
  
#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */

#include <unistd.h>

#include "solver.h"
#include "nr.h"  
#ifdef USE_MPI
#include "mbcomm.h"
#include "schsolman.h"
#endif /* USE_MPI */

#include "dofown.h"
#include "output.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>

NewtonRaphsonSolver::NewtonRaphsonSolver(const bool bTNR,
		const bool bKJ, 
		const integer IterBfAss,
		bool JacReq)
: NonlinearSolver(JacReq), pRes(NULL),
pSol(NULL),
bTrueNewtonRaphson(bTNR),
IterationBeforeAssembly(IterBfAss),
bKeepJac(bKJ),
iPerformedIterations(0),
pPrevNLP(0)
{
	NO_OP;
}

NewtonRaphsonSolver::~NewtonRaphsonSolver(void)
{
	NO_OP;
}

void
NewtonRaphsonSolver::Solve(const NonlinearProblem *pNLP,
		Solver *pS,
		const integer iMaxIter,
		const doublereal& Tol,
		integer& iIterCnt,
		doublereal& dErr,
		const doublereal& SolTol,
		doublereal& dSolErr)
{
	ASSERT(pS != NULL);
	SolutionManager *pSM = pS->pGetSolutionManager();
	
	iIterCnt = 0;
	if ((!bKeepJac) || (pNLP != pPrevNLP)) {
		iPerformedIterations = 0;
	}
	pPrevNLP = pNLP;
	dSolErr = 0.;

	doublereal dOldErr;
	doublereal dErrFactor = 1.;
	bool bJacBuilt = false;

	while (true) {
		pRes = pSM->pResHdl();
		pSol = pSM->pSolHdl();
		Size = pRes->iGetSize();

#ifdef USE_EXTERNAL
		SendExternal();
#endif /* USE_EXTERNAL */

		pRes->Reset();
		bool forceJacobian(false);
		try {
	      		pNLP->Residual(pRes);
		}
		catch (SolutionDataManager::ChangedEquationStructure) {
			if (bHonorJacRequest) {
				forceJacobian = true;
			}
		}

      		if (outputRes()) {
			pS->PrintResidual(*pRes, iIterCnt);
      		}

		/* FIXME: if Tol == 0., no convergence on residual
		 * is required, so we could simply don't compute
		 * the test; I'm leaving it in place so it appears
		 * in the output (maybe we could conditionally disable 
		 * it? */

		bool bTest = MakeResTest(pS, pNLP, *pRes, Tol, dErr);
		if (iIterCnt > 0) {
			dErrFactor *= dErr/dOldErr;
		}
		dOldErr = dErr;

#ifdef USE_MPI
		if (!bParallel || MBDynComm.Get_rank() == 0)
#endif /* USE_MPI */
		{
			if (outputIters() || outputSolverConditionNumber()) {
				if (outputIters()) {
					silent_cout("\tIteration(" << iIterCnt << ") " << dErr);

					if (bJacBuilt) {
						silent_cout(" J");
					}
				}

				if (outputSolverConditionNumber()) {
					silent_cout(" cond=");
					doublereal dCond;
					if (pSM->bGetConditionNumber(dCond)) {
						silent_cout(dCond);
					} else {
						silent_cout("NA");
					}
				}

				silent_cout(std::endl);
			}
		}
		
      	if (bTest) {
	 		return;
      	}
      		
		if (iIterCnt >= std::abs(iMaxIter)) {
			if (iMaxIter < 0 && dErrFactor < 1.) {
				return;
			}
			if (outputBailout()) {
				pS->PrintResidual(*pRes, iIterCnt);
			}
			throw NoConvergence(MBDYN_EXCEPT_ARGS);
		}
          
      	iIterCnt++;
      	bJacBuilt = false;

		if (bTrueNewtonRaphson
			|| (iPerformedIterations%IterationBeforeAssembly == 0)
			|| forceJacobian)
		{
      			pSM->MatrReset();
rebuild_matrix:;
			try {
      				pNLP->Jacobian(pSM->pMatHdl());
			} catch (MatrixHandler::ErrRebuildMatrix) {
				silent_cout("NewtonRaphsonSolver: "
						"rebuilding matrix..."
						<< std::endl);

				/* need to rebuild the matrix... */
      				pSM->MatrInitialize();
				/* FIXME: could loop forever! */
				goto rebuild_matrix;

			} catch (...) {
				throw;
			}

			TotJac++;
			bJacBuilt = true;
		}
		
		iPerformedIterations++;
		
#ifdef USE_MPI
		if (!bParallel || MBDynComm.Get_rank() == 0)
#endif /* USE_MPI */
		{
			if (outputJac()) {
				if (bJacBuilt) {
					silent_cout("Jacobian:" << std::endl
							<< *(pSM->pMatHdl()));
				} else {
					silent_cout("Jacobian: unchanged" << std::endl);
				}
			}

			if (bJacBuilt && outputMatrixConditionNumber()) {
				silent_cout("cond=" << pSM->pMatHdl()->ConditionNumber(GetCondMatNorm()) << std::endl);
			}
		}

		pSM->Solve();

      	if (outputSol()) {
			pS->PrintSolution(*pSol, iIterCnt);
		}		
		
      	pNLP->Update(pSol);
		
		bTest = MakeSolTest(pS, *pSol, SolTol, dSolErr);

		if (outputIters()) {
#ifdef USE_MPI
			if (!bParallel || MBDynComm.Get_rank() == 0)
#endif /* USE_MPI */
			{
				silent_cout("\t\tSolErr "
					<< dSolErr << std::endl);
			}
		}

		if (bTest) {
			throw ConvergenceOnSolution(MBDYN_EXCEPT_ARGS);
		}

		// allow to bail out in case of multiple CTRL^C
		if (mbdyn_stop_at_end_of_iteration()) {
			throw ErrInterrupted(MBDYN_EXCEPT_ARGS);
		}
	}
}
