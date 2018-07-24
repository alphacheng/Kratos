/*
==============================================================================
KratosIncompressibleFluidApplication 
A library based on:
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi 
pooyan@cimne.upc.edu 
rrossi@cimne.upc.edu
- CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain


Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNERS.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/
 
//   
//   Project Name:        Kratos       
//   Last Modified by:    $Author: rrossi $
//   Date:                $Date: 2008-10-13 06:58:23 $
//   Revision:            $Revision: 1.2 $
//
//


#if !defined(ASSIGN_POINT_NEUMANN3D_CONDITION_SURFACE_TENSION )
#define  ASSIGN_POINT_NEUMANN3D_CONDITION



// System includes
#include <string>
#include <iostream> 
#include <algorithm>

// External includes 


// Project includes
#include <pybind11/pybind11.h>
#include "includes/define.h"
#include "includes/define_python.h"
#include "includes/model_part.h"
#include "includes/node.h"
#include "utilities/geometry_utilities.h"
#include "geometries/point.h"
#include "ULF_application.h"
#include "custom_conditions/Point_Neumann_Monolithic2D.h"
#include "custom_conditions/Point_Neumann2D.h"
// #include "custom_conditions/Point_Neumann_Monolithic3D.h"




namespace Kratos
{
 	

	class AssignPointNeumannConditionsEmbedded
	{
	public:

		//**********************************************************************************************
		//**********************************************************************************************
		//
		//template<unsigned int TDim>
		void AssignPointNeumannConditions3D(ModelPart& ThisModelPart)
		{			
			KRATOS_TRY;
			KRATOS_WATCH("Inside of AssignPointNeumannConditionsEmbedded UTILITY")
			const int TDim=ThisModelPart.ElementsBegin()->GetGeometry().WorkingSpaceDimension();

			Properties::Pointer properties = ThisModelPart.GetMesh().pGetProperties(1);
			int id = ThisModelPart.Conditions().size();
			KRATOS_WATCH(id)
                        //const char* ConditionName = "NoSlipCondition2D";
			KRATOS_WATCH(ThisModelPart.Conditions().size())
			for(ModelPart::NodesContainerType::iterator it = ThisModelPart.NodesBegin(); 
				it!=ThisModelPart.NodesEnd(); it++)
			 {
				if( it->FastGetSolutionStepValue(IS_FREE_SURFACE) == 1.0)  
				   {	
	      			       //KRATOS_WATCH(" ASSIGNING THE NEUMANN POINT CONDITIONS -> EXTERNAL PRESSURE WILL BE APPLIED ");
				       Condition::NodesArrayType temp;	
				       temp.reserve(1);
				       temp.push_back(*(it.base()));	
		
					if (TDim==3)
						{
						//Condition::Pointer p_cond = (KratosComponents<Condition>::Get("PointNeumann3D")).Create(id, temp, properties);	
						Condition::Pointer p_cond = (KratosComponents<Condition>::Get("PointNeumannMonolithic3D")).Create(id, temp, properties);	
						(ThisModelPart.Conditions()).push_back(p_cond);
						}
					else if (TDim==2)
						{
						//Condition::Pointer p_cond = (KratosComponents<Condition>::Get("PointNeumann2D")).Create(id, temp, properties);	
						Condition::Pointer p_cond = (KratosComponents<Condition>::Get("PointNeumannMonolithic2D")).Create(id, temp, properties);
						(ThisModelPart.Conditions()).push_back(p_cond);
						}
					
					id++;
					
				  }

			}
// 			KRATOS_WATCH(id)
			KRATOS_WATCH(ThisModelPart.Conditions().size())
			KRATOS_WATCH("BEFORE SORTING")
			ThisModelPart.Conditions().Sort();
			KRATOS_WATCH(ThisModelPart.Conditions().size())

			KRATOS_CATCH("")
		}



	private:


	};

}  // namespace Kratos.

#endif // ASSIGN_POINT_NEUMANN3D_CONDITION  defined 


