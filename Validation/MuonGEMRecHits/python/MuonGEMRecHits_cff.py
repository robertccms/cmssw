import FWCore.ParameterSet.Config as cms


gemRecHitsValidation = cms.EDAnalyzer('GEMRecHitsValidation',
    verboseSimHit = cms.untracked.int32(1),
    simInputLabel = cms.InputTag('g4SimHits',"MuonGEMHits"),
    recHitsInputLabel = cms.InputTag('gemRecHits'),
    # st1, st2_short, st2_long of xbin, st1,st2_short,st2_long of ybin
    nBinGlobalZR = cms.untracked.vdouble(200,200,200,150,180,250), 
    # st1 xmin, xmax, st2_short xmin, xmax, st2_long xmin, xmax, st1 ymin, ymax...
    RangeGlobalZR = cms.untracked.vdouble(564,572,786,794,786,802,110,260,170,350,100,350), 
    nBinGlobalXY = cms.untracked.int32(720), 
    detailPlot = cms.bool(False),
)

gemRecHitTrackValidation = cms.EDAnalyzer('GEMRecHitTrackMatch',
  simInputLabel = cms.untracked.string('g4SimHits'),
  simTrackCollection = cms.InputTag('g4SimHits'),
  simVertexCollection = cms.InputTag('g4SimHits'),
  verboseSimHit = cms.untracked.int32(0),
  # GEM RecHit matching:
  verboseGEMDigi = cms.untracked.int32(0),
  gemRecHitInput = cms.InputTag("gemRecHits"),
  minBXGEM = cms.untracked.int32(-1),
  maxBXGEM = cms.untracked.int32(1),
  matchDeltaStripGEM = cms.untracked.int32(1),
  gemMinPt = cms.untracked.double(5.0),
  gemMinEta = cms.untracked.double(1.55),
  gemMaxEta = cms.untracked.double(2.45),
  detailPlot = cms.bool(False),
)

gemLocalRecoValidation = cms.Sequence( gemRecHitsValidation+gemRecHitTrackValidation )
