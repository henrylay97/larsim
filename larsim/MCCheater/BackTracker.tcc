///////////////////////////////////////////////////////////////////////////
// \author jason.stock@mines.sdsmt.edu
//
// Based on the original BackTracker by brebel@fnal.gov
//
////////////////////////////////////////////////////////////////////////////

#include "canvas/Persistency/Common/FindManyP.h"
#include "lardataobj/RecoBase/SpacePoint.h"

namespace cheat {

  //--------------------------------------------------------------------
  template <typename Evt> // DO NOT USE THIS FUNCTION FROM WITHIN ART! The
                          // BackTrackerService is designed to impliment these
                          // methods as cleanly as possible within the art
                          // framework. This is intended for gallery users.
  void
  BackTracker::PrepEvent(const Evt& evt)
  {
    if (!(this->CanRun(evt))) {
      throw cet::exception("BackTracker") << "BackTracker cannot function. "
                                          << "Is this file real data?";
    }
    fSimChannels.clear();
    this->PrepSimChannels(evt);
    //this->PrepAllHitList ( evt ); //This line temporarily commented out until I figure out how I want PrepAllHitList to work.
  }

  //--------------------------------------------------------------------
  template <typename Evt>
  void
  BackTracker::PrepSimChannels(const Evt& evt)
  {
    if (this->SimChannelsReady()) { return; }
    // The SimChannels list needs to be built.
    const auto& simChannelsHandle =
      evt.template getValidHandle<std::vector<sim::SimChannel>>(fSimChannelModuleLabel);

    art::fill_ptr_vector(fSimChannels, simChannelsHandle);

    auto comparesclambda = [](art::Ptr<sim::SimChannel> a, art::Ptr<sim::SimChannel> b) {
      return (a->Channel() < b->Channel());
    };
    if (!std::is_sorted(fSimChannels.begin(), fSimChannels.end(), comparesclambda))
      std::sort(fSimChannels.begin(), fSimChannels.end(), comparesclambda);
  }

  //--------------------------------------------------------------------
  template <typename Evt>
  void
  BackTracker::MakeRollupMap(const Evt& evt)
  {
    const auto& mcParticlesHandle =
      evt.template getValidHandle<std::vector<simb::MCParticle>>(fG4ModuleLabel);

    std::vector<art::Ptr<simb::MCParticle>> mcParticleVec;
    art::fill_ptr_vector(mcParticleVec, mcParticlesHandle);

    std::map<int,art::Ptr<simb::MCParticle>> trackIDToMCMap;

    for(auto const &mc : mcParticleVec)
      trackIDToMCMap[mc->TrackId()] = mc;

    for(auto &[trackId, mc] : trackIDToMCMap)
      {
	int origId = trackId;
	int id = trackId;
	
	if(mc->PdgCode() == 11 || mc->PdgCode() == 22)
	  {
	    while(trackIDToMCMap[mc->Mother()]->PdgCode() == 11 || trackIDToMCMap[mc->Mother()]->PdgCode() == 22)
	      {
		id = mc->Mother();
		mc = trackIDToMCMap[id];
	      }
	  }

        fRollupMap[origId] = id;
      }
  }

  //--------------------------------------------------------------------
  /*  template<typename Evt>
      void BackTracker::PrepAllHitList( const Evt& evt){
      if(this->AllHitListReady()){return;}
      const auto& allHitsHandle = evt.template
     getValidHandle<std::vector<recob::Hit>>(fHitLabel);
      art::fill_ptr_vector(fAllHitList, allHitsHandle);
      }
      */
  //--------------------------------------------------------------------
  template <typename Evt>
  std::vector<art::Ptr<recob::Hit>>
  BackTracker::SpacePointToHits_Ps(art::Ptr<recob::SpacePoint> const& spt, const Evt& evt) const
  {
    std::vector<art::Ptr<recob::SpacePoint>>
      spv; // This method needs to be rethought. For now I am directly
           // implimenting it as found in the previous backtracker.
    spv.push_back(spt);
    art::FindManyP<recob::Hit> fmh(spv, evt, fHitLabel);
    std::vector<art::Ptr<recob::Hit>> hitv = fmh.at(0);
    return hitv;
  }

  //--------------------------------------------------------------------
  template <typename Evt>
  std::vector<double>
  BackTracker::SpacePointToXYZ(detinfo::DetectorClocksData const& clockData,
                               art::Ptr<recob::SpacePoint> const& spt,
                               const Evt& evt) const
  {
    std::vector<art::Ptr<recob::Hit>> hits = this->SpacePointToHits_Ps(spt, evt);
    return this->SpacePointHitsToWeightedXYZ(clockData, hits);
  }

} // end namespace

// Local variables:
// mode: c++
// End:
