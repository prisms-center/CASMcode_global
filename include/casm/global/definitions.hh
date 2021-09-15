#ifndef CASM_global_definitions
#define CASM_global_definitions

#ifdef EIGEN_DEFAULT_DENSE_INDEX_TYPE
#define INDEX_TYPE EIGEN_DEFAULT_DENSE_INDEX_TYPE
#else
#define INDEX_TYPE long
#endif

namespace CASM {

typedef unsigned int uint;
typedef unsigned long int ulint;
typedef long int lint;

// tolerance
const double TOL = 0.00001;

// Boltzmann Constant
const double KB = 8.6173423E-05;  // eV/K

// Planck's Constant
const double PLANCK = 4.135667516E-15;  // eV-s

/// For long integer indexing:
typedef INDEX_TYPE Index;
inline bool valid_index(Index i) { return 0 <= i; }

template <typename T>
struct traits;

}  // namespace CASM

#endif
