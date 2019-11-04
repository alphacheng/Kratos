//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Aditya Ghantasala

#if !defined(DISTANCE_CALCULATION_UTILITY )
#define  DISTANCE_CALCULATION_UTILITY



// System includes


// External includes
#include "omp.h"


// Project includes
#include "includes/define.h"
#include "processes/variational_distance_calculation_process.h"
#include "utilities/parallel_levelset_distance_calculator.h"
#include "processes/calculate_signed_distance_to_3d_condition_skin_process.h"
#include "mpi/utilities/gather_modelpart_utility.h"

namespace Kratos
{

///@name Kratos Classes
///@{

/// Utility for calculating the Distance on a given modelpart
template <int TDim, class TSparseSpaceType, class TLocalSpaceType>
class KRATOS_API(CHIMERA_APPLICATION) DistanceCalculationUtility
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of DistanceCalculationUtility
    KRATOS_CLASS_POINTER_DEFINITION(DistanceCalculationUtility);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    DistanceCalculationUtility() = delete;

    /// Destructor.
    /// Deleted copy constructor
    DistanceCalculationUtility(const DistanceCalculationUtility& rOther) = delete;


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Calculates distance on the whole of rBackgroundModelPart from rSkinModelPart
     * @param rBackgroundModelPart The background modelpart where distances are calculated.
     * @param rSkinModelPart The skin modelpart from where the distances are calculated
     */
    static inline void CalculateDistance(ModelPart &rBackgroundModelPart, ModelPart &rSkinModelPart)
    {
        //typedef LinearSolverFactory<SparseSpaceType, LocalSparseSpaceType> LinearSolverFactoryType;
        //typedef LinearSolver<SparseSpaceType, TLocalSpaceType> LinearSolverType;
        //typedef VariationalDistanceCalculationProcess<TDim, TSparseSpaceType, TLocalSpaceType, LinearSolverType> VariationalDistanceCalculationProcessType;
        typedef CalculateDistanceToSkinProcess<TDim> CalculateDistanceToSkinProcessType;
        const int nnodes = static_cast<int>(rBackgroundModelPart.NumberOfNodes());

#pragma omp parallel for
        for (int i_node = 0; i_node < nnodes; ++i_node)
        {
            auto it_node = rBackgroundModelPart.NodesBegin() + i_node;
            it_node->FastGetSolutionStepValue(DISTANCE, 0) = 0.0;
            it_node->FastGetSolutionStepValue(DISTANCE, 1) = 0.0;
            it_node->SetValue(DISTANCE, 0.0);
        }

        const DataCommunicator& r_comm =
            rBackgroundModelPart.GetCommunicator().GetDataCommunicator();
        const int mpi_size = r_comm.Size();
        Model &current_model = rBackgroundModelPart.GetModel();
        ModelPart &r_gathered_skin_mp = r_comm.IsDistributed() ? current_model.CreateModelPart("GatheredSkin") : rSkinModelPart;

        // If it is distributed, gather on the rank 0 to do the distance calculation
        if (r_comm.IsDistributed())
        {
            // After this, all the ranks have complete skin mp.
            for (int rank=0; rank<mpi_size; ++rank)
                GatherModelPartUtility(rank, rSkinModelPart, 0, r_gathered_skin_mp);
        }

        // This distance computation is always local to each rank.
        // Now the bg modelpart has the distances from the skin.
        CalculateDistanceToSkinProcessType(rBackgroundModelPart, r_gathered_skin_mp).Execute();


        // Parameters amgcl_settings(R"(
        //     {
        //         "solver_type"                   : "amgcl",
        //         "tolerance"                     : 0.001,
        //         "max_iteration"                 : 200,
        //         "krylov_type"                   : "gmres",
        //         "smoother_type"                 : "ilu0",
        //         "verbosity"                     : 0
        //     }
        //     )");
        // const int max_iterations = 1;
        // LinearSolverFactoryType const &linear_solver_factory = KratosComponents<LinearSolverFactoryType>::Get("amgcl");
        // auto amgcl_solver = linear_solver_factory.Create(amgcl_settings);
        // VariationalDistanceCalculationProcessType(rBackgroundModelPart, amgcl_solver, max_iterations, VariationalDistanceCalculationProcessType::CALCULATE_EXACT_DISTANCES_TO_PLANE).Execute();

        unsigned int max_level = 100;
		double max_distance = 200;
        auto p_distance_smoother = Kratos::make_shared<ParallelDistanceCalculator<TDim>>();
        p_distance_smoother->CalculateDistances(rBackgroundModelPart, DISTANCE, NODAL_AREA, max_level, max_distance);
    }

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    ///@}
    ///@name Friends
    ///@{


    ///@}


private:
    ///@name Static Member Variables
    ///@{




    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class DistanceCalculationUtility

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

///@}


}  // namespace Kratos.

#endif // DISTANCE_CALCULATION_UTILITY  defined


