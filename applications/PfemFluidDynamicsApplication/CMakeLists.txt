set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring KratosPfemFluidDynamicsApplication ****")

################### PYBIND11
include(pybind11Tools)

include_directories( ${CMAKE_SOURCE_DIR}/kratos )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../DelaunayMeshingApplication )

kratos_add_dependency(${KRATOS_SOURCE_DIR}/applications/DelaunayMeshingApplication)

if(NOT DEFINED ${INCLUDE_TRIANGLE})
  set(INCLUDE_TRIANGLE ON)
  set(TRIANGLE_INSTALL_DIR ${CMAKE_SOURCE_DIR}/external_libraries/triangle/)
  include_directories( ${TRIANGLE_INSTALL_DIR} )
endif(NOT DEFINED ${INCLUDE_TRIANGLE})

if(NOT DEFINED ${INCLUDE_TETGEN})
  set(INCLUDE_TETGEN ON)
  set(TETGEN_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DelaunayMeshingApplication/external_modules/tetgen/)
  include_directories( ${TETGEN_INSTALL_DIR} )
  link_directories( ${TETGEN_INSTALL_DIR} )
endif(NOT DEFINED ${INCLUDE_TETGEN})


## generate variables with the sources
set( KRATOS_PFEM_FLUID_DYNAMICS_APPLICATION_CORE
	${CMAKE_CURRENT_SOURCE_DIR}/pfem_fluid_dynamics_application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pfem_fluid_dynamics_application_variables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_nodally_integrated_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_solid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_nodally_integrated_solid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_nodally_integrated_fluid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/updated_lagrangian_V_implicit_solid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_fluid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_implicit_fluid_DEM_coupling_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_explicit_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_explicit_solid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/updated_lagrangian_V_explicit_solid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/two_step_updated_lagrangian_V_P_explicit_fluid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_processes/assign_scalar_variable_to_pfem_entities_process.cpp
)

set( KRATOS_PFEM_FLUID_DYNAMICS_APPLICATION_PYTHON_INTERFACE
    # Custom python
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/pfem_fluid_dynamics_python_application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_strategies_to_python.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_processes_to_python.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_utilities_to_python.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_constitutive_laws_to_python.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_modelers_to_python.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_bounding_to_python.cpp
)


add_library(KratosPfemFluidDynamicsCore SHARED ${KRATOS_PFEM_FLUID_DYNAMICS_APPLICATION_CORE})
target_link_libraries(KratosPfemFluidDynamicsCore PUBLIC KratosCore KratosDelaunayMeshingCore)
set_target_properties(KratosPfemFluidDynamicsCore PROPERTIES COMPILE_DEFINITIONS "PFEM_FLUID_DYNAMICS_APPLICATION=EXPORT,API")

###############################################################
## define library Kratos which defines the basic python interface
pybind11_add_module(KratosPfemFluidDynamicsApplication MODULE THIN_LTO ${KRATOS_PFEM_FLUID_DYNAMICS_APPLICATION_PYTHON_INTERFACE})
target_link_libraries(KratosPfemFluidDynamicsApplication PRIVATE KratosPfemFluidDynamicsCore)
set_target_properties(KratosPfemFluidDynamicsApplication PROPERTIES PREFIX "")

# get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
# message("TestApplication subdir inc_dirs = ${inc_dirs}")

# changing the .dll suffix to .pyd
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set_target_properties(KratosPfemFluidDynamicsApplication PROPERTIES SUFFIX .pyd)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# changing the .dylib suffix to .so
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(KratosPfemFluidDynamicsApplication PROPERTIES SUFFIX .so)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Cotire
if(USE_COTIRE MATCHES ON)
    cotire(KratosPfemFluidDynamicsCore)
    cotire(KratosPfemFluidDynamicsApplication)
endif(USE_COTIRE MATCHES ON)

# Add to the KratosMultiphisics Python module
kratos_python_install(${INSTALL_PYTHON_USING_LINKS} ${CMAKE_CURRENT_SOURCE_DIR}/PfemFluidDynamicsApplication.py KratosMultiphysics/PfemFluidDynamicsApplication/__init__.py )

# Install python files
get_filename_component (CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
kratos_python_install_directory(${INSTALL_PYTHON_USING_LINKS} ${CMAKE_CURRENT_SOURCE_DIR}/python_scripts KratosMultiphysics/${CURRENT_DIR_NAME} )

# Kratos Testing. Install everything except sources to ensure that reference and configuration files are copied.
if(${INSTALL_TESTING_FILES} MATCHES ON )
    get_filename_component (CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests DESTINATION applications/${CURRENT_DIR_NAME}
        PATTERN "*.git" EXCLUDE
        PATTERN "*.c" EXCLUDE
        PATTERN "*.h" EXCLUDE
        PATTERN "*.cpp" EXCLUDE
        PATTERN "*.hpp" EXCLUDE
  )
endif(${INSTALL_TESTING_FILES} MATCHES ON)

# Install targets
install(TARGETS KratosPfemFluidDynamicsCore DESTINATION libs )
install(TARGETS KratosPfemFluidDynamicsApplication DESTINATION libs )