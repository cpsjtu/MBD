/*
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 2003-2005
 * 
 * This code is a partial merge of HmFe and MBDyn.
 *
 * Pierangelo Masarati  <masarati@aero.polimi.it>
 * Paolo Mantegazza     <mantegazza@aero.polimi.it>
 * Marco Morandini  <morandini@aero.polimi.it>
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

#ifndef DirCColMatrixHandler_hh
#define DirCColMatrixHandler_hh

#include <vector>

#include "myassert.h"
#include "solman.h"
#include "spmh.h"

/* Sparse Matrix in columns form */
template <int off>
class DirCColMatrixHandler : public CompactSparseMatrixHandler {
private:
#ifdef DEBUG
	void IsValid(void) const {
		NO_OP;
	};
#endif /* DEBUG */
	std::vector<integer *> pindices;
	std::vector<integer> indices;

public:
	DirCColMatrixHandler(std::vector<doublereal>& x,
			const std::vector<integer>& i,
			const std::vector<integer>& p);

	virtual ~DirCColMatrixHandler();

	/* used by MultiThreadDataManager to duplicate the storage array
	 * while preserving the CC indices */
	CompactSparseMatrixHandler *Copy(void) const;

public:
	doublereal & operator()(integer i_row, integer i_col) {
		ASSERTMSGBREAK(i_row > 0 && i_row <= NRows,
				"Error in CColMatrixHandler::operator(), "
				"row index out of range");
		ASSERTMSGBREAK(i_col > 0 && i_col <= NCols,
				"Error in CColMatrixHandler::operator(), "
				"col index out of range");
		integer idx = pindices[i_col][i_row];
		if (idx == -1) {
			/* matrix must be rebuilt */
			throw ErrRebuildMatrix();
		} else  {
			return Ax[idx];
		}
	};

	const doublereal& operator () (integer i_row, integer i_col) const {
		ASSERTMSGBREAK(ix > 0 && ix <= NRows,
				"Error in CColMatrixHandler::operator(), "
				"row index out of range");
		ASSERTMSGBREAK(iy > 0 && iy <= NCols,
				"Error in CColMatrixHandler::operator(), "
				"col index out of range");

		integer idx = pindices[i_col][i_row];
		if (idx == -1) {
			/* matrix must be rebuilt */
			return ::dZero;

		} else  {
			return Ax[idx];
		}
	};

	void Resize(integer n, integer nn);

	/* Estrae una colonna da una matrice */
	VectorHandler& GetCol(integer icol, VectorHandler& out) const;
	
	/* Matrix Matrix product */
protected:
	MatrixHandler*
	MatMatMul_base(void (MatrixHandler::*op)(integer iRow, integer iCol,
				const doublereal& dCoef),
			MatrixHandler* out, const MatrixHandler& in) const;
	MatrixHandler*
	MatTMatMul_base(void (MatrixHandler::*op)(integer iRow, integer iCol,
				const doublereal& dCoef),
			MatrixHandler* out, const MatrixHandler& in) const;
public:

        /* Moltiplica per uno scalare e somma a una matrice */
	MatrixHandler& MulAndSumWithShift(MatrixHandler& out,
			doublereal s = 1.,
			integer drow = 0, integer dcol = 0) const;
	
	MatrixHandler& FakeThirdOrderMulAndSumWithShift(MatrixHandler& out, 
		std::vector<bool> b, doublereal s = 1.,
		integer drow = 0, integer dcol = 0) const;
	
	/* Matrix Vector product */
protected:
	virtual VectorHandler&
	MatVecMul_base(void (VectorHandler::*op)(integer iRow,
				const doublereal& dCoef),
			VectorHandler& out, const VectorHandler& in) const;
	virtual VectorHandler&
	MatTVecMul_base(void (VectorHandler::*op)(integer iRow,
				const doublereal& dCoef),
			VectorHandler& out, const VectorHandler& in) const;

public:
};


#endif /* DirCColMatrixHandler_hh */

