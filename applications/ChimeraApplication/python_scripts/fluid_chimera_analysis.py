from __future__ import absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics
from KratosMultiphysics.FluidDynamicsApplication.fluid_dynamics_analysis import FluidDynamicsAnalysis
import KratosMultiphysics.ChimeraApplication as KratosChimera
import KratosMultiphysics.ChimeraApplication.python_solvers_wrapper_fluid_chimera

class FluidChimeraAnalysis(FluidDynamicsAnalysis):
    '''Main script for fluid chimera simulations using the navier stokes family of python solvers.'''

    def __init__(self,model,parameters):
        # Deprecation warnings
        self.full_parameters = parameters
        self.solver_parameters = parameters["solver_settings"]
        # Checking if the parameters has 'chimera_settings' entry.
        # This is required.
        if self.solver_parameters.Has("chimera_settings"):
            self.chimera_parameters = self.solver_parameters["chimera_settings"].Clone()
        else:
            raise Exception("The \"solver_settings\" should have the entry \"chimera_settings\" ")

        # Seperating the fluid solver settings.
        if self.solver_parameters.Has("fluid_solver_settings"):
            self.fluid_parameters = self.solver_parameters["fluid_solver_settings"].Clone()
        else:
            self.fluid_parameters = self.solver_parameters.Clone()

        # Extracting the chimera_parts. this is required for ApplyChimera process.
        if self.chimera_parameters.Has("chimera_parts"):
            self.chimera_levels = self.chimera_parameters["chimera_parts"].Clone()
        else:
            raise Exception("The \"solver_settings\" should have the entry \"chimera_parts\" ")

        self.chimera_echo_lvl = 0
        if self.chimera_parameters.Has("chimera_echo_level"):
            self.chimera_echo_lvl = self.chimera_parameters["chimera_echo_level"].GetInt()
        else:
            self.chimera_echo_lvl = self.fluid_parameters["echo_level"].GetInt()

        if self.chimera_parameters.Has("internal_parts_for_chimera"):
            self.chimera_internal_parts = self.chimera_parameters["internal_parts_for_chimera"].Clone()

        self.reformulate_every_step = False
        if self.chimera_parameters.Has("reformulate_chimera_every_step"):
            self.reformulate_every_step = self.chimera_parameters["reformulate_chimera_every_step"].GetBool()
        # Setting reform dofs every step to true "reform_dofs_at_each_step": false,
        else:
            if not self.fluid_parameters.Has("reform_dofs_at_each_step"):
                self.fluid_parameters.AddEmptyValue("reform_dofs_at_each_step")
                self.fluid_parameters["reform_dofs_at_each_step"].SetBool(False)

        # Import parallel modules if needed
        # has to be done before the base-class constuctor is called (in which the solver is constructed)
        # if (parameters["problem_data"]["parallel_type"].GetString() == "MPI"):
        #     raise Exception("MPI-Chimera is not implemented yet")

        self.full_parameters["solver_settings"].RemoveValue("chimera_settings")
        super(FluidChimeraAnalysis,self).__init__(model,self.full_parameters)

    def Initialize(self):
        super(FluidChimeraAnalysis,self).Initialize()
        self.__SetChimeraInternalPartsFlag()

    def _CreateProcesses(self, parameter_name, initialization_order):

        list_of_processes = super(FluidChimeraAnalysis,self)._CreateProcesses(parameter_name, initialization_order)

        main_model_part = self.model[self.fluid_parameters["model_part_name"].GetString()]
        domain_size = main_model_part.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE]
        solver_type = self.fluid_parameters["solver_type"].GetString()


        '''
            Creating the necessary variant of the apply chimera process.
        '''
        if domain_size == 2:
            if(solver_type == "Monolithic" or solver_type == "monolithic"):
                self.chimera_process = KratosChimera.ApplyChimeraProcessMonolithic2d(main_model_part,self.chimera_levels)
            elif (solver_type == "fractional_step" or solver_type == "FractionalStep"):
                self.chimera_process = KratosChimera.ApplyChimeraProcessFractionalStep2d(main_model_part,self.chimera_levels)
        else:
            if(solver_type == "Monolithic" or solver_type == "monolithic"):
                self.chimera_process = KratosChimera.ApplyChimeraProcessMonolithic3d(main_model_part,self.chimera_levels)
            elif (solver_type == "fractional_step" or solver_type == "FractionalStep"):
                self.chimera_process = KratosChimera.ApplyChimeraProcessFractionalStep3d(main_model_part,self.chimera_levels)

        self.chimera_process.SetEchoLevel(self.chimera_echo_lvl)
        self.chimera_process.SetReformulateEveryStep(self.reformulate_every_step)

        return list_of_processes

    def __SetChimeraInternalPartsFlag(self):
        '''
            This function sects the flag CHIMERA_INTERNAL_BOUNDARY on the specified modelparts
            so that they are excluded from the extract surface operation later on.
        '''
        for mp_name in self.chimera_internal_parts:
            KratosMultiphysics.VariableUtils().SetFlag(KratosChimera.CHIMERA_INTERNAL_BOUNDARY, True,  self.model[mp_name.GetString()].Nodes)

    def InitializeSolutionStep(self):
        self.ApplyBoundaryConditions() #here the processes are called
        self.ChangeMaterialProperties() #this is normally empty
        ## The following will construct the constraints
        self.chimera_process.ExecuteInitializeSolutionStep()
        self._GetSolver().InitializeSolutionStep()
        self.PrintAnalysisStageProgressInformation()

    #def RunSolutionLoop(self):
        #"""This function executes the solution loop of the AnalysisStage
        #It can be overridden by derived classes
        #"""
        #while self.KeepAdvancingSolutionLoop():
            #self.time = self._GetSolver().AdvanceInTime(self.time)
            #self.InitializeSolutionStep()
            ##self._GetSolver().Predict()
            ##is_converged = self._GetSolver().SolveSolutionStep()
            ##self.__CheckIfSolveSolutionStepReturnsAValue(is_converged)
            #self.FinalizeSolutionStep()
            #self.OutputSolutionStep()

    def FinalizeSolutionStep(self):
        super(FluidChimeraAnalysis,self).FinalizeSolutionStep()
        ## Depending on the setting this will clear the created constraits
        self.chimera_process.ExecuteFinalizeSolutionStep()


    def _CreateSolver(self):
        return KratosChimera.python_solvers_wrapper_fluid_chimera.CreateSolver(self.model, self.project_parameters)


    def _GetSimulationName(self):
        return "Fluid Chimera Analysis"

if __name__ == '__main__':
    from sys import argv

    if len(argv) > 2:
        err_msg =  'Too many input arguments!\n'
        err_msg += 'Use this script in the following way:\n'
        err_msg += '- With default parameter file (assumed to be called "ProjectParameters.json"):\n'
        err_msg += '    "python fluid_chimera_analysis.py"\n'
        err_msg += '- With custom parameter file:\n'
        err_msg += '    "python fluid_chimera_analysis.py <my-parameter-file>.json"\n'
        raise Exception(err_msg)

    if len(argv) == 2: # ProjectParameters is being passed from outside
        parameter_file_name = argv[1]
    else: # using default name
        parameter_file_name = "ProjectParameters.json"

    with open(parameter_file_name,'r') as parameter_file:
        parameters = KratosMultiphysics.Parameters(parameter_file.read())

    model = KratosMultiphysics.Model()
    simulation = FluidChimeraAnalysis(model,parameters)
    simulation.Run()
