import numpy as np

#####################################################################
# All plots, models, config file will be stored here
OutputDirName = "./Results/Output_Merged2GsfID_dask_Fall17_EE_CheckALL_Binary"
Pickle_signal = "./data/Merged-ID/Dataframe_MergedID_HDalitz_Fall17_EE.pkl"
Pickle_bkg = "./data/Merged-ID/Dataframe_MergedID_DYJets_QCD_Fall17_EE.pkl"
Clfname = "MergedID"
#####################################################################
# prepare sample
testsize = 0.2  #(0.2 means 20%)
CommonCut = "(elePresel == 1) and (nVtx > 1) and (nGsfMatchToReco >= 2) and (Class != 'Merged-1Gsf')"
#####################################################################
Classes = ["Signal", "Background"]
ClassColors = ["#377eb8", "#E16262"]
MVA = "XGB"

featureplotparam_json = "FeaturePlotParam_MergedID_EE.json"
features = [
    "rho",
    "eleSCEta",
    "eleSCRawEn",

    "eledEtaAtVtx",
    "eledPhiAtVtx",
    "elePtError",
    "eleHoverE",
    "eleEoverP",
    "eleEoverPout",
    "eleEoverPInv",

    "eleSCEtaWidth",
    "eleSCPhiWidth",
    "eleSigmaIEtaIEtaFull5x5",
    "eleSigmaIPhiIPhiFull5x5",
    "eleR9Full5x5",
    "eleBrem",

    "elePFChIso",
    "elePFPhoIso",
    "elePFNeuIso",

    "gsfPtRatio",
    "gsfDeltaR",
    "gsfRelPtRatio"
]

RandomState = 42
param = {
    "objective": "binary:logistic",
    "eval_metric": "logloss",
    "tree_method": "gpu_hist",
    "random_state": RandomState,

    "learning_rate": 0.35,
    "max_delta_step": 40,
    "max_depth": 4,
    "min_child_weight": 1000,
    "min_split_loss": 20,
    "subsample": 0.8
}

# number of trees
num_boost_round = 500
early_stopping_rounds = 10
#####################################################################
# Reweighting scheme
# ! Notice that "intwei" should be included in the input pkl files
# * instwei: (relative xs) ggF as 1, for it being the production with the largest xs and VBF as xs_VBF/xs_ggF
# * Reweighing = Balanced | one of the class in "Classes"
Reweighing = "Signal"
ptbins = [0., 10., 20., 30., 45, 60., 80, 150, 20000]
etabins = [-2.5 , -2.25, -2., -1.75, -1.5, 1.5 ,1.75, 2., 2.25, 2.5]
ptwtvar = "eleCalibPt"
etawtvar = "eleSCEta"
