// ==============================================================================
//  ChimeraApplication
//
//  License:         BSD License
//                   license: ChimeraApplication/license.txt
//
//  Main authors:    Aditya Ghantasala, https://github.com/adityaghantasala
//
// ==============================================================================

#ifndef APPLY_MULTI_POINT_CONSTRAINTS_PROCESS_CHIMERA_H
#define APPLY_MULTI_POINT_CONSTRAINTS_PROCESS_CHIMERA_H

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "processes/process.h"
#include "utilities/math_utils.h"
#include "includes/kratos_parameters.h"
// Application includes
#include "custom_utilities/multipoint_constraint_data.hpp"
#include "chimera_application_variables.h"

namespace Kratos
{

class ApplyMultipointConstraintsProcessChimera : public Process
{
public:

    /// Pointer definition of MoveRotorProcess
    KRATOS_CLASS_POINTER_DEFINITION(ApplyMultipointConstraintsProcessChimera);

    typedef MpcData::Pointer MpcDataPointerType;
    typedef Dof<double>* DofPointerType;
    typedef Dof<double> DofType;
    typedef std::map<std::string, MpcDataPointerType> MpcDataMapType;
    typedef MpcData::VariableComponentType VariableComponentType;
    typedef MpcData::VariableDataType VariableDataType;
    typedef ProcessInfo      ProcessInfoType;
    typedef ProcessInfo::Pointer      ProcessInfoPointerType;

    /// Constructor.
    ApplyMultipointConstraintsProcessChimera(  ModelPart& model_part,
                                        Parameters rParameters
                                        ) : Process(Flags()) , mr_model_part(model_part), mpcDataMap()
    {

         Parameters default_parameters( R"(
            {
                "master_model_part_name":"default_master",
                "slave_model_part_name":"default_slave",
                "constraint_sets":["default"],
                "interpolation_type":"nearest_element"            
            }  )" );

        rParameters.ValidateAndAssignDefaults(default_parameters);
        //mrMpcData(model_part.GetValue(KratosComponents< Variable<MpcData> >::Get( "MPC_DATA" ))
        mpcDataMap["default"] = MpcDataPointerType( new MpcData() );
        ProcessInfoPointerType info = mr_model_part.pGetProcessInfo();
        info->SetValue(MPC_POINTER, mpcDataMap["default"]);
    }

    ApplyMultipointConstraintsProcessChimera(  ModelPart& model_part
                                        ) : Process(Flags()) , mr_model_part(model_part), mpcDataMap()
    {

        //mrMpcData(model_part.GetValue(KratosComponents< Variable<MpcData> >::Get( "MPC_DATA" ))
        mpcDataMap["default"] = MpcDataPointerType( new MpcData() );
        ProcessInfoPointerType info = mr_model_part.pGetProcessInfo();
        info->SetValue(MPC_POINTER, mpcDataMap["default"]);

    }    

    void AddMasterSlaveRelation(Node<3> &MasterNode, VariableComponentType& MasterVariable, Node<3> &SlaveNode, VariableComponentType& SlaveVariable, double weight, int PartitionId=0)
    {
        SlaveNode.Set(SLAVE);        
        DofType &pointerSlaveDOF = SlaveNode.GetDof(SlaveVariable);
    	DofType &pointerMasterDOF = MasterNode.GetDof(MasterVariable);
        AddMasterSlaveRelationWithDofs(pointerSlaveDOF, pointerMasterDOF, weight, PartitionId);
    }

    void AddMasterSlaveRelationVariables(Node<3> &MasterNode, VariableDataType& MasterVariable, Node<3> &SlaveNode, VariableDataType& SlaveVariable, double weight, int PartitionId=0)
    {
        SlaveNode.Set(SLAVE);        
        DofType &pointerSlaveDOF = SlaveNode.GetDof(SlaveVariable);
    	DofType &pointerMasterDOF = MasterNode.GetDof(MasterVariable);
        AddMasterSlaveRelationWithDofs(pointerSlaveDOF, pointerMasterDOF, weight, PartitionId);
    }



    void AddMasterSlaveRelationWithDofs(DofType slaveDOF, DofType masterDOF, double masterWeight, int PartitionId=0 )
    {
        ProcessInfoType info = mr_model_part.GetProcessInfo();
        MpcDataPointerType pMpc = info[MPC_POINTER];
        pMpc->AddConstraint(slaveDOF, masterDOF,  masterWeight, PartitionId);
    }


    /// Destructor.
    virtual ~ApplyMultipointConstraintsProcessChimera(){
        /*for(auto mpcDataMapElem : mpcDataMap){
            delete mpcDataMapElem.second;
        }*/
    }


    void ExecuteBeforeSolutionLoop() override
    {
        KRATOS_TRY;



        KRATOS_CATCH("");
    }


    void ExecuteInitializeSolutionStep() override
    {
        KRATOS_TRY;
        
        KRATOS_CATCH("");
    }

    /// Turn back information as a string.
    virtual std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "ApplyMultipointConstraintsProcessChimera" ;
        return buffer.str();
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const override {rOStream << "ApplyMultipointConstraintsProcessChimera";}

    /// Print object's data.
    void PrintData() {
        std::cout<<"Number of slave nodes :: "<<std::endl;
        mpcDataMap["default"]->GetInfo();
    }



protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{
    ModelPart&                                 mr_model_part;
    MpcDataMapType                             mpcDataMap;

private:

    /// Assignment operator.
    ApplyMultipointConstraintsProcessChimera& operator=(ApplyMultipointConstraintsProcessChimera const& rOther){return *this;}

}; // Class MoveRotorProcess

};  // namespace Kratos.

#endif // KRATOS_MOVE_ROTOR_PROCESS_H
