// 
// File:          MPQC_GaussianBasis_Shell_Impl.hh
// Symbol:        MPQC.GaussianBasis_Shell-v0.2
// Symbol Type:   class
// Babel Version: 0.10.2
// Description:   Server-side implementation for MPQC.GaussianBasis_Shell
// 
// WARNING: Automatically generated; only changes within splicers preserved
// 
// babel-version = 0.10.2
// 

#ifndef included_MPQC_GaussianBasis_Shell_Impl_hh
#define included_MPQC_GaussianBasis_Shell_Impl_hh

#ifndef included_sidl_cxx_hh
#include "sidl_cxx.hh"
#endif
#ifndef included_MPQC_GaussianBasis_Shell_IOR_h
#include "MPQC_GaussianBasis_Shell_IOR.h"
#endif
// 
// Includes for all method dependencies.
// 
#ifndef included_Chemistry_QC_GaussianBasis_AngularType_hh
#include "Chemistry_QC_GaussianBasis_AngularType.hh"
#endif
#ifndef included_MPQC_GaussianBasis_Shell_hh
#include "MPQC_GaussianBasis_Shell.hh"
#endif
#ifndef included_sidl_BaseException_hh
#include "sidl_BaseException.hh"
#endif
#ifndef included_sidl_BaseInterface_hh
#include "sidl_BaseInterface.hh"
#endif
#ifndef included_sidl_ClassInfo_hh
#include "sidl_ClassInfo.hh"
#endif


// DO-NOT-DELETE splicer.begin(MPQC.GaussianBasis_Shell._includes)
#include <chemistry/qc/basis/basis.h>
using namespace std;
using namespace Chemistry::QC::GaussianBasis;
using namespace sc;
// DO-NOT-DELETE splicer.end(MPQC.GaussianBasis_Shell._includes)

namespace MPQC { 

  /**
   * Symbol "MPQC.GaussianBasis_Shell" (version 0.2)
   */
  class GaussianBasis_Shell_impl
  // DO-NOT-DELETE splicer.begin(MPQC.GaussianBasis_Shell._inherits)

  /** GaussianBasis_Shell_impl implements a class interface for
      gaussian shell data.

      This is an implementation of a SIDL interface.
      The stub code is generated by the Babel tool.  Do not make
      modifications outside of splicer blocks, as these will be lost.
      This is a server implementation for a Babel class, the Babel
      client code is provided by the cca-chem-generic package.
   */

  // Put additional inheritance here...
  // DO-NOT-DELETE splicer.end(MPQC.GaussianBasis_Shell._inherits)
  {

  private:
    // Pointer back to IOR.
    // Use this to dispatch back through IOR vtable.
    GaussianBasis_Shell self;

    // DO-NOT-DELETE splicer.begin(MPQC.GaussianBasis_Shell._implementation)
   
    GaussianShell *shell_ptr_;
    Ref<GaussianShell> sc_shell_;
    AngularType angular_type_;
    int max_am_;
    
    // DO-NOT-DELETE splicer.end(MPQC.GaussianBasis_Shell._implementation)

  private:
    // private default constructor (required)
    GaussianBasis_Shell_impl() 
    {} 

  public:
    // sidl constructor (required)
    // Note: alternate Skel constructor doesn't call addref()
    // (fixes bug #275)
    GaussianBasis_Shell_impl( struct MPQC_GaussianBasis_Shell__object * s ) : 
      self(s,true) { _ctor(); }

    // user defined construction
    void _ctor();

    // virtual destructor (required)
    virtual ~GaussianBasis_Shell_impl() { _dtor(); }

    // user defined destruction
    void _dtor();

    // static class initializer
    static void _load();

  public:

    /**
     * user defined non-static method.
     */
    void
    initialize (
      /* in */ void* scshell
    )
    throw () 
    ;


    /**
     * Get the number of contractions in the shell. 
     * @return number of contractions 
     */
    int32_t
    get_n_contraction() throw ( 
      ::sidl::BaseException
    );

    /**
     * Get the number of primitives in the shell.
     * @return number of primitives 
     */
    int32_t
    get_n_primitive() throw ( 
      ::sidl::BaseException
    );

    /**
     * Get the coefficient for an unnormalized primitive 
     * in a contraction.
     * @param connum contraction number
     * @param expnum primitive number
     * @return contraction coefficient 
     */
    double
    get_contraction_coef (
      /* in */ int32_t connum,
      /* in */ int32_t expnum
    )
    throw ( 
      ::sidl::BaseException
    );


    /**
     * Get the exponent for a primitive.
     * @param expnum primitive id number
     * @return exponent 
     */
    double
    get_exponent (
      /* in */ int32_t expnum
    )
    throw ( 
      ::sidl::BaseException
    );


    /**
     * Get the angular momentum for a single contraction.
     * @param connum contraction id number
     * @return angular momentum value 
     */
    int32_t
    get_angular_momentum (
      /* in */ int32_t connum
    )
    throw ( 
      ::sidl::BaseException
    );


    /**
     * Get the max angular momentum, considering all contractions 
     * in the shell.
     * @return maximum angular momentum value 
     */
    int32_t
    get_max_angular_momentum() throw ( 
      ::sidl::BaseException
    );

    /**
     * Get the angular type for a single contraction.
     * @param connum contraction number
     * @return enum AngularType 
     */
    ::Chemistry::QC::GaussianBasis::AngularType
    get_contraction_angular_type (
      /* in */ int32_t connum
    )
    throw ( 
      ::sidl::BaseException
    );


    /**
     * Get the angular type.
     * @return enum AngularType 
     */
    ::Chemistry::QC::GaussianBasis::AngularType
    get_angular_type() throw ( 
      ::sidl::BaseException
    );

    /**
     * Print the shell data. 
     */
    void
    print_shell() throw ( 
      ::sidl::BaseException
    );
  };  // end class GaussianBasis_Shell_impl

} // end namespace MPQC

// DO-NOT-DELETE splicer.begin(MPQC.GaussianBasis_Shell._misc)
// Put miscellaneous things here...
// DO-NOT-DELETE splicer.end(MPQC.GaussianBasis_Shell._misc)

#endif
