// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                   license: structural_mechanics_application/license.txt
//
//  Main authors:    Riccardo Rossi
//
// System includes
#include <iostream>

// External includes

// Project includes
#include "custom_constitutive/elastic_isotropic_3d.h"

#include "structural_mechanics_application_variables.h"

namespace Kratos
{
//******************************CONSTRUCTOR*******************************************
//************************************************************************************

ElasticIsotropic3D::ElasticIsotropic3D()
    : ConstitutiveLaw()
{
}

//******************************COPY CONSTRUCTOR**************************************
//************************************************************************************

ElasticIsotropic3D::ElasticIsotropic3D(const ElasticIsotropic3D& rOther)
    : ConstitutiveLaw(rOther)
{
}

//********************************CLONE***********************************************
//************************************************************************************

ConstitutiveLaw::Pointer ElasticIsotropic3D::Clone() const
{
    ElasticIsotropic3D::Pointer p_clone(new ElasticIsotropic3D(*this));
    return p_clone;
}

//*******************************DESTRUCTOR*******************************************
//************************************************************************************

ElasticIsotropic3D::~ElasticIsotropic3D()
{
};

//************************************************************************************
//************************************************************************************

void  ElasticIsotropic3D::CalculateMaterialResponsePK2(ConstitutiveLaw::Parameters& rValues)
{
    //b.- Get Values to compute the constitutive law:
    Flags & r_options=rValues.GetOptions();

    Vector& r_strain_vector                  = rValues.GetStrainVector();
    Vector& r_stress_vector                  = rValues.GetStressVector();

    //NOTE: SINCE THE ELEMENT IS IN SMALL STRAINS WE CAN USE ANY STRAIN MEASURE. HERE EMPLOYING THE CAUCHY_GREEN
    if( r_options.IsNot( ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN ))
    {
        CalculateCauchyGreenStrain( rValues, r_strain_vector);
    }

    if( r_options.Is( ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR ) )
    {
        Matrix& r_constitutive_matrix = rValues.GetConstitutiveMatrix();
        CalculateElasticMatrix( r_constitutive_matrix, rValues);
    }

    if( r_options.Is( ConstitutiveLaw::COMPUTE_STRESS ) )
    {
        if (rValues.IsSetDeformationGradientF() == true)
        {
            CalculateCauchyGreenStrain(rValues, r_strain_vector);
        }

        if( r_options.Is( ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR ) )
        {
            Matrix& r_constitutive_matrix = rValues.GetConstitutiveMatrix();
            noalias(r_stress_vector) = prod( r_constitutive_matrix, r_strain_vector);
        }
        else
        {
            CalculatePK2Stress( r_strain_vector, r_stress_vector, rValues);
        }
    }
}

//************************************************************************************
//************************************************************************************

// NOTE: Since we are in the hypothesis of small strains we can use the same function for everything

void ElasticIsotropic3D::CalculateMaterialResponsePK1 (ConstitutiveLaw::Parameters& rValues)
{
    CalculateMaterialResponsePK2(rValues);
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CalculateMaterialResponseKirchhoff (ConstitutiveLaw::Parameters& rValues)
{
    CalculateMaterialResponsePK2(rValues);
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CalculateMaterialResponseCauchy (ConstitutiveLaw::Parameters& rValues)
{
    CalculateMaterialResponsePK2(rValues);
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::FinalizeMaterialResponsePK1(ConstitutiveLaw::Parameters& rValues)
{
    // TODO: Add if necessary
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::FinalizeMaterialResponsePK2(ConstitutiveLaw::Parameters& rValues)
{
    // TODO: Add if necessary
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::FinalizeMaterialResponseCauchy(ConstitutiveLaw::Parameters& rValues)
{
    // TODO: Add if necessary
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::FinalizeMaterialResponseKirchhoff(ConstitutiveLaw::Parameters& rValues)
{
    // TODO: Add if necessary
}

//************************************************************************************
//************************************************************************************

double& ElasticIsotropic3D::CalculateValue(ConstitutiveLaw::Parameters& rParameterValues, const Variable<double>& rThisVariable, double& rValue)
{
    Vector& r_strain_vector                  = rParameterValues.GetStrainVector();
    Vector& r_stress_vector                  = rParameterValues.GetStressVector();
    
    if (rThisVariable == STRAIN_ENERGY)
    {
        this->CalculateCauchyGreenStrain(rParameterValues, r_strain_vector);
        this->CalculatePK2Stress( r_strain_vector, r_stress_vector, rParameterValues);

        rValue = 0.5 * inner_prod( r_strain_vector, r_stress_vector); // Strain energy = 0.5*E:C:E
    }

    return( rValue );
}

//*************************CONSTITUTIVE LAW GENERAL FEATURES *************************
//************************************************************************************

void ElasticIsotropic3D::GetLawFeatures(Features& rFeatures)
{
    //Set the type of law
    rFeatures.mOptions.Set( THREE_DIMENSIONAL_LAW );
    rFeatures.mOptions.Set( INFINITESIMAL_STRAINS );
    rFeatures.mOptions.Set( ISOTROPIC );

    //Set strain measure required by the consitutive law
    rFeatures.mStrainMeasures.push_back(StrainMeasure_Infinitesimal);
    rFeatures.mStrainMeasures.push_back(StrainMeasure_Deformation_Gradient);

    //Set the strain size
    rFeatures.mStrainSize = 6;

    //Set the spacedimension
    rFeatures.mSpaceDimension = 3;
}

//************************************************************************************
//************************************************************************************

int ElasticIsotropic3D::Check(
    const Properties& rMaterialProperties,
    const GeometryType& rElementGeometry,
    const ProcessInfo& rCurrentProcessInfo
)
{
    if(YOUNG_MODULUS.Key() == 0 || rMaterialProperties[YOUNG_MODULUS] <= 0.0)
    {
        KRATOS_ERROR << "YOUNG_MODULUS has Key zero or invalid value " << std::endl;
    }

    const double& nu = rMaterialProperties[POISSON_RATIO];
    const bool check = bool( (nu >0.499 && nu<0.501 ) || (nu < -0.999 && nu > -1.01 ) );

    if(POISSON_RATIO.Key() == 0 || check==true)
    {
        KRATOS_ERROR << "POISSON_RATIO has Key zero or invalid value " << std::endl;
    }

    if(DENSITY.Key() == 0 || rMaterialProperties[DENSITY] < 0.0)
    {
        KRATOS_ERROR << "DENSITY has Key zero or invalid value " << std::endl;
    }

    return 0;
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CheckClearElasticMatrix(
    Matrix& C
)
{
    const SizeType size_system = this->GetStrainSize();
    if (C.size1() != size_system || C.size2() != size_system)
        C.resize(size_system, size_system, false);
    C.clear();
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CalculateElasticMatrix(Matrix& C, ConstitutiveLaw::Parameters& rValues)
{
    const Properties& r_material_properties = rValues.GetMaterialProperties();
    const double& E = r_material_properties[YOUNG_MODULUS];
    const double& NU = r_material_properties[POISSON_RATIO];

    this->CheckClearElasticMatrix(C);

    const double c1 = E / (( 1.00 + NU ) * ( 1 - 2 * NU ) );
    const double c2 = c1 * ( 1 - NU );
    const double c3 = c1 * NU;
    const double c4 = c1 * 0.5 * ( 1 - 2 * NU );

    C( 0, 0 ) = c2;
    C( 0, 1 ) = c3;
    C( 0, 2 ) = c3;
    C( 1, 0 ) = c3;
    C( 1, 1 ) = c2;
    C( 1, 2 ) = c3;
    C( 2, 0 ) = c3;
    C( 2, 1 ) = c3;
    C( 2, 2 ) = c2;
    C( 3, 3 ) = c4;
    C( 4, 4 ) = c4;
    C( 5, 5 ) = c4;
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CalculatePK2Stress(
    const Vector& rStrainVector,
    Vector& rStressVector,
    ConstitutiveLaw::Parameters& rValues
)
{
    const Properties& r_material_properties = rValues.GetMaterialProperties();
    const double& E = r_material_properties[YOUNG_MODULUS];
    const double& NU = r_material_properties[POISSON_RATIO];

    const double c1 = E / ((1.00 + NU) * (1 - 2 * NU));
    const double c2 = c1 * (1 - NU);
    const double c3 = c1 * NU;
    const double c4 = c1 * 0.5 * (1 - 2 * NU);

    rStressVector[0] = c2 * rStrainVector[0] + c3 * rStrainVector[1] + c3 * rStrainVector[2];
    rStressVector[1] = c3 * rStrainVector[0] + c2 * rStrainVector[1] + c3 * rStrainVector[2];
    rStressVector[2] = c3 * rStrainVector[0] + c3 * rStrainVector[1] + c2 * rStrainVector[2];
    rStressVector[3] = c4 * rStrainVector[3];
    rStressVector[4] = c4 * rStrainVector[4];
    rStressVector[5] = c4 * rStrainVector[5];
}

//************************************************************************************
//************************************************************************************

void ElasticIsotropic3D::CalculateCauchyGreenStrain(
    ConstitutiveLaw::Parameters& rValues,
    Vector& rStrainVector
)
{
    const SizeType space_dimension = this->WorkingSpaceDimension();

    //1.-Compute total deformation gradient
    const Matrix& F = rValues.GetDeformationGradientF();
    KRATOS_DEBUG_ERROR_IF(F.size1()!= space_dimension || F.size2() != space_dimension) << "the size of DeformationGradientF is not equal to the space dimension" << std::endl;

    Matrix E_tensor = prod(trans(F),F);
    E_tensor -= IdentityMatrix(space_dimension, space_dimension);
    E_tensor *= 0.5;

    noalias(rStrainVector) = MathUtils<double>::StrainTensorToVector(E_tensor);
}

} // Namespace Kratos
