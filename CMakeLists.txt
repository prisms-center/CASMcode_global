cmake_minimum_required(VERSION 3.20)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

project(CASMcode_global VERSION 2.2.0 LANGUAGES CXX)

# set CMAKE_INSTALL_X variables
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# try to use ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

##############################################
## Find dependencies

# Should find ZLIB::ZLIB
find_package(ZLIB)

# if no user CMAKE_INSTALL_PREFIX, use CASM_PREFIX if it exists
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(DEFINED CASM_PREFIX)
    message(STATUS "CMAKE_INSTALL_PREFIX initialized to default, so updating CMAKE_INSTALL_PREFIX to CASM_PREFIX")
    set(CMAKE_INSTALL_PREFIX ${CASM_PREFIX} CACHE PATH "set CMAKE_INSTALL_PREFIX to CASM_PREFIX" FORCE)
    message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
  endif()
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


##############################################
## Build libcasm_global

# create libcasm_global
set(
  libcasm_global_HEADERS
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/FileData.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/FormatFlag.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/Help.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/Log.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/SafeOfstream.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/container/json_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/container/stream_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatterDecl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatterFilter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatterFilter_impl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatterTools.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatterTools_impl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataFormatter_impl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DataStream.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/DatumFormatterAdapter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/EigenDataStream.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/FormattedDataFile.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/dataformatter/FormattedDataFile_impl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/enum/io_traits.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/enum/json_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/enum/stream_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/json/InputParser.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/json/InputParser_impl.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/json/jsonFile.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/json/jsonParser.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/json/optional.hh
  ${PROJECT_SOURCE_DIR}/include/casm/casm_io/string_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/Array.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/BaseCounter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/ContainerTraits.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/Counter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/IsoCounter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/MultiCounter.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/Permutation.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/PolyTrie.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/Template_Algorithms.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/Tensor.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/algorithm.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/io/PermutationIO.hh
  ${PROJECT_SOURCE_DIR}/include/casm/container/multivector.hh
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Cholesky
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/CholmodSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Core
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Dense
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Eigen
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Eigenvalues
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Geometry
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Householder
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/IterativeLinearSolvers
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Jacobi
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/KLUSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/LU
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/MetisSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/OrderingMethods
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/PaStiXSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/PardisoSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/QR
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/QtAlignedMalloc
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SPQRSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SVD
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/Sparse
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SparseCholesky
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SparseCore
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SparseLU
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SparseQR
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/StdDeque
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/StdList
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/StdVector
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/SuperLUSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/UmfPackSupport
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Cholesky/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Cholesky/LDLT.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Cholesky/LLT.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Cholesky/LLT_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/CholmodSupport/CholmodSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/CholmodSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ArithmeticSequence.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Array.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ArrayBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ArrayWrapper.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Assign.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/AssignEvaluator.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Assign_MKL.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/BandMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Block.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/BooleanRedux.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CommaInitializer.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ConditionEstimator.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CoreEvaluators.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CoreIterators.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CwiseBinaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CwiseNullaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CwiseTernaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CwiseUnaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/CwiseUnaryView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/DenseBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/DenseCoeffsBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/DenseStorage.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Diagonal.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/DiagonalMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/DiagonalProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Dot.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/EigenBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ForceAlignedAccess.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Fuzzy.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/GeneralProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/GenericPacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/GlobalFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/IO.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/IndexedView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Inverse.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Map.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/MapBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/MathFunctionsImpl.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Matrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/MatrixBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/NestByValue.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/NoAlias.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/NumTraits.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/PartialReduxEvaluator.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/PermutationMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/PlainObjectBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Product.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ProductEvaluators.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Random.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Redux.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Ref.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Replicate.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Reshaped.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/ReturnByValue.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Reverse.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Select.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/SelfAdjointView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/SelfCwiseBinaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Solve.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/SolveTriangular.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/SolverBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/StableNorm.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/StlIterators.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Stride.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Swap.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Transpose.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Transpositions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/TriangularMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/VectorBlock.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/VectorwiseOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/Visitor.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX512/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX512/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX512/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AVX512/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/MatrixProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/MatrixProductCommon.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/MatrixProductMMA.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/AltiVec/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/CUDA/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/BFloat16.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/ConjHelper.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/GenericPacketMathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/GenericPacketMathFunctionsFwd.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/Half.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/Settings.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/Default/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/GPU/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/GPU/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/GPU/Tuple.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/GPU/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/HIP/hcc/math_constants.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/MSA/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/MSA/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/MSA/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/NEON/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/NEON/GeneralBlockPanelKernel.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/NEON/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/NEON/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/NEON/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SSE/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SSE/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SSE/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SSE/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SVE/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SVE/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SVE/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SYCL/InteropHeaders.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SYCL/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SYCL/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SYCL/SyclMemoryModel.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/SYCL/TypeCasting.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/ZVector/Complex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/ZVector/MathFunctions.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/arch/ZVector/PacketMath.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/AssignmentFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/BinaryFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/NullaryFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/StlFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/TernaryFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/functors/UnaryFunctors.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralBlockPanelKernel.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixMatrixTriangular.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixMatrixTriangular_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixMatrix_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/GeneralMatrixVector_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/Parallelizer.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointMatrixMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointMatrixMatrix_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointMatrixVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointMatrixVector_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/SelfadjointRank2Update.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularMatrixMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularMatrixMatrix_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularMatrixVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularMatrixVector_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularSolverMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularSolverMatrix_BLAS.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/products/TriangularSolverVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/BlasUtil.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/ConfigureVectorization.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/Constants.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/DisableStupidWarnings.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/ForwardDeclarations.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/IndexedViewHelper.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/IntegralConstant.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/MKL_support.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/Macros.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/Memory.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/Meta.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/NonMPL2.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/ReenableStupidWarnings.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/ReshapedHelper.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/Serializer.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/StaticAssert.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/SymbolicIndex.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Core/util/XprHelper.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/ComplexEigenSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/ComplexSchur.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/ComplexSchur_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/EigenSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/GeneralizedEigenSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/GeneralizedSelfAdjointEigenSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/HessenbergDecomposition.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/MatrixBaseEigenvalues.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/RealQZ.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/RealSchur.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/RealSchur_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/SelfAdjointEigenSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/SelfAdjointEigenSolver_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Eigenvalues/Tridiagonalization.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/AlignedBox.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/AngleAxis.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/EulerAngles.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Homogeneous.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Hyperplane.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/OrthoMethods.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/ParametrizedLine.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Quaternion.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Rotation2D.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/RotationBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Scaling.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Transform.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Translation.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/Umeyama.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Geometry/arch/Geometry_SIMD.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Householder/BlockHouseholder.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Householder/Householder.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Householder/HouseholderSequence.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Householder/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/BasicPreconditioners.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/BiCGSTAB.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/ConjugateGradient.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/IncompleteCholesky.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/IncompleteLUT.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/IterativeSolverBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/LeastSquareConjugateGradient.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/IterativeLinearSolvers/SolveWithGuess.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Jacobi/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/Jacobi/Jacobi.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/KLUSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/KLUSupport/KLUSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/Determinant.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/FullPivLU.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/InverseImpl.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/PartialPivLU.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/PartialPivLU_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/LU/arch/InverseSize4.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/MetisSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/MetisSupport/MetisSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/OrderingMethods/Amd.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/OrderingMethods/Eigen_Colamd.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/OrderingMethods/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/OrderingMethods/Ordering.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/PaStiXSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/PaStiXSupport/PaStiXSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/PardisoSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/PardisoSupport/PardisoSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/ColPivHouseholderQR.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/ColPivHouseholderQR_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/CompleteOrthogonalDecomposition.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/FullPivHouseholderQR.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/HouseholderQR.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/HouseholderQR_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/QR/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SPQRSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SPQRSupport/SuiteSparseQRSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/BDCSVD.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/JacobiSVD.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/JacobiSVD_LAPACKE.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/SVDBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SVD/UpperBidiagonalization.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCholesky/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCholesky/SimplicialCholesky.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCholesky/SimplicialCholesky_impl.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/AmbiVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/CompressedStorage.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/ConservativeSparseSparseProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/MappedSparseMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseAssign.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseBlock.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseColEtree.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseCompressedBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseCwiseBinaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseCwiseUnaryOp.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseDenseProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseDiagonalProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseDot.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseFuzzy.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseMap.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseMatrixBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparsePermutation.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseRedux.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseRef.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseSelfAdjointView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseSolverBase.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseSparseProductWithPruning.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseTranspose.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseTriangularView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseUtil.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/SparseView.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseCore/TriangularSolver.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLUImpl.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_Memory.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_Structs.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_SupernodalMatrix.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_Utils.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_column_bmod.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_column_dfs.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_copy_to_ucol.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_gemm_kernel.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_heap_relax_snode.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_kernel_bmod.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_panel_bmod.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_panel_dfs.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_pivotL.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_pruneL.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseLU/SparseLU_relax_snode.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseQR/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SparseQR/SparseQR.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/StlSupport/StdDeque.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/StlSupport/StdList.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/StlSupport/StdVector.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/StlSupport/details.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SuperLUSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/SuperLUSupport/SuperLUSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/UmfPackSupport/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/UmfPackSupport/UmfPackSupport.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/Image.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/Kernel.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/RealSvd2x2.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/blas.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/lapack.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/lapacke.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/misc/lapacke_mangling.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/ArrayCwiseBinaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/ArrayCwiseUnaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/BlockMethods.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/CommonCwiseBinaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/CommonCwiseUnaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/IndexedViewMethods.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/InternalHeaderCheck.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/MatrixCwiseBinaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/MatrixCwiseUnaryOps.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/Eigen/src/plugins/ReshapedMethods.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/MersenneTwister/MersenneTwister.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/gzstream/gzstream.h
  ${PROJECT_SOURCE_DIR}/include/casm/external/nlohmann/json.hpp
  ${PROJECT_SOURCE_DIR}/include/casm/external/pybind11_json/pybind11_json.hpp
  ${PROJECT_SOURCE_DIR}/include/casm/global/definitions.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/eigen.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/enum.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/enum/io_traits.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/enum/json_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/enum/stream_io.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/errors.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/filesystem.hh
  ${PROJECT_SOURCE_DIR}/include/casm/global/version.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/CASM_Array_math.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/CASM_Eigen_math.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/CASM_TMP.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/CASM_math.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/CRTPBase.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/Comparisons.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/HallOfFame.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/HierarchyID.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/KroneckerTensorProduct.h
  ${PROJECT_SOURCE_DIR}/include/casm/misc/PCA.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/ParsingDictionary.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/TypeInfo.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/UnaryCompare.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/Validator.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/algorithm.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/cloneable_ptr.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/string_algorithm.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/type_traits.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/unique_cloneable_map.hh
  ${PROJECT_SOURCE_DIR}/include/casm/misc/unique_map.hh
  ${PROJECT_SOURCE_DIR}/include/casm/system/Popen.hh
  ${PROJECT_SOURCE_DIR}/include/casm/system/RuntimeLibrary.hh
)
set(
  libcasm_global_SOURCES
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/FileData.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/FormatFlag.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/Log.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/dataformatter/DataFormatter.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/dataformatter/DataStream.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/dataformatter/FormattedDataFile.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/json/InputParser.cc
  ${PROJECT_SOURCE_DIR}/src/casm/casm_io/json/jsonParser.cc
  ${PROJECT_SOURCE_DIR}/src/casm/container/Permutation.cc
  ${PROJECT_SOURCE_DIR}/src/casm/container/io/PermutationIO.cc
  ${PROJECT_SOURCE_DIR}/src/casm/external/gzstream/gzstream.C
  ${PROJECT_SOURCE_DIR}/src/casm/global/enum/io_traits.cc
  ${PROJECT_SOURCE_DIR}/src/casm/global/enum/json_io.cc
  ${PROJECT_SOURCE_DIR}/src/casm/global/enum/stream_io.cc
  ${PROJECT_SOURCE_DIR}/src/casm/global/version.cc
  ${PROJECT_SOURCE_DIR}/src/casm/misc/CASM_Array_math.cc
  ${PROJECT_SOURCE_DIR}/src/casm/misc/CASM_Eigen_math.cc
  ${PROJECT_SOURCE_DIR}/src/casm/misc/CASM_math.cc
  ${PROJECT_SOURCE_DIR}/src/casm/system/Popen.cc
  ${PROJECT_SOURCE_DIR}/src/casm/system/RuntimeLibrary.cc
)
add_library(casm_global SHARED ${libcasm_global_SOURCES})
target_include_directories(casm_global
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/casm/external>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/casm/external/gzstream>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/casm/external>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/casm/external/gzstream>
)
target_compile_options(casm_global
  PUBLIC
    "-DTXT_VERSION=\"${CMAKE_PROJECT_VERSION}\""
    -DEIGEN_DEFAULT_DENSE_INDEX_TYPE=long
    -DGZSTREAM_NAMESPACE=gz
)
target_link_libraries(casm_global
  ZLIB::ZLIB
  ${CMAKE_DL_LIBS}
)


##############################################
## Install libcasm_global

# install header files in <prefix>/libcasm/include/,
# while preserving directory structure
foreach ( filevar ${libcasm_global_HEADERS} )
  file(RELATIVE_PATH relfile ${PROJECT_SOURCE_DIR}/include/ ${filevar})
  get_filename_component( reldir ${relfile} DIRECTORY )
  install( FILES ${filevar} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${reldir} )
endforeach()

# install libcasm_global in <prefix>/libcasm/lib/
install(
  TARGETS casm_global
  EXPORT CASMcode_globalTargets
  DESTINATION lib)

##############################################
## Python extensions

# The CMake package config and target files are installed under the Python
# package root. This is necessary to ensure that all the relative paths in the
# helloTargets.cmake resolve correctly. It also provides encapsulation.
#
# The actual path used must be selected so that consuming projects can locate it
# via `find_package`. To support finding CMake packages in the Python package
# prefix, using `find_package`s default search path of
# `<prefix>/<name>/share/<name>*/cmake/` is reasonable. Adding the Python
# package installation prefix to CMAKE_PREFIX_PATH in combination with this path
# will allow `find_package` to find this package and any other package installed
# via a Python package if the CMake and Python packages are named the same.
set(CASM_GLOBAL_CMAKE_PACKAGE_INSTALL_SUBDIR "share/CASMcode_global/cmake")

# configure and install libcasm.casmglobal.__main__.py
# this allows getting configuration information of the installed package using:
# - python -m libcasm.casmglobal --cmakelists
# - python -m libcasm.casmglobal --prefix
configure_file("${PROJECT_SOURCE_DIR}/python/libcasm/casmglobal/__main__.py.in"
  "${PROJECT_BINARY_DIR}/python/libcasm/casmglobal/__main__.py")
install(
  FILES "${PROJECT_BINARY_DIR}/python/libcasm/casmglobal/__main__.py"
  DESTINATION casmglobal)

install(
  EXPORT CASMcode_globalTargets
  NAMESPACE CASM::
  DESTINATION ${CASM_GLOBAL_CMAKE_PACKAGE_INSTALL_SUBDIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  CASMcode_globalConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/CASMcode_globalConfig.cmake.in" CASMcode_globalConfig.cmake
  INSTALL_DESTINATION ${CASM_GLOBAL_CMAKE_PACKAGE_INSTALL_SUBDIR})

install(FILES "${PROJECT_BINARY_DIR}/CASMcode_globalConfig.cmake"
              "${PROJECT_BINARY_DIR}/CASMcode_globalConfigVersion.cmake"
        DESTINATION ${CASM_GLOBAL_CMAKE_PACKAGE_INSTALL_SUBDIR})

# We are using the SKBUILD variable, which is defined when scikit-build is
# running the CMake build, to control building the Python wrapper. This allows
# the C++ project to be installed, standalone, when using the standard CMake
# build flow.
if(DEFINED SKBUILD)

  # call pybind11-config to obtain the root of the cmake package
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pybind11 --cmakedir
                  OUTPUT_VARIABLE pybind11_ROOT_RAW)
  string(STRIP ${pybind11_ROOT_RAW} pybind11_ROOT)
  find_package(pybind11)

  # The extension modules must load the casm_global library as a dependency when the
  # extension module is loaded. The easiest way to locate the casm_global library is
  # via RPATH. Absolute RPATHs are possible, but they make the resulting
  # binaries not redistributable to other Python installations (conda is broke,
  # wheel reuse is broke, and more!).
  #
  # Placing the casm_global library in the package and using relative RPATHs that
  # doesn't point outside of the package means that the built package is
  # relocatable. This allows for safe binary redistribution.

  ### libcasm.casmglobal._casmglobal ###
  pybind11_add_module(_casmglobal MODULE
                      "${PROJECT_SOURCE_DIR}/python/src/casmglobal.cpp")
  target_link_libraries(_casmglobal PRIVATE casm_global)
  install(TARGETS _casmglobal DESTINATION casmglobal)
  if(APPLE)
    set_target_properties(
      _casmglobal PROPERTIES INSTALL_RPATH "@loader_path/../lib")
  else()
    set_target_properties(
      _casmglobal PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
  endif()

  ### libcasm.counter._counter ###
  pybind11_add_module(_counter MODULE
                      "${PROJECT_SOURCE_DIR}/python/src/counter.cpp")
  target_link_libraries(_counter PRIVATE casm_global)
  install(TARGETS _counter DESTINATION counter)
  if(APPLE)
    set_target_properties(
      _counter PROPERTIES INSTALL_RPATH "@loader_path/../lib")
  else()
    set_target_properties(
      _counter PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
  endif()

endif()
