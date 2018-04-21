import FWCore.ParameterSet.Config as cms

from RecoVertex.PrimaryVertexProducer.TkClusParameters_cff import DA_vectParameters

offlinePrimaryVertices = cms.EDProducer(
    "PrimaryVertexProducer",

    verbose = cms.untracked.bool(False),
    TrackLabel = cms.InputTag("generalTracks"),
    beamSpotLabel = cms.InputTag("offlineBeamSpot"),
    
    TkFilterParameters = cms.PSet(
        algorithm=cms.string('filter'),

#        maxNormalizedChi2 = cms.double(10.0),
#        minPixelLayersWithHits=cms.int32(2),
#        minSiliconLayersWithHits = cms.int32(5),
#        maxD0Significance = cms.double(4.0), 

# 2015 version
        maxNormalizedChi2 = cms.double(80.0),
        minPixelLayersWithHits=cms.int32(1),
        minSiliconLayersWithHits = cms.int32(3),
        maxD0Significance = cms.double(7.0), 
        minPt = cms.double(0.0),
#        maxEta = cms.double(2.4),
        maxEta = cms.double(2.5),
        trackQuality = cms.string("any")
    ),

    TkClusParameters = DA_vectParameters,

    vertexCollections = cms.VPSet(
     [cms.PSet(label=cms.string(""),
               algorithm=cms.string("AdaptiveVertexFitter"),
#               chi2cutoff = cms.double(2.5),
# RC, 3, hardcoded in 76X: https://github.com/cms-sw/cmssw/blob/CMSSW_7_6_X/RecoVertex/VertexTools/interface/GeometricAnnealing.h
# but 4 recommended
               chi2cutoff = cms.double(4.0),
# RC
#               minNdof=cms.double(0.0),
               minNdof=cms.double(-1.1),
               useBeamConstraint = cms.bool(False),
               maxDistanceToBeam = cms.double(1.0)
               ),
      cms.PSet(label=cms.string("WithBS"),
               algorithm = cms.string('AdaptiveVertexFitter'),
#               chi2cutoff = cms.double(2.5),
# but 4 recommended
               chi2cutoff = cms.double(4.0),
# RC
#               minNdof=cms.double(2.0),
               minNdof=cms.double(-2.0),
               useBeamConstraint = cms.bool(True),
               maxDistanceToBeam = cms.double(1.0)
               )
      ]
    )
                                        

                                        
)

# This customization is needed in the trackingLowPU era to be able to
# produce vertices also in the cases in which the pixel detector is
# not included in data-taking, like it was the case for "Quiet Beam"
# collisions on 2016 with run 269207.

from Configuration.Eras.Modifier_trackingLowPU_cff import trackingLowPU
trackingLowPU.toModify(offlinePrimaryVertices,
                            TkFilterParameters = dict(minPixelLayersWithHits = 0))


# higher eta cut for the phase 2 tracker
from Configuration.Eras.Modifier_phase2_tracker_cff import phase2_tracker 
phase2_tracker.toModify(offlinePrimaryVertices, 
                        TkFilterParameters = dict(maxEta = 4.0))

from Configuration.Eras.Modifier_pp_on_XeXe_2017_cff import pp_on_XeXe_2017
from Configuration.Eras.Modifier_pp_on_AA_2018_cff import pp_on_AA_2018
for e in [pp_on_XeXe_2017, pp_on_AA_2018]:
    e.toModify(offlinePrimaryVertices,
               TkFilterParameters = dict(maxD0Significance = 3.0),
               TkClusParameters = cms.PSet(
            algorithm = cms.string("gap"),
            TkGapClusParameters = cms.PSet(
                zSeparation = cms.double(1.0)        
                )
            )
               )
    
