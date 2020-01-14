#if !defined(KRATOS_CAD_JSON_INPUT_INCLUDED )
#define  KRATOS_CAD_JSON_INPUT_INCLUDED


// System includes

// External includes

// Project includes
#include "includes/io.h"
#include "includes/kratos_parameters.h"
#include "includes/model_part.h"
#include "includes/node.h"

// Geometries
#include "geometries/nurbs_curve_geometry.h"
#include "geometries/nurbs_surface_geometry.h"
#include "geometries/brep_surface.h"
#include "geometries/brep_curve_on_surface.h"
#include "geometries/geometry.h"

namespace Kratos
{

  ///@name Kratos Classes
  ///@{
  /// Short class definition.
  /** Gives IO capabilities for Nurbs based Brep models in the JSON format defined in 
  https://amses-journal.springeropen.com/articles/10.1186/s40323-018-0109-4.
  */
    template<class TNodeType = Node<3>, class TEmbeddedNodeType = Point>
    class KRATOS_API(KRATOS_CORE) CadJsonInput //: public IO
    {
    public:

        ///@}
        ///@name Type Definitions
        ///@{

        /// Pointer definition of CadJsonInput
        KRATOS_CLASS_POINTER_DEFINITION(CadJsonInput);

        typedef std::size_t SizeType;
        typedef std::size_t IndexType;

        typedef PointerVector<TNodeType> ContainerPointType;
        typedef PointerVector<TEmbeddedNodeType> ContainerPointEmbeddedType;

        typedef NurbsSurfaceGeometry<3, ContainerPointType> NurbsSurfaceType;
        typedef BrepSurface<ContainerPointType, ContainerPointEmbeddedType> BrepSurfaceType;
        typedef BrepCurveOnSurface<ContainerPointType, ContainerPointEmbeddedType> BrepCurveOnSurfaceType;

        typedef DenseVector<typename BrepCurveOnSurfaceType::Pointer> BrepCurveOnSurfaceLoopType;
        typedef DenseVector<DenseVector<typename BrepCurveOnSurfaceType::Pointer>> BrepCurveOnSurfaceLoopArrayType;

        ///@}
        ///@name Life Cycle
        ///@{

        /// Constructor.
        CadJsonInput(
            const Parameters& rCadJsonParameters)
            : mCadJsonParameters(rCadJsonParameters)
        {};

        /// Destructor.
        ~CadJsonInput() = default;

        ///@}
        ///@name Python exposed Functions
        ///@{

        void ReadModelPart(ModelPart& rModelPart)// override
        {
            ReadBreps(mCadJsonParameters, rModelPart);
        }

        ///@}

    private:

        ///@name Read in Brep
        ///@{

        void ReadBreps(
            const Parameters& rParameters,
            ModelPart& rModelPart)
        {
            for (IndexType brep_index = 0; brep_index < rParameters.size(); brep_index++)
            {
                ReadBrepFaces(rParameters[brep_index], rModelPart);
            }

            for (IndexType brep_index = 0; brep_index < rParameters.size(); brep_index++)
            {
                ReadBrepEdges(rParameters[brep_index], rModelPart);
            }
        }

        void ReadBrepFaces(
            const Parameters& rParameters,
            ModelPart& rModelPart)
        {
            if (rParameters.Has("faces"))
                ReadBrepSurfaces(rParameters["faces"], rModelPart);
        }

        void ReadBrepEdges(
            const Parameters& rParameters,
            ModelPart& rModelPart)
        {
            if (rParameters.Has("edges"))
                ReadBrepCurveOnSurfaces(rParameters["edges"], rModelPart);
        }

        ///@}
        ///@name Read in Brep Geometries
        ///@{

        void ReadBrepSurfaces(
                const Parameters& rParameters,
                ModelPart& rModelPart)
        {
            for (IndexType i = 0; i < rParameters.size(); i++)
            {
                KRATOS_ERROR_IF_NOT(rParameters[i].Has("brep_id") || rParameters[i].Has("brep_name"))
                    << "Missing 'brep_id' or 'brep_name' in brep face" << std::endl;

                KRATOS_ERROR_IF_NOT(rParameters[i].Has("surface"))
                    << "Missing 'surface' in brep face" << std::endl;

                auto p_surface = ReadNurbsSurface<3, TNodeType>(rParameters["surface"]);

                bool is_trimmed = true;
                if (rParameters[i]["surface"].Has("is_trimmed"))
                    is_trimmed = rParameters[i]["surface"]["is_trimmed"].GetBool();

                if (rParameters[i].Has("boundary_loops"))
                {
                    BrepCurveOnSurfaceLoopArrayType outer_loops, inner_loops;
                    tie(outer_loops, inner_loops) = ReadBoundaryLoops(rParameters[i]["boundary_loops"], p_surface);

                    auto p_brep_surface =
                        Kratos::make_shared<BrepSurfaceType>(
                            p_surface,
                            outer_loops,
                            inner_loops,
                            is_trimmed);

                    if (rParameters[i].Has("brep_id"))
                        p_brep_surface->SetId(rParameters[i]["brep_id"].GetInt());
                    else if (rParameters[i].Has("brep_name"))
                        p_brep_surface->SetId(rParameters[i]["brep_name"].GetString());

                    rModelPart.AddGeometry(p_brep_surface);
                }
                else
                {
                    auto p_brep_surface =
                        Kratos::make_shared<BrepSurfaceType>(
                            p_surface);

                    if (rParameters[i].Has("brep_id"))
                        p_brep_surface->SetId(rParameters[i]["brep_id"].GetInt());
                    else if (rParameters[i].Has("brep_name"))
                        p_brep_surface->SetId(rParameters[i]["brep_name"].GetString());

                    rModelPart.AddGeometry(p_brep_surface);
                }
            }
        }

        ///@}
        ///@name Read in Surface Trimming
        ///@{

        BrepCurveOnSurfaceLoopType
            ReadTrimmingCurveVector(
                const Parameters& rParameters,
                typename NurbsSurfaceType::Pointer pNurbsSurface)
        {
            KRATOS_ERROR_IF(rParameters.size() < 1)
                << "Trimming curve list has no element." << std::endl;

            BrepCurveOnSurfaceLoopType
                trimming_brep_curve_vector(rParameters.size());

            for (IndexType tc_idx = 0; tc_idx < rParameters.size(); tc_idx++)
            {
                trimming_brep_curve_vector[tc_idx] = ReadTrimmingCurve(rParameters[tc_idx], pNurbsSurface);
            }

            return trimming_brep_curve_vector;
        }

        typename BrepCurveOnSurfaceType::Pointer
            ReadTrimmingCurve(
                const Parameters& rParameters,
                typename NurbsSurfaceType::Pointer pNurbsSurface)
        {
            KRATOS_ERROR_IF_NOT(rParameters.Has("curve_direction"))
                << "Missing 'curve_direction' in nurbs curve" << std::endl;
            bool curve_direction = rParameters["curve_direction"].GetBool();

            KRATOS_ERROR_IF_NOT(rParameters.Has("parameter_curve"))
                << "Missing 'parameter_curve' in nurbs curve" << std::endl;

            auto p_trimming_curve = ReadNurbsCurve<2, TEmbeddedNodeType>(rParameters["parameter_curve"]);

            auto p_brep_curve_on_surface
                = Kratos::make_shared<BrepCurveOnSurfaceType>(
                    pNurbsSurface, p_trimming_curve);

            if (rParameters.Has("trim_index"))
                p_brep_curve_on_surface->SetId(rParameters["trim_index"].GetInt());

            return p_brep_curve_on_surface;
        }

        std::tuple<BrepCurveOnSurfaceLoopArrayType, BrepCurveOnSurfaceLoopArrayType>
            ReadBoundaryLoops(
                const Parameters& rParameters,
                typename NurbsSurfaceType::Pointer pNurbsSurface)
        {
            BrepCurveOnSurfaceLoopArrayType outer_loops;
            BrepCurveOnSurfaceLoopArrayType inner_loops;

            for (IndexType bl_idx = 0; bl_idx < rParameters.size(); bl_idx++)
            {
                KRATOS_ERROR_IF_NOT(rParameters.Has("loop_type"))
                    << "Missing 'loop_type' in boundary loops, in "
                    << bl_idx << " loop." << std::endl;
                std::string loop_type = rParameters["loop_type"].GetString();

                KRATOS_ERROR_IF_NOT(rParameters.Has("trimming_curves"))
                    << "Missing 'trimming_curves' in boundary loops"
                    << bl_idx << " loop." << std::endl;
                auto trimming_curves = ReadTrimmingCurveVector(rParameters["trimming_curves"], pNurbsSurface);

                if (loop_type == "outer")
                {
                    outer_loops.resize(outer_loops.size() + 1);
                    outer_loops[outer_loops.size()-1] = trimming_curves;
                }
                else if (loop_type == "inner")
                {
                    inner_loops.resize(inner_loops.size() + 1);
                    inner_loops[inner_loops.size() - 1] = trimming_curves;
                }
                else
                {
                    KRATOS_ERROR << "Loop type: " << loop_type
                        << " is not supported." << std::endl;
                }
            }

            return std::make_tuple(outer_loops, inner_loops);
        }

        ///@}
        ///@name Read in Nurbs Geometries
        ///@{

        void ReadBrepCurveOnSurfaces(
            const Parameters& rParameters,
            ModelPart& rModelPart)
        {
            for (IndexType i = 0; i < rParameters.size(); i++)
            {
                ReadBrepEdge(rParameters[i], rModelPart);
            }
        }

        void ReadBrepEdge(
            const Parameters& rParameters,
            ModelPart& rModelPart)
        {
                KRATOS_ERROR_IF_NOT(rParameters.Has("brep_id") || rParameters.Has("brep_name"))
                    << "Missing 'brep_id' or 'brep_name' in brep face" << std::endl;

                if (rParameters.Has("topology"))
                {
                    for (IndexType i = 0; i < rParameters["topology"].size(); i++)
                    {
                        KRATOS_ERROR_IF_NOT(rParameters["topology"][i].Has("brep_id") || rParameters["topology"][i].Has("brep_name"))
                            << "Missing 'brep_id' or 'brep_name' in brep face" << std::endl;

                        if (rParameters["topology"][i].Has("brep_id"))
                        {
                            auto p_geometry = rModelPart.pGetGeometry(rParameters["topology"][i]["brep_id"].GetInt());
                            auto p_trim = p_geometry->pGetGeometryPart(rParameters["topology"][i]["trim_index"].GetInt());
                        }
                    }
                }
        }

        ///@}
        ///@name Read in Nurbs Geometries
        ///@{

        template<int TWorkingSpaceDimension, class TThisNodeType>
        typename NurbsCurveGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>::Pointer
            ReadNurbsCurve(
                const Parameters& rParameters)
        {
            bool is_rational = true;
            if (rParameters.Has("is_rational"))
                is_rational = rParameters["is_rational"].GetBool();

            KRATOS_ERROR_IF_NOT(rParameters.Has("knot_vector"))
                << "Missing 'knot_vector' in nurbs curve" << std::endl;
            Vector knot_vector = rParameters["knot_vector"].GetVector();

            KRATOS_ERROR_IF_NOT(rParameters["parameter_curve"].Has("degree"))
                << "Missing 'degree' in nurbs curve" << std::endl;
            int polynomial_degree = rParameters["degree"].GetInt();

            auto control_points = ReadControlPointVector<TThisNodeType>(
                rParameters["control_points"]);

            if (is_rational)
            {
                Vector control_point_weights = ReadControlPointWeightVector(
                    rParameters["control_points"]);

                return Kratos::make_shared<NurbsCurveGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>>(
                    NurbsCurveGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>(
                        control_points,
                        polynomial_degree,
                        knot_vector));
            }
            return Kratos::make_shared<NurbsCurveGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>>(
                NurbsCurveGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>(
                    control_points,
                    polynomial_degree,
                    knot_vector));
        }

        template<int TWorkingSpaceDimension, class TThisNodeType>
        typename NurbsSurfaceGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>::Pointer
            ReadNurbsSurface(
                const Parameters& rParameters)
        {
            bool is_rational = true;
            if(rParameters.Has("is_rational"))
                is_rational = rParameters["is_rational"].GetBool();

            KRATOS_ERROR_IF_NOT(rParameters.Has("knot_vectors"))
                << "Missing 'knot_vector' in nurbs surface" << std::endl;
            KRATOS_ERROR_IF(rParameters["knot_vectors"].size() != 2)
                << "'knot_vectors' need to be of size two, knot_vector_u and knot_vector_v" << std::endl;
            Vector knot_vector_u = rParameters["knot_vectors"][0].GetVector();
            Vector knot_vector_v = rParameters["knot_vectors"][1].GetVector();

            KRATOS_ERROR_IF_NOT(rParameters.Has("degrees"))
                << "Missing 'degrees' in nurbs surface" << std::endl;
            KRATOS_ERROR_IF(rParameters["degrees"].size() != 2)
                << "'degrees' need to be of size two, p and q" << std::endl;
            int p = rParameters["degrees"][0].GetInt();
            int q = rParameters["degrees"][1].GetInt();

            auto control_points = ReadControlPointVector<TThisNodeType>(
                rParameters["control_points"]);

            if (is_rational)
            {
                Vector control_point_weights = ReadControlPointWeightVector(
                    rParameters["control_points"]);

                return Kratos::make_shared<NurbsSurfaceGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>>(
                        control_points,
                        p,
                        q,
                        knot_vector_u,
                        knot_vector_v,
                        control_point_weights);
            }
            return Kratos::make_shared<NurbsSurfaceGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>>(
                NurbsSurfaceGeometry<TWorkingSpaceDimension, PointerVector<TThisNodeType>>(
                    control_points,
                    p,
                    q,
                    knot_vector_u,
                    knot_vector_v));
        }

        ///@}
        ///@name Read in Control Points
        ///@{

        Vector ReadControlPointWeightVector(
            const Parameters& rParameters)
        {
            Vector control_point_weights = ZeroVector(rParameters.size());
            KRATOS_ERROR_IF(rParameters.size() == 0)
                << "Length of control point list is zero!" << std::endl;
            KRATOS_ERROR_IF(rParameters[0].size() != 4)
                << "Control points need to be provided in following structure: [[x, y, z, weight]] or [id, [x, y, z, weight]]"
                << "Size of inner vector incorrect!"
                << std::endl;

            SizeType number_of_entries = rParameters[0].size();
            KRATOS_ERROR_IF(number_of_entries != 1 || number_of_entries != 2)
                << "Control points need to be provided in following structure: [[x, y, z, weight]] or [id, [x, y, z, weight]]"
                << std::endl;

            for (IndexType cp_idx = 0; cp_idx < rParameters.size(); cp_idx++)
            {
                control_point_weights[cp_idx] = rParameters[cp_idx][number_of_entries - 1][3].GetDouble();
            }

            return control_point_weights;
        }

        template<class TThisNodeType>
        PointerVector<TThisNodeType>
            ReadControlPointVector(
                const Parameters& rParameters)
        {
            PointerVector<TThisNodeType> control_points(rParameters.size());

            for (IndexType cp_idx = 0; cp_idx < rParameters.size(); cp_idx++)
            {
                control_points[cp_idx] = ReadNode<TThisNodeType>(rParameters[cp_idx]);
            }

            return control_points;
        }

        ///@}
        ///@name Read in Nodes/ Points
        ///@{

        template<class TThisNodeType>
        TThisNodeType
            ReadNode(
                const Parameters& rParameters)
        {
            SizeType number_of_entries = rParameters[0].size();
            KRATOS_ERROR_IF(number_of_entries != 1 || number_of_entries != 2)
                << "Control points need to be provided in following structure: [[x, y, z, weight]] or [id, [x, y, z, weight]]"
                << std::endl;
            if (number_of_entries == 1)
            {
                Vector cp = rParameters[0].GetVector();

                return TThisNodeType(0, cp[0], cp[1], cp[2]);
            }
            else
            {
                SizeType id = rParameters[0].GetInt();
                Vector cp = rParameters[1].GetVector();

                return TThisNodeType(id, cp[0], cp[1], cp[2]);
            }
        }

        ///@}
        ///@name Members
        ///@{

        const Parameters& mCadJsonParameters;
        int mEchoLevel;

        ///@}
    }; // Class CadJsonInput
}  // namespace Kratos.

#endif // KRATOS_CAD_JSON_INPUT_INCLUDED  defined