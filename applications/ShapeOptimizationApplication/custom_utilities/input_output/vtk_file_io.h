// ==============================================================================
//  KratosShapeOptimizationApplication
//
//  License:         BSD License
//                   license: ShapeOptimizationApplication/license.txt
//
//  Main authors:    Baumgärtner Daniel, https://github.com/dbaumgaertner
//                   Sferza Massimo, https://github.com/IIIaxS
//
// ==============================================================================

#ifndef VTK_FILE_IO_H
#define VTK_FILE_IO_H

// ------------------------------------------------------------------------------
// System includes
// ------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>      // for setprecision
#include <map>

// ------------------------------------------------------------------------------
// Project includes
// ------------------------------------------------------------------------------
#include "../../kratos/includes/define.h"
#include "../../kratos/processes/process.h"
#include "../../kratos/includes/node.h"
#include "../../kratos/includes/element.h"
#include "../../kratos/includes/model_part.h"
#include "../../kratos/includes/kratos_flags.h"
#include "shape_optimization_application.h"

// ==============================================================================

namespace Kratos
{

///@name Kratos Globals
///@{

using namespace std;

///@}
///@name Type Definitions
///@{


///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.

*/

class VTKFileIO
{
public:
    ///@name Type Definitions
    ///@{

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    VTKFileIO( ModelPart& designSurface, Parameters& optimizationSettings )
        : mrDesignSurface( designSurface ),
          mrOptimizationSettings( optimizationSettings )
    {
        partialOutputFilename = initializeOutputFilenameWithPath( optimizationSettings );
        outputId = 0;
    }

    /// Destructor.
    virtual ~VTKFileIO()
    {
    }


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    // ==============================================================================
    string initializeOutputFilenameWithPath( Parameters& optimizationSettings  )
    {
        string outputDirectory = optimizationSettings["output"]["output_directory"].GetString();
        string outputFilename = outputDirectory + "/" + optimizationSettings["output"]["design_history_filename"].GetString() + "_";
        return outputFilename;
    }

    // --------------------------------------------------------------------------
    void updateOutputFilename()
    {
        string outputFilename = partialOutputFilename + to_string(outputId) + ".vtk";
        outputId++;
        mOutputFilename = outputFilename;
    }
    
    // -------------------------------------------------------------------------- 
    void initializeLogging()
    {
        // fill the dictionary KratosIdToVtkId
        int VtkId = 0;
        for (ModelPart::NodeIterator node_i = mrDesignSurface.NodesBegin(); node_i != mrDesignSurface.NodesEnd(); ++node_i)
        {
            int KratosId = node_i->Id();
            KratosIdToVtkId[KratosId] = VtkId;
            VtkId++;
        }

        // determine VtkCellListSize
        VtkCellListSize = 0;
        for (ModelPart::ConditionIterator condition_i = mrDesignSurface.ConditionsBegin(); condition_i != mrDesignSurface.ConditionsEnd(); ++condition_i)
        {
            VtkCellListSize++;
            
            VtkCellListSize += condition_i->GetGeometry().size(); 
        }
    }    

    // -------------------------------------------------------------------------- 
    void logNodalResults( const int optimizationIteration )
    {
        // initialize output file
        ofstream outputFile;
        updateOutputFilename();
        outputFile.open(mOutputFilename, ios::out | ios::trunc );

        // write header
        outputFile << "# vtk DataFile Version 4.0" << "\n";        
        outputFile << "vtk output" << "\n";        
        outputFile << "ASCII" << "\n";        
        outputFile << "DATASET UNSTRUCTURED_GRID" << "\n";                
        outputFile.close();     

        // write mesh to file
        writeNodes();
        writeElements();
        writeElementTypes();

        // write nodal results
        writeFirstNodalResultsAsPointData();
        writeOtherNodalResultsAsFieldData();

        outputFile.close(); 
    }

    // --------------------------------------------------------------------------
    void writeNodes()
    {
        ofstream outputFile;
        outputFile.open(mOutputFilename, ios::out | ios::app );
        outputFile << scientific;
        outputFile << setprecision(16);

        // write nodes header
        outputFile << "POINTS " << mrDesignSurface.NumberOfNodes() << " float" << "\n";
        
        // write nodes
        for (ModelPart::NodeIterator node_i = mrDesignSurface.NodesBegin(); node_i != mrDesignSurface.NodesEnd(); ++node_i)
        {
            double x_coordinate = node_i->X();
            double y_coordinate = node_i->Y();
            double z_coordinate = node_i->Z();
            outputFile << " " << x_coordinate;
            outputFile << " " << y_coordinate;
            outputFile << " " << z_coordinate << "\n";
        }

        outputFile.close();     
    }

    // --------------------------------------------------------------------------
    void writeElements()
    {
        ofstream outputFile;
        outputFile.open(mOutputFilename, ios::out | ios::app );

        // write elements header
        outputFile << "CELLS " << mrDesignSurface.NumberOfConditions() << " " << VtkCellListSize << "\n";
        
        // write elements
        for (ModelPart::ConditionIterator condition_i = mrDesignSurface.ConditionsBegin(); condition_i != mrDesignSurface.ConditionsEnd(); ++condition_i)
        {
            ModelPart::ConditionType::GeometryType condition_geometry = condition_i->GetGeometry();
            const int numberOfNodes = condition_geometry.size(); 

            if( numberOfNodes == 3 )
            {
                outputFile << numberOfNodes;
                outputFile << " " << KratosIdToVtkId[condition_geometry[0].Id()]; 
                outputFile << " " << KratosIdToVtkId[condition_geometry[1].Id()]; 
                outputFile << " " << KratosIdToVtkId[condition_geometry[2].Id()] << "\n";                
            }
            else if( numberOfNodes == 4 )
            {
                outputFile << numberOfNodes;
                outputFile << " " << KratosIdToVtkId[condition_geometry[0].Id()]; 
                outputFile << " " << KratosIdToVtkId[condition_geometry[1].Id()]; 
                outputFile << " " << KratosIdToVtkId[condition_geometry[2].Id()]; 
                outputFile << " " << KratosIdToVtkId[condition_geometry[3].Id()] << "\n";                
            } 
            else 
                KRATOS_THROW_ERROR(std::runtime_error,"Design surface contains conditions with geometries for which no VTK-output is implemented!","" )
        }

        outputFile.close();     
    }

    // --------------------------------------------------------------------------
    void writeElementTypes()
    {
        ofstream outputFile;
        outputFile.open(mOutputFilename, ios::out | ios::app );

        // write element types header
        outputFile << "CELL_TYPES " << mrDesignSurface.NumberOfConditions() << "\n";
        
        // write elements types
        for (ModelPart::ConditionIterator condition_i = mrDesignSurface.ConditionsBegin(); condition_i != mrDesignSurface.ConditionsEnd(); ++condition_i)
        {
            ModelPart::ConditionType::GeometryType condition_geometry = condition_i->GetGeometry();
            const int numberOfNodes = condition_geometry.size(); 

            if( numberOfNodes == 3 )
            {
                outputFile << 5 << "\n";                
            }
            else if( numberOfNodes == 4 )
            {
                outputFile << 9 << "\n";                
            } 
            else 
                KRATOS_THROW_ERROR(std::runtime_error,"Design surface contains conditions with geometries for which no VTK-output is implemented!","" )
        }

        outputFile.close();     
    }

    // --------------------------------------------------------------------------
    void writeFirstNodalResultsAsPointData()
    {
        ofstream outputFile;
        outputFile.open(mOutputFilename, ios::out | ios::app );
        
        // write nodal results header
        Parameters nodalResults = mrOptimizationSettings["output"]["nodal_results"];
        outputFile << "POINT_DATA " << mrDesignSurface.NumberOfNodes() << "\n";

        // write nodal results variable header            
        string nodalResultName = nodalResults[0].GetString();
        unsigned int dataCharacteristic = 0; // 0: unknown, 1: Scalar value, 2: 3 DOF global translation vector
        if( KratosComponents<Variable<double>>::Has(nodalResultName))
        {
            dataCharacteristic = 1;
            outputFile << "SCALARS " << nodalResultName << " float" << "\n";
        }
        else if( KratosComponents<Variable< array_1d<double,3>>>::Has(nodalResultName))
        {
            dataCharacteristic = 2;
            outputFile << "VECTORS " << nodalResultName << " float" << "\n";
        }
        // write nodal results
        outputFile << scientific;
        outputFile << setprecision(16);
        for (ModelPart::NodeIterator node_i = mrDesignSurface.NodesBegin(); node_i != mrDesignSurface.NodesEnd(); ++node_i)
        {
            if(dataCharacteristic==1)
            {
                Variable<double> nodalResultVariable = KratosComponents<Variable<double>>::Get(nodalResultName);
                double& nodalResult = node_i->FastGetSolutionStepValue(nodalResultVariable);
                outputFile << nodalResult << "\n"; 
            }
            else if(dataCharacteristic==2)
            {
                Variable< array_1d<double,3>> nodalResultVariable = KratosComponents<Variable< array_1d<double,3>>>::Get(nodalResultName);
                array_1d<double,3>& nodalResult = node_i->FastGetSolutionStepValue(nodalResultVariable);
                outputFile << nodalResult[0] << " ";
                outputFile << nodalResult[1] << " ";
                outputFile << nodalResult[2] << "\n"; 
            }
        }
        outputFile.close();     
    }
    
    // --------------------------------------------------------------------------
    void writeOtherNodalResultsAsFieldData()
    {
        ofstream outputFile;
        outputFile.open(mOutputFilename, ios::out | ios::app );
        
        // write nodal results header
        Parameters nodalResults = mrOptimizationSettings["output"]["nodal_results"];
        outputFile << "FIELD FieldData " << nodalResults.size()-1 << "\n";

        // Loop over all nodal result variables
        for(unsigned int entry = 1; entry < nodalResults.size(); entry++)
        {
            // write nodal results variable header            
            string nodalResultName = nodalResults[entry].GetString();
            unsigned int dataCharacteristic = 0; // 0: unknown, 1: Scalar value, 2: 3 DOF global translation vector
            if( KratosComponents<Variable<double>>::Has(nodalResultName))
            {
                dataCharacteristic = 1;
                outputFile << nodalResultName << " 1 " << mrDesignSurface.NumberOfNodes() << " float" << "\n";
            }
            else if( KratosComponents<Variable< array_1d<double,3>>>::Has(nodalResultName))
            {
                dataCharacteristic = 2;
                outputFile << nodalResultName << " 3 " << mrDesignSurface.NumberOfNodes() << " float" << "\n";
            }

            // write nodal results
            outputFile << scientific;
            outputFile << setprecision(16);
            for (ModelPart::NodeIterator node_i = mrDesignSurface.NodesBegin(); node_i != mrDesignSurface.NodesEnd(); ++node_i)
            {
                if(dataCharacteristic==1)
                {
                    Variable<double> nodalResultVariable = KratosComponents<Variable<double>>::Get(nodalResultName);
                    double& nodalResult = node_i->FastGetSolutionStepValue(nodalResultVariable);
                    outputFile << nodalResult << "\n"; 
                }
                else if(dataCharacteristic==2)
                {
                    Variable< array_1d<double,3>> nodalResultVariable = KratosComponents<Variable< array_1d<double,3>>>::Get(nodalResultName);
                    array_1d<double,3>& nodalResult = node_i->FastGetSolutionStepValue(nodalResultVariable);
                    outputFile << nodalResult[0] << " ";
                    outputFile << nodalResult[1] << " ";
                    outputFile << nodalResult[2] << "\n"; 
                }
            }
        }
        outputFile.close();     
    }

    // ==============================================================================

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
    virtual string Info() const
    {
        return "VTKFileIO";
    }

    /// Print information about this object.
    virtual void PrintInfo(ostream& rOStream) const
    {
        rOStream << "VTKFileIO";
    }

    /// Print object's data.
    virtual void PrintData(ostream& rOStream) const
    {
    }


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    // ==============================================================================
    // Initialized by class constructor
    // ==============================================================================
    ModelPart& mrDesignSurface;
    Parameters& mrOptimizationSettings;
    string mOutputFilename;
    string partialOutputFilename;
    int outputId;
    map<int,int> KratosIdToVtkId;
    int VtkCellListSize;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
//      VTKFileIO& operator=(VTKFileIO const& rOther);

    /// Copy constructor.
//      VTKFileIO(VTKFileIO const& rOther);


    ///@}

}; // Class VTKFileIO

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

///@}


}  // namespace Kratos.

#endif // VTK_FILE_IO_H
