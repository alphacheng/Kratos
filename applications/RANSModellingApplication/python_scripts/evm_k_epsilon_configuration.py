from __future__ import print_function, absolute_import, division

import KratosMultiphysics as Kratos
import KratosMultiphysics.RANSModellingApplication as KratosRANS
from turbulence_eddy_viscosity_model_configuration import TurbulenceEddyViscosityModelConfiguration

from KratosMultiphysics.kratos_utilities import CheckIfApplicationsAvailable
if CheckIfApplicationsAvailable("FluidDynamicsApplication"):
    import KratosMultiphysics.FluidDynamicsApplication
else:
    msg = "k-epsilon turbulence model depends on the FluidDynamicsApplication which is not found."
    msg += " Please re-install/compile with FluidDynamicsApplication"
    raise Exception(msg)


class TurbulenceKEpsilonConfiguration(
        TurbulenceEddyViscosityModelConfiguration):
    def __init__(self, model, parameters):
        super(TurbulenceKEpsilonConfiguration, self).__init__(
            model, parameters)

        default_settings = Kratos.Parameters(r'''{
            "scheme_settings": {},
            "echo_level"          :0,
            "use_high_re_elements": true,
            "turbulent_kinetic_energy_settings":{},
            "turbulent_energy_dissipation_rate_settings":{},
            "constants":
            {
                "wall_smoothness_beta"    : 5.2,
                "von_karman"              : 0.41,
                "c_mu"                    : 0.09,
                "c1"                      : 1.44,
                "c2"                      : 1.92,
                "sigma_k"                 : 1.0,
                "sigma_epsilon"           : 1.3
            },
            "flow_parameters":
            {
                "ramp_up_time"            : 0.5,
                "wall_y_plus"             : 11.06
            },
            "coupling_settings" :{}
        }''')

        parameters["model_settings"].ValidateAndAssignDefaults(
            default_settings)
        parameters["model_settings"]["constants"].ValidateAndAssignDefaults(
            default_settings["constants"])
        parameters["model_settings"][
            "flow_parameters"].ValidateAndAssignDefaults(
                default_settings["flow_parameters"])
        self.model_settings = parameters["model_settings"]

        if (self.model_settings["use_high_re_elements"].GetBool()):
            self.model_elements_list = ["RANSEVMK", "RANSEVMEpsilon"]
            self.model_conditions_list = ["Condition", "RANSEVMEpsilonWallCondition"]
        else:
            self.model_elements_list = ["RANSEVMLowReK", "RANSEVMLowReEpsilon"]
            self.model_conditions_list = ["Condition", "Condition"]
        self.rans_solver_configurations = []
        self.is_initial_values_assigned = False

        self.ramp_up_time = self.model_settings["flow_parameters"][
            "ramp_up_time"].GetDouble()

    def InitializeModelConstants(self):
        # reading constants
        constants = self.model_settings["constants"]
        self.fluid_model_part.ProcessInfo[
            KratosRANS.WALL_SMOOTHNESS_BETA] = constants[
                "wall_smoothness_beta"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.WALL_VON_KARMAN] = constants["von_karman"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENCE_RANS_C_MU] = constants["c_mu"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENCE_RANS_C1] = constants["c1"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENCE_RANS_C2] = constants["c2"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.
            TURBULENT_KINETIC_ENERGY_SIGMA] = constants["sigma_k"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA] = constants[
                "sigma_epsilon"].GetDouble()
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENT_VISCOSITY_MIN] = self.nu_t_min
        self.fluid_model_part.ProcessInfo[
            KratosRANS.TURBULENT_VISCOSITY_MAX] = self.nu_t_max

        if (self.model_settings["use_high_re_elements"].GetBool()):
            self.fluid_model_part.ProcessInfo[
                KratosRANS.RANS_WALL_Y_PLUS] = self.model_settings[
                    "flow_parameters"]["wall_y_plus"].GetDouble()

    def PrepareSolvingStrategy(self):
        scheme_settings = self.model_settings["scheme_settings"]

        # create turbulent kinetic energy strategy
        model_part = self.model_parts_list[0]
        solver_settings = self.model_settings[
            "turbulent_kinetic_energy_settings"]
        scalar_variable = KratosRANS.TURBULENT_KINETIC_ENERGY
        scalar_variable_rate = KratosRANS.TURBULENT_KINETIC_ENERGY_RATE
        relaxed_scalar_variable_rate = KratosRANS.RANS_AUXILIARY_VARIABLE_1
        self.rans_solver_configurations.append(
            self.CreateStrategy(solver_settings, scheme_settings, model_part,
                                scalar_variable, scalar_variable_rate,
                                relaxed_scalar_variable_rate))

        current_strategy = self.rans_solver_configurations[-1][0]
        self.strategies_list.append(current_strategy)
        self.GetTurbulenceSolvingProcess().AddStrategy(current_strategy,
                                                       scalar_variable)

        # create turbulent energy dissipation rate strategy
        model_part = self.model_parts_list[1]
        solver_settings = self.model_settings[
            "turbulent_energy_dissipation_rate_settings"]
        scalar_variable = KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE
        scalar_variable_rate = KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2
        relaxed_scalar_variable_rate = KratosRANS.RANS_AUXILIARY_VARIABLE_2
        self.rans_solver_configurations.append(
            self.CreateStrategy(solver_settings, scheme_settings, model_part,
                                scalar_variable, scalar_variable_rate,
                                relaxed_scalar_variable_rate))

        current_strategy = self.rans_solver_configurations[-1][0]
        self.strategies_list.append(current_strategy)
        self.GetTurbulenceSolvingProcess().AddStrategy(current_strategy,
                                                       scalar_variable)

        Kratos.Logger.PrintInfo(
            self.__class__.__name__,
            "All turbulence solution strategies are created.")

    def AddVariables(self):
        # adding k-epsilon specific variables
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.TURBULENT_KINETIC_ENERGY)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.TURBULENT_KINETIC_ENERGY_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.RANS_AUXILIARY_VARIABLE_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.RANS_AUXILIARY_VARIABLE_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(
            KratosRANS.WALL_VELOCITY)

        super(TurbulenceKEpsilonConfiguration, self).AddVariables()

    def AddDofs(self):
        Kratos.VariableUtils().AddDof(KratosRANS.TURBULENT_KINETIC_ENERGY,
                                      self.fluid_model_part)
        Kratos.VariableUtils().AddDof(
            KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE,
            self.fluid_model_part)

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                "DOFs added successfully.")

    def Initialize(self):
        super(TurbulenceKEpsilonConfiguration, self).Initialize()
        self.InitializeModelConstants()

    def InitializeSolutionStep(self):
        if (self.fluid_model_part.ProcessInfo[KratosRANS.
                                              IS_CO_SOLVING_PROCESS_ACTIVE]):
            super(TurbulenceKEpsilonConfiguration,
                  self).InitializeSolutionStep()

    def FinalizeSolutionStep(self):
        super(TurbulenceKEpsilonConfiguration, self).FinalizeSolutionStep()
        time = self.fluid_model_part.ProcessInfo[Kratos.TIME]
        if (time >= self.ramp_up_time):
            self.fluid_model_part.ProcessInfo[
                KratosRANS.IS_CO_SOLVING_PROCESS_ACTIVE] = True

    def GetTurbulenceSolvingProcess(self):
        if self.turbulence_model_process is None:
            self.turbulence_model_process = KratosRANS.KEpsilonCoSolvingProcess(
                self.fluid_model_part,
                self.model_settings["coupling_settings"], self.GetYPlusModel())

            Kratos.Logger.PrintInfo(self.__class__.__name__,
                                    "Created turbulence solving process.")

        return self.turbulence_model_process
