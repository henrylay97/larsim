////////////////////////////////////////////////////////////////////////
/// \file  ParticleListAction.h
/// \brief Use Geant4's user "hooks" to maintain a list of particles generated by Geant4.
///
/// \author  seligman@nevis.columbia.edu
////////////////////////////////////////////////////////////////////////

/// This class implements the LArG4::UserAction interface in order to
/// accumulate a list of particles modeled by Geant4.
//
/// It uses multiple inheritance: it inherits from LArG4::UserAction,
/// in order to take advantage of Geant4's user hooks; it also
/// inherits from cfg::Observer, because it accesses a parameter from
/// an XML configuration file.

#ifndef LArG4_ParticleListAction_h
#define LArG4_ParticleListAction_h

#include "Geant4/G4Types.hh"
#include "TLorentzVector.h"

#include "larcorealg/CoreUtils/ParticleFilters.h" // util::PositionInVolumeFilter
#include "nusimdata/SimulationBase/simb.h" // simb::GeneratedParticleIndex_t
#include "nug4/G4Base/UserAction.h"
#include "cetlib/exempt_ptr.h"

#include <map>
#include <memory>

// Forward declarations.
class G4Event;
class G4Track;
class G4Step;

namespace sim {
  class ParticleList;
}

namespace simb {
  class MCParticle;
}

namespace larg4 {

  class ParticleListAction : public g4b::UserAction
  {
  public:
    using GeneratedParticleIndex_t = simb::GeneratedParticleIndex_t;

    struct ParticleInfo_t {

      cet::exempt_ptr<simb::MCParticle> particle;  ///< Object representing particle.
      bool              keep = false;        ///< if there was decision to keep
      /// Index of the particle in the original generator truth record.
      GeneratedParticleIndex_t truthIndex = simb::NoGeneratedParticleIndex;

      /// Resets the information (does not release memory it does not own)
      void clear()
        {
          particle = nullptr;
          keep = false;
          truthIndex = simb::NoGeneratedParticleIndex;
        }

      /// Returns whether there is a particle
      bool hasParticle() const { return !particle.empty(); }

      /// Returns whether there is a particle
      bool isPrimary() const { return simb::isGeneratedParticleIndex(truthIndex); }

      /// Rerturns whether there is a particle known to be kept
      bool keepParticle() const { return hasParticle() && keep; }

      /// Returns the index of the particle in the generator truth record.
      GeneratedParticleIndex_t truthInfoIndex() const { return truthIndex; }

    }; // ParticleInfo_t

    // Standard constructors and destructors;
    ParticleListAction(double energyCut,
                       bool storeTrajectories=false,
                       bool keepEMShowerDaughters=false,
                       bool keepMCParticleList=true
                       );

    // UserActions method that we'll override, to obtain access to
    // Geant4's particle tracks and trajectories.
    virtual void             BeginOfEventAction(const G4Event*);
    virtual void             EndOfEventAction  (const G4Event*);
    virtual void             PreTrackingAction (const G4Track*);
    virtual void             PostTrackingAction(const G4Track*);
    virtual void             SteppingAction    (const G4Step* );

    /// Grabs a particle filter
    void ParticleFilter(std::unique_ptr<util::PositionInVolumeFilter>&& filter)
      { fFilter = std::move(filter); }


    // TrackID of the current particle, EveID if the particle is from an EM shower
    static int               GetCurrentTrackID() { return fCurrentTrackID; }
    static int               GetCurrentPdgCode() { return fCurrentPdgCode; }

    void                     ResetTrackIDOffset() { fTrackIDOffset = 0;     }

    // Returns the ParticleList accumulated during the current event.
    const sim::ParticleList* GetList() const;

    /// Returns a map of truth record information index for each of the primary
    /// particles (by track ID).
    std::map<int, GeneratedParticleIndex_t> const& GetPrimaryTruthMap() const
      { return fPrimaryTruthMap; }

    /// Returns whether a particle list is being kept.
    bool hasList() const { return static_cast<bool>(fparticleList); }

    /// Returns the index of primary truth (`sim::NoGeneratorIndex` if none).
    GeneratedParticleIndex_t GetPrimaryTruthIndex(int trackId) const;

    // Yields the ParticleList accumulated during the current event.
    sim::ParticleList&& YieldList();

    /// returns whether the specified particle has been marked as dropped
    static bool isDropped(simb::MCParticle const* p);

  private:

    // this method will loop over the fParentIDMap to get the
    // parentage of the provided trackid
    int                      GetParentage(int trackid) const;

    G4double                 fenergyCut;             ///< The minimum energy for a particle to
                                                     ///< be included in the list.
    ParticleInfo_t           fCurrentParticle;       ///< information about the particle currently being simulated
                                                     ///< for a single particle.
    std::unique_ptr<sim::ParticleList> fparticleList; ///< The accumulated particle information for
                                                     ///< all particles in the event.
    G4bool                   fstoreTrajectories;     ///< Whether to store particle trajectories with each particle.
    std::map<int, int>       fParentIDMap;           ///< key is current track ID, value is parent ID
    static int               fCurrentTrackID;        ///< track ID of the current particle, set to eve ID
                                                     ///< for EM shower particles
    static int               fCurrentPdgCode;        ///< pdg code of current particle
    static int               fTrackIDOffset;         ///< offset added to track ids when running over
                                                     ///< multiple MCTruth objects.
    bool                     fKeepEMShowerDaughters; ///< whether to keep EM shower secondaries, tertiaries, etc

    std::unique_ptr<util::PositionInVolumeFilter> fFilter; ///< filter for particles to be kept

    /// Map: particle track ID -> index of primary information in MC truth.
    std::map<int, GeneratedParticleIndex_t> fPrimaryTruthMap;

    /// Adds a trajectory point to the current particle, and runs the filter
    void AddPointToCurrentParticle(TLorentzVector const& pos,
                                   TLorentzVector const& mom,
                                   std::string    const& process);

  };

} // namespace LArG4

#endif // LArG4_ParticleListAction_h
