#include <vector>
#include <iostream>
#include <list>
#include "ell.h"

#define MAX_MAT_PARAM 10
#define MAX_MATS      10
#define MAX_GP_VARS   10
#define INT_VARS_GP   7  // eps_p_1, alpha_1
#define VARS_AT_GP    7  // eps_p_1, alpha_1

#define glo_elem3D(ex,ey,ez) ((ez)*(nx-1)*(ny-1) + (ey)*(nx-1) + (ex))
#define intvar_ix(e,gp,var) ((e)*8*INT_VARS_GP + (gp)*INT_VARS_GP + (var))

using namespace std;

struct MacroGp_t {
  int id;
  double *int_vars;
};

struct material_t {
  // normal
  double E;
  double nu;
  // plasticity
  double k;
  double mu;
  double lambda;
  double K_alpha; 
  double Sy;
  // flags
  bool plasticity;
  bool damage;
};

class Problem {

  public:

    int dim, npe, nvoi;
    int nx, ny, nz, nn; 
    double lx, ly, lz, dx, dy, dz;
    int nelem;
    int size_tot;

    int micro_type;
    double micro_params[5];
    int numMaterials;
    material_t material_list[MAX_MATS];
    std::list<MacroGp_t> MacroGp_list;

    double *elem_strain; // average strain on each element
    double *elem_stress; // average stress on each element
    int *elem_type; // number that is changes depending on the element type
    double *vars_old, *vars_new; 

    ell_matrix A;
    double *u, *du, *b;

    Problem (int dim, int size[3], int micro_type, double *micro_params, int *mat_types, double *params);
    ~Problem (void);

    void loc_hom_Stress (int macro_id, double *MacroStrain, double *MacroStress);
    void loc_hom_Ctan (int macroGp_id, double *MacroStrain, double *MacroCtan);

    void solve (void);
    void newtonRaphson (bool *non_linear_flag);

    void setDisp (double *eps);

    void Assembly_A (void);
    double Assembly_b (bool *non_linear_flag);

    void getElemental_A (int ex, int ey, double (&Ae)[2*4*2*4]);
    void getElemental_A (int ex, int ey, int ez, double (&Ae)[3*8*3*8]);
    void getCtanPlasSecant (int ex, int ey, int ez, int gp, double ctan[6][6]);
    void getCtanPlasExact (int ex, int ey, int ez, int gp, double ctan[6][6]);
    void getCtanPlasPert (int ex, int ey, int ez, int gp, double ctan[6][6]);

    void getElemental_b (int ex, int ey, bool *non_linear_flag, double (&be)[2*4]);
    void getElemental_b (int ex, int ey, int ez, bool *non_linear_flag, double (&be)[3*8]);

    void getStrain (int ex, int ey, int gp, double *strain_gp);
    void getStrain (int ex, int ey, int ez, int gp, double *strain_gp);

    void getStress (int ex, int ey, int gp, double strain_gp[3], bool *non_linear_flag, double *stress_gp);
    void getStress (int ex, int ey, int ez, int gp, double strain_gp[3], bool *non_linear_flag, double *stress_gp);
    void getDeviatoric (double tensor[6], double tensor_dev[6]);

    void getElemDisp (int ex, int ey, double *elem_disp);
    void getElemDisp (int ex, int ey, int ez, double *elem_disp);

    int getElemType (int ex, int ey);
    int getElemType (int ex, int ey, int ez);
    void getMaterial (int e, material_t &material);

    void calc_bmat_3D (int gp, double bmat[6][3*8]);

    void calcDistributions (void);
    void calcAverageStress (double stress_ave[6]);
    void calcAverageStrain (double strain_ave[6]);

    void writeVtu (int time_step, int elem);
    void output (int time_step, int elem, int macro_gp_global, double *MacroStrain);

};
