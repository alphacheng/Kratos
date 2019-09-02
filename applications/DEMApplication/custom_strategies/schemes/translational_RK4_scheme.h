#if !defined(KRATOS_TRANSLATIONAL_RK4_SCHEME_H_INCLUDED )
#define  KRATOS_TRANSLATIONAL_RK4_SCHEME_H_INCLUDED

// System includes
#include <string>
#include <iostream>
#include <cfloat>

// Project includes
#include "dem_integration_scheme.h"
#include "includes/define.h"
#include "utilities/openmp_utils.h"
#include "includes/model_part.h"
#include "custom_utilities/GeometryFunctions.h"
#include "utilities/quaternion.h"

namespace Kratos {

    class KRATOS_API(DEM_APPLICATION) TranslationalRungeKuttaScheme : public DEMIntegrationScheme {
    public:

        typedef ModelPart::NodesContainerType NodesArrayType;

        /// Pointer definition of TranslationalRungeKuttaScheme
        KRATOS_CLASS_POINTER_DEFINITION(TranslationalRungeKuttaScheme);

        /// Default constructor.
        TranslationalRungeKuttaScheme() {}

        /// Destructor.
        virtual ~TranslationalRungeKuttaScheme() {}

        DEMIntegrationScheme* CloneRaw() const override {
            DEMIntegrationScheme* cloned_scheme(new TranslationalRungeKuttaScheme(*this));
            return cloned_scheme;
        }

        DEMIntegrationScheme::Pointer CloneShared() const override {
            DEMIntegrationScheme::Pointer cloned_scheme(new TranslationalRungeKuttaScheme(*this));
            return cloned_scheme;
        }

        BoundedMatrix<double, 4, 3> mRungeKuttaK = ZeroMatrix(4,3);
        BoundedMatrix<double, 4, 3> mRungeKuttaL = ZeroMatrix(4,3);
        array_1d<double, 3> mInitialDispl = ZeroVector(3);

        void SetTranslationalIntegrationSchemeInProperties(Properties::Pointer pProp, bool verbose = true) const override;
        void SetRotationalIntegrationSchemeInProperties(Properties::Pointer pProp, bool verbose = true) const override;

        void UpdateTranslationalVariables(
                int StepFlag,
                Node < 3 >& i,
                array_1d<double, 3 >& coor,
                array_1d<double, 3 >& displ,
                array_1d<double, 3 >& delta_displ,
                array_1d<double, 3 >& vel,
                const array_1d<double, 3 >& initial_coor,
                const array_1d<double, 3 >& force,
                const double force_reduction_factor,
                const double mass,
                const double delta_t,
                const bool Fix_vel[3]) override;

        /// Turn back information as a string.

        virtual std::string Info() const override{
            std::stringstream buffer;
            buffer << "TranslationalRungeKuttaScheme";
            return buffer.str();
        }

        /// Print information about this object.

        virtual void PrintInfo(std::ostream& rOStream) const override{
            rOStream << "TranslationalRungeKuttaScheme";
        }

        /// Print object's data.

        virtual void PrintData(std::ostream& rOStream) const override{
        }


    protected:

    private:

        /// Assignment operator.
        TranslationalRungeKuttaScheme& operator=(TranslationalRungeKuttaScheme const& rOther) {
            return *this;
        }

        /// Copy constructor.
        TranslationalRungeKuttaScheme(TranslationalRungeKuttaScheme const& rOther) {
            *this = rOther;
        }
    };


    inline std::istream& operator>>(std::istream& rIStream,
            TranslationalRungeKuttaScheme& rThis) {
        return rIStream;
    }

    inline std::ostream& operator<<(std::ostream& rOStream,
            const TranslationalRungeKuttaScheme& rThis) {
        rThis.PrintInfo(rOStream);
        rOStream << std::endl;
        rThis.PrintData(rOStream);

        return rOStream;
    }

}

#endif