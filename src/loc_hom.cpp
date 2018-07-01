/*
 *  This source code is part of MicroPP: a finite element library
 *  to solve microstructural problems for composite materials.
 *
 *  Copyright (C) - 2018 - Guido Giuntoli <gagiuntoli@gmail.com>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "micro.h"

void Problem::calcCtanLinear()
{
	double sig_1[6], eps_1[6];
	double d_eps = 1.0e-8;
  	bool non_linear;

  	for (int i=0; i<nvoi; i++)
  	{
    	for (int v=0; v<nvoi; v++)
      		eps_1[v] = 0.0;

    	eps_1[i] += d_eps;

    	setDisp(eps_1);
    	newtonRaphson(&non_linear);
    	calcAverageStress(sig_1);

    	for (int v=0; v<nvoi; v++)
      		ctan_lin[v][i] = sig_1[v] / d_eps;
  	}
}

bool Problem::LinearCriteria (const double *macro_strain)
{
  	double macro_stress[6];

  	for (int i=0; i<nvoi; i++) {
    	macro_stress[i] = 0.0;
    	for (int j=0; j<nvoi; j++) {
      		macro_stress[i] += ctan_lin[i][j] * macro_strain[j];
    	}
  	}
  	double Inv = Invariant_I1(macro_stress);

  	if (fabs(Inv) > I_reached) I_reached = fabs(Inv);

  	return (fabs(Inv) < I_max) ? true : false;
}

double Problem::Invariant_I1 (const double *tensor)
{
  	if (dim == 2)
    	return tensor[0] + tensor[1];
  	if (dim == 3)
    	return tensor[0] + tensor[1] + tensor[2];
}

double Problem::Invariant_I2 (const double *tensor)
{
  	if (dim == 3)
    	return \
    		tensor[0]*tensor[1] + tensor[0]*tensor[2] + tensor[1]*tensor[2] + \
    		tensor[3]*tensor[3] + tensor[4]*tensor[4] + tensor[5]*tensor[5];
}

void Problem::set_macro_strain(const int gp_id, const double *macro_strain)
{
  	list<GaussPoint_t>::iterator GaussPoint;
  	for (GaussPoint=GaussPointList.begin(); GaussPoint!=GaussPointList.end(); GaussPoint++) {
    	if (GaussPoint->id == gp_id) {
      		for (int i=0; i<nvoi; i++)
				GaussPoint->macro_strain[i] = macro_strain[i];
      		break;
    	}
  	}
	if (GaussPoint ==  GaussPointList.end()) {

    	GaussPoint_t GaussPointNew;

    	GaussPointNew.id = gp_id;
    	GaussPointNew.int_vars_n = NULL;
    	GaussPointNew.int_vars_k = NULL;
    	for (int i=0; i<nvoi; i++)
      		GaussPointNew.macro_strain[i] = macro_strain[i];

    	GaussPointNew.convergence.I_reached = -1.0e10;

    	GaussPointList.push_back(GaussPointNew);
  	}
}

void Problem::get_macro_stress(const int gp_id, double *macro_stress)
{
	for (auto const& gp : GaussPointList)
    	if (gp.id == gp_id) {
      		for (int i=0; i<nvoi; i++)
				macro_stress[i] = gp.macro_stress[i];
      		break;
    	}
}

void Problem::get_macro_ctan(const int gp_id, double *macro_ctan)
{
	for (auto const& gp : GaussPointList)
    	if (gp.id == gp_id) {
      		for (int i = 0; i < nvoi*nvoi; ++i)
				macro_ctan[i] = gp.macro_ctan[i];
      		break;
    	}
}

void Problem::localizeHomogenize()
{
	for (auto& gp : GaussPointList) {
    	if (gp.int_vars_n == NULL) {
      		for (int i = 0; i < num_int_vars; ++i)
				vars_old[i] = 0.0;
    	} else {
      		for (int i = 0; i < num_int_vars; ++i)
				vars_old[i] = gp.int_vars_n[i];
    	}

    	I_reached = gp.convergence.I_reached;

    	if ((LinearCriteria(gp.macro_strain) == true) && (gp.int_vars_n == NULL)) {

      		// S = CL : E
      		for (int i = 0; i < nvoi; ++i) {
				gp.macro_stress[i] = 0.0;
				for (int j = 0; j < nvoi; ++j)
	  				gp.macro_stress[i] += ctan_lin[i][j] * gp.macro_strain[j];
      		}
      		// C = CL
      		for (int i=0; i<nvoi; i++) {
				for (int j=0; j<nvoi; j++)
	  				gp.macro_ctan[i*nvoi + j] = ctan_lin[i][j];
      		}
      		gp.convergence.NR_Its_Stress = 0;
      		gp.convergence.NR_Err_Stress = 0.0;
      		for (int i=0; i<nvoi; i++) {
				gp.convergence.NR_Its_Ctan[i] = 0;
				gp.convergence.NR_Err_Ctan[i] = 0.0;
      		}

    	} else {

      		// HOMOGENIZE SIGMA (FEM)
      		bool non_linear;
      		setDisp(gp.macro_strain);
      		newtonRaphson(&non_linear); 
      		calcAverageStress(gp.macro_stress);

    		if (non_linear == true)
    		{
      			if(gp.int_vars_k == NULL) {
					gp.int_vars_k = (double *)malloc(num_int_vars*sizeof(double));
					gp.int_vars_n = (double *)malloc(num_int_vars*sizeof(double));
				}
      			for (int i=0; i<num_int_vars; i++)
					gp.int_vars_k[i] = vars_new[i];
			}

      		gp.convergence.NR_Its_Stress = NR_its;
      		gp.convergence.NR_Err_Stress = NR_norm;

      		// HOMOGENIZE CTAN (FEM)
      		double eps_1[6], sig_0[6], sig_1[6], dEps = 1.0e-10;
      		setDisp(gp.macro_strain);
      		newtonRaphson(&non_linear);
      		calcAverageStress(sig_0);

      		for (int i=0; i<nvoi; i++) {
				for (int v=0; v<nvoi; v++)
	  				eps_1[v] = gp.macro_strain[v];
				eps_1[i] += dEps;

				setDisp(eps_1);
				newtonRaphson(&non_linear);
				calcAverageStress(sig_1);
				for (int v=0; v<nvoi; v++)
	  				gp.macro_ctan[v*nvoi + i] = (sig_1[v] - sig_0[v]) / dEps;

				gp.convergence.NR_Its_Ctan[i] = NR_its;
				gp.convergence.NR_Err_Ctan[i] = NR_norm;
      		}
    	}
    	gp.convergence.I_reached_aux = I_reached;
  	}
}

void Problem::updateInternalVariables()
{
	list<GaussPoint_t>::iterator GaussPoint;
  	for (GaussPoint=GaussPointList.begin(); GaussPoint!=GaussPointList.end(); GaussPoint++) {
      	if(GaussPoint->int_vars_n != NULL) {
      		for (int i=0; i<num_int_vars; i++)
				GaussPoint->int_vars_n[i] = GaussPoint->int_vars_k[i];
    	}
  	}
}
