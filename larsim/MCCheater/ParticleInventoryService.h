////////////////////////////////////////////////////////////////////////
// \file ParticleInventoryService.h
// \brief A service for managing the ParticleInventory when run in art.
//
// \author jason.stock@mines.sdsmt.edu
// Based on the original BackTracker by Brian Rebel (brebel@fnal.gov)
////////////////////////////////////////////////////////////////////////
#ifndef CHEAT_PARTICLEINVENTORYSERVICESERVICE_H
#define CHEAT_PARTICLEINVENTORYSERVICESERVICE_H

#include "larsim/MCCheater/ParticleInventory.h"

namespace sim {
  class EveIdCalculator;
  class ParticleList;
}

#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Persistency/Provenance/ScheduleContext.h"

namespace art {
  class ActivityRegistry;
  class Event;
}

#include "canvas/Persistency/Common/Ptr.h"
#include "fhiclcpp/types/Comment.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Table.h"

namespace fhicl {
  class ParameterSet;
}

#include <set>
#include <vector>

namespace cheat{
  class ParticleInventoryService: private ParticleInventory
  {
    public:

      struct ParticleInventoryServiceConfig{
        fhicl::Table<ParticleInventory::ParticleInventoryConfig> ParticleInventoryTable{
          fhicl::Name("ParticleInventory"),
          fhicl::Comment("This is the fhicl configuration for the ParticleInventory Service Provider") };
      };

      //attempting to be compliant with ServiceUtil.h. Should ask LArSoft expert to review.
      using provider_type = ParticleInventory;
      const provider_type* provider() const
      { return static_cast<const provider_type*>(this); }


      ParticleInventoryService(const ParticleInventoryServiceConfig& config, art::ActivityRegistry& reg);
      ParticleInventoryService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);

      //Move this function into the ParticleInventory.cpp file, and give it an appropriate CheckReady and Prep before the return.
      const sim::ParticleList& ParticleList() const;

      void Rebuild( const art::Event& evt );

      void SetEveIdCalculator(sim::EveIdCalculator *ec) { ParticleInventory::SetEveIdCalculator(ec); }

      //Does this make sense? A track Id to a single particle? This is not a one to one relationship.
      const simb::MCParticle* TrackIdToParticle_P(int id) const;
      simb::MCParticle        TrackIdToParticle(int const id) const
      { return *(this->TrackIdToParticle_P(id)); }//Users are encouraged to use TrackIdToParticleP

      const simb::MCParticle* TrackIdToMotherParticle_P(int id) const;
      simb::MCParticle        TrackIdToMotherParticle(int const id) const //Users are encouraged to use TrackIdToMotherParticleP
      { return *(this->TrackIdToMotherParticle_P(id)); }

      const art::Ptr<simb::MCTruth>& TrackIdToMCTruth_P(int id) const;
      simb::MCTruth                  TrackIdToMCTruth (int const id) const //Users are encouraged to use TrackIdToMCTruthP
      { return *(this->TrackIdToMCTruth_P(id)); }

      int TrackIdToEveTrackId(int tid) const;

      const art::Ptr<simb::MCTruth>& ParticleToMCTruth_P(const simb::MCParticle* p) const; //Users are encouraged to use ParticleToMCTruthP
      simb::MCTruth                  ParticleToMCTruth (const simb::MCParticle* p) const
      { return *(this->ParticleToMCTruth_P(p)); }

      const std::vector< art::Ptr<simb::MCTruth> >& MCTruthVector_Ps() const; //I don't want this to be able to return a vector of copies. Too much chance of significant memory usage.

      std::vector<const simb::MCParticle*> MCTruthToParticles_Ps(art::Ptr<simb::MCTruth> const& mct) const; //I don't want this to be able to return a vector of copies. Too much chance of significant memory usage.

      std::set<int> GetSetOfTrackIds() const;
      std::set<int> GetSetOfEveIds() const;



    private:

      void priv_PrepEvent        ( const art::Event& evt, art::ScheduleContext);
      void priv_PrepParticleList            ( const art::Event& evt);
      void priv_PrepMCTruthList             ( const art::Event& evt);
      void priv_PrepTrackIdToMCTruthIndex   ( const art::Event& evt);
      bool priv_CanRun(const art::Event& evt) const;

      bool priv_ParticleListReady()     { return  ParticleInventory::ParticleListReady(); }
      bool priv_MCTruthListReady()      { return  ParticleInventory::MCTruthListReady(); }
      bool priv_TrackIdToMCTruthReady() { return  ParticleInventory::TrackIdToMCTruthReady();}
  };//class ParticleInventoryService

}//namespace

DECLARE_ART_SERVICE(cheat::ParticleInventoryService, LEGACY)


#endif //CHEAT_PARTICLEINVENTORYSERVICESERVICE_H
