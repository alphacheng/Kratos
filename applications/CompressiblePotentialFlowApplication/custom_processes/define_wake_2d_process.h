//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Aggelos Eznepidis
//

// #ifndef KRATOS_DEFINE_WAKE_2D_PROCESS_H
// #define KRATOS_DEFINE_WAKE_2D_PROCESS_H

#if !defined(KRATOS_DEFINE_WAKE_2D_PROCESS_H_INCLUDED )
#define KRATOS_DEFINE_WAKE_2D_PROCESS_H_INCLUDED

#include "includes/kratos_flags.h"
// #include "geometries/geometry.h"
// #include "utilities/geometry_utilities.h"
#include "compressible_potential_flow_application_variables.h"
// #include "utilities/math_utils.h"

// #include <boost/functional/hash.hpp> //TODO: remove this dependence when Kratos has en internal one
// #include <unordered_map> //TODO: remove this dependence when Kratos has en internal one
// #include <utility>

#include <string>
#include <iostream>
#include <sstream>

#include "includes/model_part.h"
#include "includes/kratos_parameters.h"
#include "processes/process.h"

namespace Kratos
{

class KRATOS_API(COMPRESSIBLE_POTENTIAL_FLOW_APPLICATION) DefineWake2DProcess: public Process
//class DefineWake2DProcess: public Process
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of Process
    KRATOS_CLASS_POINTER_DEFINITION(DefineWake2DProcess);

    typedef ModelPart::ElementType ElementType;
    typedef ModelPart::ConditionType ConditionType;
    ///@}
    ///@name Life Cycle
    ///@{

    // DefineWake2DProcess(
    //     ModelPart& rModelPart,
    //     Parameters Settings
    //     ) : Process(Flags()) ,
    //         mrModelPart(rModelPart),
    //         mSettings( Settings)
    // {
    //     KRATOS_TRY
    //     Parameters default_parameters( R"(
    //     {
    //         "model_part_name": "",
    //         "wake_direction": [1.0,0.0,0.0],
    //         "epsilon": 1e-9
    //     }  )" );

    //     Settings.ValidateAndAssignDefaults(default_parameters);
    //     KRATOS_CATCH("")
    // }

    DefineWake2DProcess(
        ModelPart& rBodyModelPart,
        ModelPart& rFluidModelPart
        ) : Process(Flags()) ,
            mrBodyModelPart(rBodyModelPart),
            mrFluidModelPart(rFluidModelPart)
    {
    }

    /// Assignment operator.
    DefineWake2DProcess& operator=(DefineWake2DProcess const& rOther) = delete;

    /// Destructor.
    ~DefineWake2DProcess() override = default;

    /// Copy constructor.
    DefineWake2DProcess(DefineWake2DProcess const& rOther) = delete;

    ///@}
    ///@name Operators
    ///@{
    /// This operator is provided to call the process as a function and simply calls the Execute method.
    void operator()()
    {
        Execute();
    }

    ///@}
    ///@name Operations
    ///@{

    //void SaveTrailingEdgeNodecpp();
    //void MarkWakeElementscpp();
    /// Check elements to make sure that their jacobian is positive and conditions to ensure that their face normals point outwards
    void Execute() override;
    // {
    //     KRATOS_TRY;
    //     std::cout << "Hi"<< std::endl;
    //     //auto trailing_edge = mrBodyModelPart.NodesBegin();
    //     //SaveTrailingEdgeNodecpp(trailing_edge);
    //     //MarkWakeElementscpp(trailing_edge);


    //     KRATOS_CATCH("");
    // }

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "DefineWake2DProcess";
    }


    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "DefineWake2DProcess";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        this->PrintInfo(rOStream);
    }


private:

    ModelPart& mrBodyModelPart;
    ModelPart& mrFluidModelPart;
    //Parameters mSettings;   /// The settings of the problem (names of the conditions and elements)
    //Flags mrOptions;

    //void SaveTrailingEdgeNodecpp();
    template <typename TE> void SaveTrailingEdgeNodecpp(TE &trailing_edge_node);
    template <typename TE> void MarkWakeElementscpp(TE trailing_edge);
    template <typename GE, typename IT> void MarkTrailingEdgeElementscpp(ModelPart& trailing_edge_model_part, GE geom, IT it_elem);
    template <typename GE, typename IT, typename TE> void SelectPotentiallyWakeElementscpp(GE geom, IT it_elem, TE trailing_edge);
    //void MarkTrailingEdgeElementscpp(ModelPart& trailing_edge_model_part, int i);
    //void SelectPotentiallyWakeElementscpp(int i);
    //void MarkWakeElementscpp();
}; // Class Process



/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  DefineWake2DProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const DefineWake2DProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

} // namespace Kratos

#endif // KRATOS_DEFINE_WAKE_2D_PROCESS_H