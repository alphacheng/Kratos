//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//

#include "add_trilinos_utilities_to_python.h"

// Trilinos includes
#include "Epetra_FEVector.h"
#include "Epetra_FECrsMatrix.h"
#include "Epetra_MpiComm.h"

// KratosCore dependencies
#include "includes/model_part.h"
#include "linear_solvers/linear_solver.h"
#include "solving_strategies/strategies/solving_strategy.h"
#include "spaces/ublas_space.h"

// TrilinosApplication dependencies
#include "trilinos_space.h"

// Chimera trilinos extensions
#include "custom_strategies/custom_builder_and_solvers/trilinos_chimera_block_builder_and_solver.h"
#include "custom_strategies/strategies/fs_strategy.h"
#include "custom_utilities/solver_settings.h"

namespace Kratos {
namespace Python {

void AddTrilinosStrategiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    typedef TrilinosSpace<Epetra_FECrsMatrix, Epetra_FEVector> TrilinosSparseSpaceType;
    typedef UblasSpace<double, Matrix, Vector> TrilinosLocalSpaceType;
    typedef LinearSolver<TrilinosSparseSpaceType, TrilinosLocalSpaceType > TrilinosLinearSolverType;
    typedef BuilderAndSolver< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType > TrilinosBaseBuilderAndSolverType;
    typedef TrilinosChimeraBlockBuilderAndSolver< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType > TrilinosChimeraResidualBasedBuilderAndSolverType;

    using TrilinosBaseSolvingStrategy = SolvingStrategy< TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType >;
    using BaseSolverSettings = SolverSettings<TrilinosSparseSpaceType, TrilinosLocalSpaceType, TrilinosLinearSolverType>;

    py::class_<
        TrilinosChimeraResidualBasedBuilderAndSolverType,
        typename TrilinosChimeraResidualBasedBuilderAndSolverType::Pointer,
        TrilinosBaseBuilderAndSolverType  >
    (m, "TrilinosChimeraResidualBasedBuilderAndSolver").def(py::init<Epetra_MpiComm&, int, TrilinosLinearSolverType::Pointer > () )
    ;

}
}
}
