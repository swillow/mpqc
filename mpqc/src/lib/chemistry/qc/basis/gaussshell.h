
#ifndef _chemistry_qc_basis_gaussshell_h
#define _chemistry_qc_basis_gaussshell_h

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <util/state/state.h>
#include <math/scmat/vector3.h>

class CartesianIter
{
  private:
    int a_;
    int c_;
    int l_;
    int bfn_;
  public:
    CartesianIter(int l);
    ~CartesianIter();
    void start();
    void next();
    operator int();
    int a();
    int b();
    int c();
    int l();
    int bfn();
};

class RefKeyVal;
class GaussianShell: public SavableState
{
#   define CLASSNAME GaussianShell
#   define HAVE_KEYVAL_CTOR
#   define HAVE_STATEIN_CTOR
#   include <util/state/stated.h>
#   include <util/class/classd.h>
  public:
    enum PrimitiveType { Normalized, Unnormalized };
    enum GaussianType { Cartesian, Pure };
  private:
    int nprim;
    int ncon;
    int nfunc;
    int* l;
    int* puream;
    double* exp;
    double** coef;  // contraction coefficients for unnormalized primitives

    double shell_normalization(int);
    void convert_coef();
    void normalize_shell();
    void compute_nfunc();
    PrimitiveType keyval_init(const RefKeyVal&,int,int);
    static const char* amtypes;
    static const char* AMTYPES;
  public:
    // Users of GaussianShell must pass pointers to newed memory that is kept
    // by GaussianShell and deleted by the destructor.
    // The arguments for the following ctor are:
    //   ncn is the number of contracted functions
    //     (1 except for SP and gen. con.)
    //   nprm is the number of primitives
    //   e gives the exponents (length nprm)
    //   am gives the angular momentum (length ncn)
    //   pure is 1 for pure am and 0 for cartesian (length ncn)
    //   c are the contraction coefficients (length ncn by nprm)
    //   pt describes whether the primitive functions are to be considered
    //     normalized or unnormalized.  this effects whether or not c is
    //     manipulated to give the correct normalization.
    GaussianShell(
                  int ncn,
                  int nprm,
                  double* e,
                  int* am,
                  int* pure,
                  double** c,
                  PrimitiveType pt = GaussianShell::Normalized);
    // In this ctor pure is either GaussianShell::Cartesian or
    // Gaussian::Pure and all of the contracted functions are
    // treated in that way. (The user doesn\'t need to compute
    // generate a int*pure vector in this case.)
    GaussianShell(
                  int ncn,
                  int nprm,
                  double* e,
                  int* am,
                  GaussianType pure,
                  double** c,
                  PrimitiveType pt = GaussianShell::Normalized);
    GaussianShell(const RefKeyVal&);
    GaussianShell(StateIn&);
    GaussianShell(const RefKeyVal&,int pure);
    ~GaussianShell();
    void save_data_state(StateOut&);
    int nprimitive() const;
    int ncontraction() const;
    int nfunction() const;
    int am(int con) const;
    int max_am() const;
    char amchar(int con) const;
    int nfunction(int con) const;
    int ncartesian(int con) const;
    int is_cartesian(int con) const;
    int is_pure(int con) const;
    // returns the con coef for unnormalized primitives
    double coefficient_unnorm(int con,int prim) const;
    // returns the con coef for normalized primitives
    double coefficient_norm(int con,int prim) const;
//     double normalization(int con,int bfn) const;
    double exponent(int iprim) const;

    // compute the value of this shell at offset r
    int values(const SCVector3& r, double* basis_values);
    int grad_values(const SCVector3& R,
                    double* g_values,
                    double* basis_values=0) const;

    void print(FILE*fp=stdout) const;
};

SavableState_REF_dec(GaussianShell);

#ifdef INLINE_FUNCTIONS
#include <chemistry/qc/basis/gaussshe_i.h>
#endif
  
#endif
