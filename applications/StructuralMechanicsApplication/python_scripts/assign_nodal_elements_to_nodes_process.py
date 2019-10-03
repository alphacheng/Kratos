from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics
import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication

def Factory(settings, Model):
    if not isinstance(settings, KratosMultiphysics.Parameters):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return AssignNodalElementsToNodesProcess(Model, settings["Parameters"])

## All the processes python should be derived from "Process"
class AssignNodalElementsToNodesProcess(KratosMultiphysics.Process):
    """This process creates nodeal elements in all the nodes of a model part

    Only the member variables listed below should be accessed directly.

    Public member variables:
    Model -- the container of the different model parts.
    settings -- Kratos parameters containing solver settings.
    """
    def __init__(self, Model, settings):
        """ The default constructor of the class

        Keyword arguments:
        self -- It signifies an instance of a class.
        Model -- the container of the different model parts.
        settings -- Kratos parameters containing solver settings.
        """

        KratosMultiphysics.Process.__init__(self)

        default_settings = KratosMultiphysics.Parameters("""
        {
            "main_model_part_name"           : "Structure",
            "sub_model_part_name"            : "",
            "rayleigh_damping"               : false,
            "interval"                       : [0.0, 1e30]
        }
        """
        )

        to_validate_parameters = KratosMultiphysics.Parameters("""{}""")
        if settings.Has("mesh_id":
            to_validate_parameters.AddValue("mesh_id", settings["mesh_id"])
        if settings.Has("model_part_name"):
            to_validate_parameters.AddValue("model_part_name", settings["model_part_name"])
        if settings.Has("sub_model_part_name"):
            to_validate_parameters.AddValue("sub_model_part_name", settings["sub_model_part_name"])
        if settings.Has("rayleigh_damping"):
            to_validate_parameters.AddValue("rayleigh_damping", settings["rayleigh_damping"])
        if settings.Has("interval"):
            to_validate_parameters.AddValue("interval", settings["interval"])

        # Overwrite the default settings with user-provided parameters
        to_validate_parameters.RecursivelyValidateAndAssignDefaults(default_settings)
        if settings.Has("mesh_id"):
            settings.SetValue("mesh_id", to_validate_parameters["mesh_id"])
        else:
            settings.AddValue("mesh_id", to_validate_parameters["mesh_id"])
        if settings.Has("model_part_name"):
            settings.SetValue("model_part_name", to_validate_parameters["model_part_name"])
        else:
            settings.AddValue("model_part_name", to_validate_parameters["model_part_name"])
        if settings.Has("sub_model_part_name"):
            settings.SetValue("sub_model_part_name", to_validate_parameters["sub_model_part_name"])
        else:
            settings.AddValue("sub_model_part_name", to_validate_parameters["sub_model_part_name"])
        if settings.Has("rayleigh_damping"):
            settings.SetValue("rayleigh_damping", to_validate_parameters["rayleigh_damping"])
        else:
            settings.AddValue("rayleigh_damping", to_validate_parameters["rayleigh_damping"])
        if settings.Has("interval"):
            settings.SetValue("interval", to_validate_parameters["interval"])
        else:
            settings.AddValue("interval", to_validate_parameters["interval"])

        # List of auxiliar parameters to assign in case not defined
        auxiliar_parameters = KratosMultiphysics.Parameters("""
        {
            "nodal_mass"                     : null,
            "nodal_inertia"                  : [null, null, null],
            "nodal_stiffness"                : [null, null, null],
            "nodal_rotational_stiffness"     : [null, null, null],
            "nodal_damping_ratio"            : [null, null, null],
            "nodal_rotational_damping_ratio" : [null, null, null]
        }
        """
        )

        if not settings.Has("nodal_mass"):
            settings.AddValue("nodal_mass", auxiliar_parameters["nodal_mass"])
        if not settings.Has("nodal_inertia"):
            settings.AddValue("nodal_inertia", auxiliar_parameters["nodal_inertia"])
        if not settings.Has("nodal_stiffness"):
            settings.AddValue("nodal_stiffness", auxiliar_parameters["nodal_stiffness"])
        if not settings.Has("nodal_rotational_stiffness"):
            settings.AddValue("nodal_rotational_stiffness", auxiliar_parameters["nodal_rotational_stiffness"])
        if not settings.Has("nodal_damping_ratio"):
            settings.AddValue("nodal_damping_ratio", auxiliar_parameters["nodal_damping_ratio"])
        if not settings.Has("nodal_rotational_damping_ratio"):
            settings.AddValue("nodal_rotational_damping_ratio", auxiliar_parameters["nodal_rotational_damping_ratio"])

        # The main model part
        self.model = Model
        self.main_model_part = self.model[settings["model_part_name"].GetString()]

        # The creation of the process
        process_parameters = KratosMultiphysics.Parameters("""{}""")
        process_parameters.AddValue("model_part_name", settings["sub_model_part_name"])
        process_parameters.AddValue("rayleigh_damping", settings["rayleigh_damping"])
        process_parameters.AddValue("nodal_mass", settings["nodal_mass"])
        process_parameters.AddValue("nodal_inertia", settings["nodal_inertia"])
        process_parameters.AddValue("nodal_stiffness", settings["nodal_stiffness"])
        process_parameters.AddValue("nodal_rotational_stiffness", settings["nodal_rotational_stiffness"])
        process_parameters.AddValue("nodal_damping_ratio", settings["nodal_damping_ratio"])
        process_parameters.AddValue("nodal_rotational_damping_ratio", settings["nodal_rotational_damping_ratio"])
        process_parameters.AddValue("interval", settings["interval"])
        self.assign_nodal_elements_to_nodes = StructuralMechanicsApplication.AssignNodalElementsToNodesProcess(self.main_model_part, process_parameters)

    def ExecuteInitialize(self):
        """ This method is executed at the begining to initialize the process

        Keyword arguments:
        self -- It signifies an instance of a class.
        """
        self.assign_nodal_elements_to_nodes.ExecuteInitialize()

    def ExecuteInitializeSolutionStep(self):
        """ This method is executed in order to initialize the current step

        Keyword arguments:
        self -- It signifies an instance of a class.
        """
        self.assign_nodal_elements_to_nodes.ExecuteInitializeSolutionStep()