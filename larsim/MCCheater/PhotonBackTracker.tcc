#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace cheat{


  //----------------------------------------------------------------
  template<typename Evt>
    const bool PhotonBackTracker::CanRun(Evt const& evt) {
      return ! ( evt.isRealData() ) ;
    }


  //----------------------------------------------------------------
  template<typename Evt>
    void PhotonBackTracker::PrepOpDetBTRs(Evt const& evt)
    {
      if(this->BTRsReady()){ return;}
      const std::vector<art::InputTag> G4ModuleLabels = (fG4ModuleLabels.empty()) ?
        std::vector{fG4ModuleLabel} : fG4ModuleLabels;

      auto compareBTRlambda = [](art::Ptr<sim::OpDetBacktrackerRecord> a,
                                 art::Ptr<sim::OpDetBacktrackerRecord> b)
                                {return(a->OpDetNum()<b->OpDetNum());};

      for (auto& G4ModuleLabel : G4ModuleLabels) {
        auto const& btrHandle = evt.template getValidHandle < std::vector < sim::OpDetBacktrackerRecord > > (G4ModuleLabel);
        art::fill_ptr_vector(priv_OpDetBTRs, btrHandle);
        if (!std::is_sorted(priv_OpDetBTRs.begin(), priv_OpDetBTRs.end(), compareBTRlambda))
          std::sort(priv_OpDetBTRs.begin(),priv_OpDetBTRs.end(),compareBTRlambda);

        // // // // // // // // // // // // // // // // // // // // // // // //
        // DUNE-specific code which hasn't been migrated anywhere better yet //
        // // // // // // // // // // // // // // // // // // // // // // // //
        //art::fill_ptr_vector(priv_DivRecs, divrecHandle);
        //auto compareDivReclambda = [](art::Ptr<sim::OpDetDivRec> a, art::Ptr<sim::OpDetDivRec> b) {return(a->OpDetNum() < b->OpDetNum());};
        /*if (!std::is_sorted(priv_DivRecs.begin(), priv_DivRecs.end(), compareDivReclambda))
          std::sort(priv_DivRecs.begin(), priv_DivRecs.end(), compareDivReclambda);*/
        //art::FindManyP<raw::OpDetWaveform, sim::OpDetDivRec> fp(priv_OpDetBTRs, evt, fWavLabel);// fp;
        //art::FindOneP<raw::OpDetWaveform, sim::OpDetDivRec> fp(priv_OpDetBTRs, evt, fWavLabel);// fp;
        //They come in sorted by BTR. Now make an index matched vector of data_t sorted by BTR. No. I need easy, not efficient. Map of DetNum to data_t. data_t is then channel mapped.
        /*
          if (fp.isValid()){
          for( size_t btr_iter=0; btr_iter<priv_OpDetBTRs.size(); ++btr_iter){
          auto btr=priv_OpDetBTRs.at(btr_iter);
          auto od = btr->OpDetNum();
          auto const& dr = fp.data(btr_iter);
          for(auto& d : dr)
          {
          if(!d) continue;
          priv_od_to_DivRec[od]=*d;//->ref();
          }

          }
          }else{throw cet::exception("PhotonBackTracker")<<"find Waveforms and DivRecs from BTRs failed.";}
        */
        // // // // // // // // // // // // // // // // // // // // // // // //
        // DUNE-specific code which hasn't been migrated anywhere better yet //
        // // // // // // // // // // // // // // // // // // // // // // // //
      }
      return;
    }

    //----------------------------------------------------------------
    //ToDo: Figure out why I get OpHit* out of here instead of art::Ptr.
    template<typename Evt>
      void PhotonBackTracker::PrepOpFlashToOpHits( Evt const& evt)
      {
        if(this->OpFlashToOpHitsReady()){ return;}
        //std::vector< art::Handle< std::vector < recob::OpFlash >>> flashHandles;
        //evt.getManyByType(flashHandles);
        auto flashHandles = evt.template getMany<std::vector<recob::OpFlash>>();
        for( const auto& handle : flashHandles)
        {
          std::vector< art::Ptr < recob::OpFlash > > flash_vec;
          if(handle.failedToGet())
          {
            mf::LogWarning("PhotonBackTracker")<<" failed to get handle to recob::OpFlash. Has reco run yet?";
            return;
          }
          art::fill_ptr_vector(flash_vec, handle);
          auto tag = art::InputTag( handle.provenance()->moduleLabel() );
          art::FindManyP<recob::OpHit>  flash_hit_assn(flash_vec, evt, tag);
          //          std::cout<<"flash_hit_assn.size: "<<flash_hit_assn.size()<<"\n";
          for ( size_t i = 0; i < flash_vec.size(); ++i)
          {
            art::Ptr< recob::OpFlash > flashp = flash_vec.at(i);
            std::vector< art::Ptr< recob::OpHit > > ophits = flash_hit_assn.at(i);
            auto check = priv_OpFlashToOpHits.emplace(flashp, ophits);
            if ( check.second == false )
            {
              // loop ophit_ps
              // push_back to vector.
              for ( auto& ophitp : ophits )
              {
                check.first->second.push_back(ophitp);
              }
            }
          }
        }
      }

    //----------------------------------------------------------------
    template<typename Evt>
      void PhotonBackTracker::PrepEvent( Evt const& evt)
      {
        if( !(this->CanRun( evt ) ) ){
          throw cet::exception("PhotonBackTracker")
            <<"PhotonBackTracker cannot function."
            <<"Is this file real data?";
        }
        priv_OpDetBTRs.clear();
        this->PrepOpDetBTRs(evt);
        this->PrepOpFlashToOpHits(evt);
      }
    }
