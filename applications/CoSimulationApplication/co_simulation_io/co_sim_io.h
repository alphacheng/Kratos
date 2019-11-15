// KRATOS  / ___|___/ ___|(_)_ __ ___  _   _| | __ _| |_(_) ___  _ ___
//        | |   / _ \___ \| | '_ ` _ \| | | | |/ _` | __| |/ _ \| '_  |
//        | |__| (_) |__) | | | | | | | |_| | | (_| | |_| | (_) | | | |
//         \____\___/____/|_|_| |_| |_|\__,_|_|\__,_|\__|_|\___/|_| |_|
//
//  License:		 BSD License
//                   license: CoSimulationApplication/license.txt
//
//  Main authors:    Philipp Bucher
//

#ifndef KRATOS_CO_SIM_IO_H_INCLUDED
#define KRATOS_CO_SIM_IO_H_INCLUDED

/*
This file defines the IO of Kratos-CoSimulation for the exchange of data
with external solvers.
By default the communication is done through files,
support for sockets and MPI can optionally be enabled
*/

// #define KRATOS_CO_SIM_IO_ENABLE_SOCKETS // uncomment for Sockets support
// #define KRATOS_CO_SIM_IO_ENABLE_MPI // uncomment for MPI support

// System includes
#include <string>
#include <memory>

// Project includes
#include "impl/co_sim_io_impl.h"

namespace CoSim {

using Internals::CoSimIO; // TODO remove this

static void Connect(const char* pName)
{
    using namespace Internals;
    KRATOS_CO_SIM_ERROR_IF(HasIO(pName)) << "A CoSimIO for " << pName << " already exists!" << std::endl;

    s_co_sim_ios[std::string(pName)] = std::unique_ptr<CoSimIO>(new CoSimIO("rName", "rSettings")); // make_unique is C++14
    GetIO(pName).Connect();
}

static void Disconnect(const char* pName)
{
    using namespace Internals;
    KRATOS_CO_SIM_ERROR_IF_NOT(HasIO(pName)) << "Trying to disconnect CoSimIO " << pName << " which does not exist!" << std::endl;

    GetIO(pName).Disconnect();
    s_co_sim_ios.erase(std::string(pName));
}

static void ImportData(const char* pName)
{
    using namespace Internals;
}




} // namespace CoSim


#endif /* KRATOS_CO_SIM_IO_H_INCLUDED */