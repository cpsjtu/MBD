/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2008
 *
 * Marco Morandini	<morandini@aero.polimi.it>
 * Pierangelo Masarati	<masarati@aero.polimi.it>
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
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>


#define _GNU_SOURCE 1
#include <fenv.h>
static void __attribute__ ((constructor))
trapfpe ()
{
  /* Enable some exceptions.  At startup all exceptions are masked.  */

  feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
}

#include "nlrheo_damper.h"

extern "C" int
nlrheo_int_func(double t, const double y[], double f[], void *para);

std::string file_name_dati_simulazione;
std::string file_name_variabili_ottimizzazione;
std::ifstream fin;

int
nlrheo_get_int(int *i)
{
	fin >> *i;
	return 0;
}

int
nlrheo_get_real(double *d)
{
	fin >> *d;
	if (fin.eof()) {
		fin.close();
		fin.clear();
		fin.open(file_name_variabili_ottimizzazione.c_str());
		if (!fin) {
			std::cerr << "unable to open file "
				<< file_name_variabili_ottimizzazione
				<< std::endl;
			throw;
		}
		fin >> *d;
		if (fin.eof()) {
			throw;
		}
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	bool homemade = true;

	switch (argc) {
	case 4:
		homemade = false;
		// fallthru

	case 3:
		file_name_dati_simulazione = argv[1];
		file_name_variabili_ottimizzazione = argv[2];
		break;

	default:
		std::cerr << "usage: nlrheo_main <data> <vars> [1]"
			<< std::endl;
		exit(EXIT_FAILURE);
	}

	::fin.open(file_name_dati_simulazione.c_str());
	if (!::fin) {
		std::cerr << "unable to open file "
			<< file_name_dati_simulazione << std::endl;
		throw;
	}

	sym_params pa = { 0 };

	pa.scale_eps = 1.;
	pa.scale_f = 1.;

	::fin >> pa.hi_freq_force_filter_coeff; 
	pa.hi_freq_force_filter_coeff = 1./pa.hi_freq_force_filter_coeff;
	::fin >> pa.low_freq_displ_filter_coeff;
	::fin >> pa.static_low_freq_stiffness;

	::fin >> pa.nsubsteps;

	sym_params *pap = 0;
	if (nlrheo_parse(&pap,
		pa.scale_eps, pa.scale_f,
		pa.hi_freq_force_filter_coeff,
		pa.low_freq_displ_filter_coeff,
		pa.static_low_freq_stiffness,
		pa.nsubsteps, 1.e-4))
	{
		return -1;
	}

	pa = *pap;
	nlrheo_init(&pa);

	double final_time = 20.;
	double delta_t = 1.e-3;

	if (!homemade) {
		nlrheo_update(&pa, 0., 0., 1., 0);
	}

	for (double t = 0.; t < final_time; ) {
		pa.tf = t + delta_t;
		if (pa.tf < 5.) {
			pa.sf = 1. * pa.tf;
			pa.vf = 1.;
		} else if (pa.tf < 10.) {
			pa.sf = 5. - 1. * (pa.tf - 5.);
			pa.vf = -1.;
		} else {
			pa.sf = 0.;
			pa.vf = 0.;
		}

		int rc;

		if (homemade) {
			pa.ti = t;
			if (pa.ti < 5.) {
				pa.si = 1. * pa.ti;
				pa.vi = 1.;
			} else if (pa.ti < 10.) {
				pa.si = 5. - 1. * (pa.ti - 5.);
				pa.vi = -1.;
			} else {
				pa.si = 0.;
				pa.vi = 0.;
			}

			pa.dt = delta_t / pa.nsubsteps;

			while (t < pa.tf) {
				rc = gsl_odeiv_evolve_apply(pa.evolve,
					pa.control, pa.stepint,
					&pa.sys, &t, pa.tf, &pa.dt, pa.y);
				pa.dt = std::max(pa.dt, pa.dtmin);
			}

		} else {
			rc = nlrheo_update(&pa, pa.tf, pa.sf, pa.vf, 0);
			t = pa.tf;
		}

		std::cout << t
			<< " " << pa.f
			<< " " << pa.sf
			<< " " << pa.vf
			<< " " << rc
			<< std::endl;
	}
   
	return 0;
}
