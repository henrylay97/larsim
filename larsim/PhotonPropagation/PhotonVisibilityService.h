////////////////////////////////////////////////////////////////////////
// \file PhotonVisibilityService.h
//
// \brief Service to report opdet visibility to different points in
//         the system
//
// \author bjpjones@mit.edu
//
////////////////////////////////////////////////////////////////////////
#ifndef PHOTONVISIBILITYSERVICE_H
#define PHOTONVISIBILITYSERVICE_H

#include "larcorealg/Geometry/geo_vectors_utils.h"          // geo::vect namespace
#include "larcoreobj/SimpleTypesAndConstants/geo_vectors.h" // geo::Point_t
#include "larsim/PhotonPropagation/IPhotonLibrary.h"
#include "larsim/PhotonPropagation/LibraryMappingTools/IPhotonMappingTransformations.h"
#include "larsim/PhotonPropagation/PhotonVisibilityTypes.h"
#include "larsim/Simulation/PhotonVoxels.h"

#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"

namespace fhicl {
  class ParameterSet;
}

// ROOT
class TF1;

// C/C++ standard libraries
#include <memory> // std::unique_ptr<>
#include <string>
#include <vector>

///General LArSoft Utilities
namespace phot {

  class PhotonVisibilityService {

    /// Type of optical library index.
    using LibraryIndex_t = phot::IPhotonMappingTransformations::LibraryIndex_t;

  public:
    ~PhotonVisibilityService();
    PhotonVisibilityService(fhicl::ParameterSet const& pset);

    void reconfigure(fhicl::ParameterSet const& p);

    double GetQuenchingFactor(double dQdx) const;

    template <typename Point>
    static double
    DistanceToOpDet(Point const& p, unsigned int OpDet)
    {
      return DistanceToOpDetImpl(geo::vect::toPoint(p), OpDet);
    }
    template <typename Point>
    static double
    SolidAngleFactor(Point const& p, unsigned int OpDet)
    {
      return SolidAngleFactorImpl(geo::vect::toPoint(p), OpDet);
    }

    template <typename Point>
    bool
    HasVisibility(Point const& p, bool wantReflected = false) const
    {
      return doHasVisibility(geo::vect::toPoint(p), wantReflected);
    }

    template <typename Point>
    float
    GetVisibility(Point const& p, unsigned int OpChannel, bool wantReflected = false) const
    {
      return doGetVisibility(geo::vect::toPoint(p), OpChannel, wantReflected);
    }

    template <typename Point>
    MappedCounts_t
    GetAllVisibilities(Point const& p, bool wantReflected = false) const
    {
      return doGetAllVisibilities(geo::vect::toPoint(p), wantReflected);
    }

    void LoadLibrary() const;
    void StoreLibrary();

    void StoreLightProd(int VoxID, double N);
    void RetrieveLightProd(int& VoxID, double& N) const;

    void SetLibraryEntry(int VoxID, OpDetID_t libOpChannel, float N, bool wantReflected = false);
    float GetLibraryEntry(int VoxID, OpDetID_t libOpChannel, bool wantReflected = false) const;
    bool HasLibraryEntries(int VoxID, bool wantReflected = false) const;
    phot::IPhotonLibrary::Counts_t GetLibraryEntries(int VoxID, bool wantReflected = false) const;

    template <typename Point>
    MappedT0s_t
    GetReflT0s(Point const& p) const
    {
      return doGetReflT0s(geo::vect::toPoint(p));
    }
    void SetLibraryReflT0Entry(int VoxID, int OpChannel, float value);
    phot::IPhotonLibrary::Counts_t GetLibraryReflT0Entries(int VoxID) const;
    float GetLibraryReflT0Entry(int VoxID, OpDetID_t libOpChannel) const;

    template <typename Point>
    MappedParams_t
    GetTimingPar(Point const& p) const
    {
      return doGetTimingPar(geo::vect::toPoint(p));
    }
    void SetLibraryTimingParEntry(int VoxID, int OpChannel, float value, size_t parnum);
    phot::IPhotonLibrary::Params_t GetLibraryTimingParEntries(int VoxID) const;
    float GetLibraryTimingParEntry(int VoxID, OpDetID_t libOpChannel, size_t npar) const;

    template <typename Point>
    MappedFunctions_t
    GetTimingTF1(Point const& p) const
    {
      return doGetTimingTF1(geo::vect::toPoint(p));
    }
    void SetLibraryTimingTF1Entry(int VoxID, int OpChannel, TF1 const& func);
    phot::IPhotonLibrary::Functions_t GetLibraryTimingTF1Entries(int VoxID) const;

    void SetDirectLightPropFunctions(TF1 const* functions[8],
                                     double& d_break,
                                     double& d_max,
                                     double& tf1_sampling_factor) const;
    void SetReflectedCOLightPropFunctions(TF1 const* functions[5],
                                          double& t0_max,
                                          double& t0_break_point) const;
    void LoadTimingsForVUVPar(std::vector<std::vector<double>> (&v)[7],
                              double& step_size,
                              double& max_d,
                              double& min_d,
                              double& vuv_vgroup_mean,
                              double& vuv_vgroup_max,
                              double& inflexion_point_distance,
                              double& angle_bin_timing_vuv) const;
    void LoadTimingsForVISPar(std::vector<double>& distances,
                              std::vector<double>& radial_distances,
                              std::vector<std::vector<std::vector<double>>>& cut_off,
                              std::vector<std::vector<std::vector<double>>>& tau,
                              double& vis_vmean,
                              double& angle_bin_timing_vis) const;
    void LoadVUVSemiAnalyticProperties ( bool &isFlatPDCorr,
                                         bool &isDomePDCorr,
                                         double &delta_angulo_vuv,
                                         double &radius) const;
    void LoadGHFlat( std::vector<std::vector<double>>  &GHvuvpars_flat,
                     std::vector<double> &border_corr_angulo_flat,
                     std::vector<std::vector<double>> &border_corr_flat) const;
    void LoadGHDome( std::vector<std::vector<double>>  &GHvuvpars_dome,
                     std::vector<double> &border_corr_angulo_dome,
                     std::vector<std::vector<double>> &border_corr_dome) const;
    void LoadVisSemiAnalyticProperties ( double &delta_angulo_vis,
                                         double &radius) const;
    void LoadVisParsFlat(std::vector<double> &vis_distances_x_flat,
                         std::vector<double> &vis_distances_r_flat,
                         std::vector<std::vector<std::vector<double>>> &vispars_flat) const;
    void LoadVisParsDome(std::vector<double> &vis_distances_x_dome,
                         std::vector<double> &vis_distances_r_dome,
                         std::vector<std::vector<std::vector<double>>> &vispars_dome) const;

    bool
    IsBuildJob() const
    {
      return fLibraryBuildJob;
    }
    bool
    UseParameterization() const
    {
      return fParameterization;
    }
    bool
    StoreReflected() const
    {
      return fStoreReflected;
    }
    bool
    StoreReflT0() const
    {
      return fStoreReflT0;
    }
    bool
    IncludeParPropTime() const
    {
      return fParPropTime;
    }
    size_t
    ParPropTimeNpar() const
    {
      return fParPropTime_npar;
    }
    std::string
    ParPropTimeFormula() const
    {
      return fParPropTime_formula;
    }

    bool
    IncludePropTime() const
    {
      return fIncludePropTime;
    }
    bool
    UseNhitsModel() const
    {
      return fUseNhitsModel;
    }
    bool
    ApplyVISBorderCorrection() const
    {
      return fApplyVISBorderCorrection;
    }
    std::string
    VISBorderCorrectionType() const
    {
      return fVISBorderCorrectionType;
    }

    const sim::PhotonVoxelDef&
    GetVoxelDef() const
    {
      return fVoxelDef;
    }
    size_t NOpChannels() const;

  private:
    int fCurrentVoxel;
    double fCurrentValue;
    // for c2: fCurrentReflValue is unused
    //double fCurrentReflValue;

    float fXmin, fXmax;
    float fYmin, fYmax;
    float fZmin, fZmax;
    int fNx, fNy, fNz;

    bool fUseCryoBoundary;

    bool fLibraryBuildJob;
    bool fDoNotLoadLibrary;
    bool fParameterization;
    bool fHybrid;
    bool fStoreReflected;
    bool fStoreReflT0;
    bool fIncludePropTime;
    bool fUseNhitsModel;
    bool fApplyVISBorderCorrection;
    std::string fVISBorderCorrectionType;

    bool fParPropTime;
    size_t fParPropTime_npar;
    std::string fParPropTime_formula;
    int fParPropTime_MaxRange;
    bool fInterpolate;
    bool fReflectOverZeroX;

    TF1* fparslogNorm = nullptr;
    TF1* fparslogNorm_far = nullptr;
    TF1* fparsMPV = nullptr;
    TF1* fparsMPV_far = nullptr;
    TF1* fparsWidth = nullptr;
    TF1* fparsCte = nullptr;
    TF1* fparsCte_far = nullptr;
    TF1* fparsSlope = nullptr;
    double fD_break, fD_max, fTF1_sampling_factor;
    TF1* fparslogNorm_refl = nullptr;
    TF1* fparsMPV_refl = nullptr;
    TF1* fparsWidth_refl = nullptr;
    TF1* fparsCte_refl = nullptr;
    TF1* fparsSlope_refl = nullptr;
    double fT0_max, fT0_break_point;

    //for vuv time parametrization
    std::vector<double> fDistances_landau;
    std::vector<std::vector<double>> fNorm_over_entries;
    std::vector<std::vector<double>> fMpv;
    std::vector<std::vector<double>> fWidth;
    std::vector<double> fDistances_exp;
    std::vector<std::vector<double>> fSlope;
    std::vector<std::vector<double>> fExpo_over_Landau_norm;
    double fstep_size, fmax_d, fmin_d, fvuv_vgroup_mean, fvuv_vgroup_max, finflexion_point_distance, fangle_bin_timing_vuv;
    // for vis time parameterisation (exists for SBND, DUNE-SP)
    std::vector<double> fDistances_refl;
    std::vector<double> fDistances_radial_refl;
    std::vector<std::vector<std::vector<double>>> fCut_off;
    std::vector<std::vector<std::vector<double>>> fTau;
    double fvis_vmean, fangle_bin_timing_vis;

    //for the semi-analytic vuv/direct light signal (number of hits) correction
    bool fIsFlatPDCorr, fIsDomePDCorr;
    //parametrization exists for DUNE-SP and for SBND
    double fdelta_angulo_vuv;
    // flat PDs
    std::vector<std::vector<double>> fGHvuvpars_flat;
    std::vector<double> fborder_corr_angulo_flat;
    std::vector<std::vector<double>> fborder_corr_flat;
    // dome PDs
    std::vector<std::vector<double>> fGHvuvpars_dome;
    std::vector<double> fborder_corr_angulo_dome;
    std::vector<std::vector<double>> fborder_corr_dome;
    // for the semi-analytic visible/reflection light hits correction
    // parameterization exists for DUNE-SP and SBND
    double fdelta_angulo_vis;
    // flat PDs
    std::vector<double> fvis_distances_x_flat;
    std::vector<double> fvis_distances_r_flat;
    std::vector<std::vector<std::vector<double>>> fvispars_flat;
    // dome PDs
    std::vector<double> fvis_distances_x_dome;
    std::vector<double> fvis_distances_r_dome;
    std::vector<std::vector<std::vector<double>>> fvispars_dome;

    // optical detector information, rest using geometry service
    double fradius;

    std::string fLibraryFile;
    mutable IPhotonLibrary* fTheLibrary;
    sim::PhotonVoxelDef fVoxelDef;

    /// Mapping of detector space into library space.
    std::unique_ptr<phot::IPhotonMappingTransformations> fMapping;

    geo::Point_t LibLocation(geo::Point_t const& p) const;

    int
    VoxelAt(geo::Point_t const& p) const
    {
      return fVoxelDef.GetVoxelID(LibLocation(p));
    }

    // same as `doGetVisibility()` but the channel number refers to the library
    // ID rather than to the actual optical detector ID.
    float doGetVisibilityOfOpLib(geo::Point_t const& p,
                                 LibraryIndex_t libIndex,
                                 bool wantReflected = false) const;

    // --- BEGIN Implementation functions --------------------------------------
    /// @name Implementation functions
    /// @{

    static double DistanceToOpDetImpl(geo::Point_t const& p, unsigned int OpDet);

    static double SolidAngleFactorImpl(geo::Point_t const& p, unsigned int OpDet);

    bool doHasVisibility(geo::Point_t const& p, bool wantReflected = false) const;

    float doGetVisibility(geo::Point_t const& p,
                          unsigned int OpChannel,
                          bool wantReflected = false) const;

    MappedCounts_t doGetAllVisibilities(geo::Point_t const& p, bool wantReflected = false) const;

    MappedT0s_t doGetReflT0s(geo::Point_t const& p) const;

    MappedParams_t doGetTimingPar(geo::Point_t const& p) const;

    MappedFunctions_t doGetTimingTF1(geo::Point_t const& p) const;

    /// @}
    // --- END Implementation functions ----------------------------------------

  }; // class PhotonVisibilityService
} //namespace phot
DECLARE_ART_SERVICE(phot::PhotonVisibilityService, LEGACY)
#endif // PHOTONVISIBILITYSERVICE_H
