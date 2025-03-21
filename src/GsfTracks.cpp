#include "GsfTracks.h"
#include "Utilities.h"
#include <utility> // std::pair
#include <TString.h>


ROOT::RDF::RNode gsf::DefineGSFColumns(ROOT::RDF::RNode df){
    auto make_ditrkPt = [](const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk1, const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk2){
        ROOT::RVec<float> v(eleTrk1.size());
        for (size_t i = 0; i < eleTrk1.size(); i++){
            v[i] = (eleTrk1[i]+eleTrk2[i]).Pt();
        }
        return v;
    };

    // match the gsf tracks to the associated electron
    auto nf = df.Define("M_GSF",                "(float) 0.000511") // assume it is electron
                .Define("isMainGSF",            gsf::IsMainGSF,             {"event", "nGSFTrk", "gsfD0", "gsfDz", "nEle", "eleD0", "eleDz"})
                .Define("ambGSF",               gsf::TrkEleAssociation,     {"nGSFTrk", "gsfD0", "gsfDz", "nEle", "eleD0", "eleDz", "isMainGSF"})
                .Define("nGsfMatchToReco",      gsf::CalcNGsfMatchToReco,   {"nEle", "ambGSF"})
                .Define("eleTrkIdx",            gsf::FindMainGSF,           {"nEle", "ambGSF"})
                .Define("eleSubTrkIdx",         gsf::FindSubGSF_dRMin,      {"nEle", "ambGSF", "gsfEta", "gsfPhi", "gsfCharge"})

                .Define("eleTrkPt",             gsf::MatchIndexF,           {"nEle", "eleTrkIdx", "gsfPt"})
                .Define("eleTrkEta",            gsf::MatchIndexF,           {"nEle", "eleTrkIdx", "gsfEta"})
                .Define("eleTrkPhi",            gsf::MatchIndexF,           {"nEle", "eleTrkIdx", "gsfPhi"})
                .Define("eleTrkCharge",         gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfCharge"})
                .Define("eleTrkLayers",         gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfLayers"})
                .Define("eleTrkMissHits",       gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfMissHits"})
                .Define("eleTrkD0",             gsf::MatchIndexF,           {"nEle", "eleTrkIdx", "gsfD0"})
                .Define("eleTrkDz",             gsf::MatchIndexF,           {"nEle", "eleTrkIdx", "gsfDz"})
                .Define("eleTrkConvVeto",       gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfConvVeto"})
                .Define("eleTrkLostHits",       gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfLostHits"})
                .Define("eleTrkPixelHits",      gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfPixelHits"})

                .Define("eleSubTrkPt",          gsf::MatchIndexF,           {"nEle", "eleSubTrkIdx", "gsfPt"})
                .Define("eleSubTrkEta",         gsf::MatchIndexF,           {"nEle", "eleSubTrkIdx", "gsfEta"})
                .Define("eleSubTrkPhi",         gsf::MatchIndexF,           {"nEle", "eleSubTrkIdx", "gsfPhi"})
                .Define("eleSubTrkCharge",      gsf::MatchIndexI,           {"nEle", "eleSubTrkIdx", "gsfCharge"})
                .Define("eleSubTrkLayers",      gsf::MatchIndexI,           {"nEle", "eleSubTrkIdx", "gsfLayers"})
                .Define("eleSubTrkMissHits",    gsf::MatchIndexI,           {"nEle", "eleSubTrkIdx", "gsfMissHits"})
                .Define("eleSubTrkD0",          gsf::MatchIndexF,           {"nEle", "eleSubTrkIdx", "gsfD0"})
                .Define("eleSubTrkDz",          gsf::MatchIndexF,           {"nEle", "eleSubTrkIdx", "gsfDz"})
                .Define("eleSubTrkConvVeto",    gsf::MatchIndexI,           {"nEle", "eleTrkIdx", "gsfConvVeto"})
                .Define("eleSubTrkLostHits",    gsf::MatchIndexI,           {"nEle", "eleSubTrkIdx", "gsfLostHits"})
                .Define("eleSubTrkPixelHits",   gsf::MatchIndexI,           {"nEle", "eleSubTrkIdx", "gsfPixelHits"})

                .Define("eleTrk1",              utils::P4Vector,            {"eleTrkPt", "eleTrkEta", "eleTrkPhi", "M_GSF"})
                .Define("eleTrk2",              utils::P4Vector,            {"eleSubTrkPt", "eleSubTrkEta", "eleSubTrkPhi", "M_GSF"})
                .Define("diTrkPt",              make_ditrkPt,               {"eleTrk1", "eleTrk2"})
                .Define("gsfPtRatio",           gsf::GetTrkPtRatio,         {"nEle", "nGsfMatchToReco", "eleTrk1", "eleTrk2"})
                .Define("gsfDeltaR",            gsf::GetTrkdR,              {"nEle", "nGsfMatchToReco", "eleTrk1", "eleTrk2"})
                .Define("gsfPtSum",             gsf::GetTrkPtSum,           {"nEle", "nGsfMatchToReco", "eleTrk1", "eleTrk2"})
                .Define("gsfRelPtRatio",        gsf::GetTrkRelPtRatio,      {"nEle", "eleSCRawEn", "nGsfMatchToReco", "eleTrk1", "eleTrk2"});
    return nf;
}


ROOT::RVec<int> gsf::IsMainGSF(
    const Long64_t event,
    const int nGSFTrk,
    const ROOT::RVec<float>& gsfD0,
    const ROOT::RVec<float>& gsfDz,
    const int nEle,
    const ROOT::RVec<float>& eleD0,
    const ROOT::RVec<float>& eleDz
){
    ROOT::RVec<int> v(nGSFTrk);
    for (int i = 0; i < nGSFTrk; i++){
        // if the GSF track with D0, Dz the same as electron,
        // then this GSF track is the main GSF track of the electron
        int isMatched = 0;
        for (int j = 0; j < nEle; j++){
            if ((eleD0[j] == gsfD0[i]) && (eleDz[j] == gsfDz[i])){
                isMatched = 1;
                break;
            }
        }
        v[i] = isMatched;
    }

    if (v[0] != 1)
        throw std::runtime_error(Form("First gsf track should be main gsf track! (event: %lld)", event));

    const int nMainGSF = ROOT::VecOps::Sum(v);
    if (nMainGSF != nEle)
        throw std::runtime_error(Form("Number of main gsf tracks is not the same as number of electron! (event: %lld)", event));

    return v;
}


ROOT::RVec<ROOT::RVec<int>> gsf::TrkEleAssociation(
    const int nGSFTrk,
    const ROOT::RVec<float>& gsfD0,
    const ROOT::RVec<float>& gsfDz,
    const int nEle,
    const ROOT::RVec<float>& eleD0,
    const ROOT::RVec<float>& eleDz,
    const ROOT::RVec<int>& isMainGSF // branch added by the gsf::IsMainGSF
){
    ROOT::RVec<ROOT::RVec<int>> v_associateIdx;  // v_associateIdx[i_ele][i_gsf]
    v_associateIdx.clear();

    const ROOT::RVec<int> mainGsfIdx = Nonzero(isMainGSF);
    for (int i = 0; i < nEle; i++){
        const float mainGSFD0 = eleD0[i];
        const float mainGSFDz = eleDz[i];
        std::pair<int, int> results(-1, -1);
        for (size_t j = 0; j < mainGsfIdx.size(); j++){
            if (gsfD0[mainGsfIdx[j]] == mainGSFD0 && gsfDz[mainGsfIdx[j]] == mainGSFDz){
                results.first = j;
                results.second = mainGsfIdx[j];
                break;
            }
        }

        // the first element is always the index of main gsf track
        ROOT::RVec<int> associateIdx; // the indecies of gsf tracks associated with the electron.
        associateIdx.clear();
        if (results.second == mainGsfIdx.back()){ // the last main gsf track in this event
            for (int k = results.second; k < nGSFTrk; k++){
                associateIdx.push_back(k);
            }
        }
        else{
            for (int k = results.second; k < mainGsfIdx[results.first + 1]; k++){
                associateIdx.push_back(k);
            }
        }

        v_associateIdx.push_back(associateIdx);
    }

    return v_associateIdx;
}


ROOT::RVec<int> gsf::CalcNGsfMatchToReco(
    const int nEle,
    const ROOT::RVec<ROOT::RVec<int>>& ambGSF // branch added by the gsf::TrkEleAssociation
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        v[i] = ambGSF[i].size();
    }
    return v;
}


ROOT::RVec<int> gsf::FindMainGSF(
    const int nEle,
    const ROOT::RVec<ROOT::RVec<int>>& ambGSF // branch added by the gsf::TrkEleAssociation
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        v[i] = ambGSF[i][0];
    }
    return v;
}


ROOT::RVec<int> gsf::FindSubGSF_PtMax(
    const int nEle,
    const ROOT::RVec<ROOT::RVec<int>>& ambGSF, // branch added by the gsf::TrkEleAssociation
    const ROOT::RVec<float>& gsfPt,
    const ROOT::RVec<int>& gsfCharge
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        const int nGsfMatchToReco = ambGSF[i].size();
        float tmp = -999.;
        int eleSubTrkIdx = -1;
        for (int j = 1; j < nGsfMatchToReco; j++){// start from 1, beacause 0 is main gsf track
            if (gsfPt[ambGSF[i][j]] > tmp){
                tmp = gsfPt[ambGSF[i][j]];
                eleSubTrkIdx = ambGSF[i][j];
            }
        }
        v[i] = eleSubTrkIdx;
    }

    return v;
}


ROOT::RVec<int> gsf::FindSubGSF_dRMin(
    const int nEle,
    const ROOT::RVec<ROOT::RVec<int>>& ambGSF, // branch added by the gsf::TrkEleAssociation
    const ROOT::RVec<float>& gsfEta,
    const ROOT::RVec<float>& gsfPhi,
    const ROOT::RVec<int>& gsfCharge
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        const int nGsfMatchToReco = ambGSF[i].size();
        const int eleTrkIdx = ambGSF[i][0];
        float tmp = 999.;
        int eleSubTrkIdx = -1;
        for (int j = 1; j < nGsfMatchToReco; j++){ // j = 0 is main gsf track
            const float dR = ROOT::VecOps::DeltaR(gsfEta[eleTrkIdx], gsfEta[ambGSF[i][j]], gsfPhi[eleTrkIdx], gsfPhi[ambGSF[i][j]]);
            if (dR < tmp){
                tmp = dR;
                eleSubTrkIdx = ambGSF[i][j];
            }
        }
        v[i] = eleSubTrkIdx;
    }

    return v;
}


ROOT::RVec<int> gsf::FindSubGSF_dRMinWithCuts(
    const int nEle,
    const ROOT::RVec<float>& eleSCEta,
    const ROOT::RVec<ROOT::RVec<int>>& ambGSF, // branch added by the gsf::TrkEleAssociation
    const ROOT::RVec<float>& gsfEta,
    const ROOT::RVec<float>& gsfPhi,
    const ROOT::RVec<int>& gsfCharge,
    const ROOT::RVec<float>& gsfD0,
    const ROOT::RVec<float>& gsfDz,
    const ROOT::RVec<int>& gsfMissHits
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        const int nGsfMatchToReco = ambGSF[i].size();
        const int eleTrkIdx = ambGSF[i][0];
        float tmp = 999.;
        int eleSubTrkIdx = -1;
        bool isEB = fabs(eleSCEta[i] < 1.479);
        for (int j = 1; j < nGsfMatchToReco; j++){ // j = 0 is main gsf track
            
            if (gsfCharge[eleTrkIdx] * gsfCharge[ambGSF[i][j]] > 0)
                continue;
            
            bool track_from_pv = (isEB && fabs(gsfD0[ambGSF[i][j]]) < 0.02 && fabs(gsfDz[ambGSF[i][j]]) < 0.1) || (!isEB && fabs(gsfD0[ambGSF[i][j]]) < 0.05 && fabs(gsfDz[ambGSF[i][j]]) < 0.2);
            bool track_no_conv = gsfMissHits[ambGSF[i][j]] <= 0;
            if (!track_from_pv || !track_no_conv)
                continue;
            
            const float dR = ROOT::VecOps::DeltaR(gsfEta[eleTrkIdx], gsfEta[ambGSF[i][j]], gsfPhi[eleTrkIdx], gsfPhi[ambGSF[i][j]]);
            if (dR < tmp){
                tmp = dR;
                eleSubTrkIdx = ambGSF[i][j];
            }
        }
        v[i] = eleSubTrkIdx;
    }

    return v;
}


ROOT::RVec<float> gsf::MatchIndexF(
    const int nEle,
    const ROOT::RVec<int>& eleTrkIdx, // branch added by the gsf::FindMainGSF or gsf::FindSubGSF_dRMin
    const ROOT::RVec<float>& var
){
    ROOT::RVec<float> v(nEle);
    for (int i = 0; i < nEle; i++){
        v[i] = var[eleTrkIdx[i]];
    }
    return v;
}


ROOT::RVec<int> gsf::MatchIndexI(
    const int nEle,
    const ROOT::RVec<int>& eleTrkIdx, // branch added by the gsf::FindMainGSF or gsf::FindSubGSF_dRMin
    const ROOT::RVec<int>& var
){
    ROOT::RVec<int> v(nEle);
    for (int i = 0; i < nEle; i++){
        v[i] = var[eleTrkIdx[i]];
    }
    return v;
}


ROOT::RVec<float> gsf::GetTrkPtRatio(
    const int nEle,
    const ROOT::RVec<int>& nGsfMatchToReco, // branch added by the gsf::CalcNGsfMatchToReco
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk1,
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk2
){
    ROOT::RVec<float> v(nEle);
    for (int i = 0; i < nEle; i++){
        float ptratio = -999;
        if (nGsfMatchToReco[i] > 1)
            ptratio = eleTrk2[i].Pt()/eleTrk1[i].Pt();
        v[i] = ptratio;
    }
    return v;
}


ROOT::RVec<float> gsf::GetTrkdR(
    const int nEle,
    const ROOT::RVec<int>& nGsfMatchToReco, // branch added by the gsf::CalcNGsfMatchToReco
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk1,
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk2
){
    ROOT::RVec<float> v(nEle);
    for (int i = 0; i < nEle; i++){
        float dr = -999.;
        if (nGsfMatchToReco[i] > 1)
            dr = ROOT::VecOps::DeltaR(eleTrk1[i].Eta(), eleTrk2[i].Eta(), eleTrk1[i].Phi(), eleTrk2[i].Phi());
        v[i] = dr;
    }
    return v;
}


ROOT::RVec<float> gsf::GetTrkPtSum(
    const int nEle,
    const ROOT::RVec<int>& nGsfMatchToReco, // branch added by the gsf::CalcNGsfMatchToReco
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk1,
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk2
){
    ROOT::RVec<float> v(nEle);
    for (int i = 0; i < nEle; i++){
        float ptsum = -999.;
        if (nGsfMatchToReco[i] > 1)
            ptsum = eleTrk1[i].Pt() + eleTrk2[i].Pt();
        v[i] = ptsum;
    }
    return v;
}


ROOT::RVec<float> gsf::GetTrkRelPtRatio(
    const int nEle,
    const ROOT::RVec<float>& eleSCRawEn,
    const ROOT::RVec<int>& nGsfMatchToReco,
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk1,
    const ROOT::RVec<ROOT::Math::PtEtaPhiMVector>& eleTrk2
){
    ROOT::RVec<float> v(nEle);
    for (int i = 0; i < nEle; i++){
        float relptsum = -999.;
        if (nGsfMatchToReco[i] == 1)
            relptsum = eleTrk1[i].Pt()/eleSCRawEn[i];
        if (nGsfMatchToReco[i] > 1)
            relptsum = (eleTrk1[i] + eleTrk2[i]).Pt()/eleSCRawEn[i];
        v[i] = relptsum;
    }
    return v;
}


ROOT::RVec<int> gsf::FindBC(
    const ROOT::RVec<int> eleTrkIdx,
    const ROOT::RVec<float>& gsfEta,
    const ROOT::RVec<float>& gsfPhi,
    const int nBC,
    const ROOT::RVec<float>& bcEta,
    const ROOT::RVec<float>& bcPhi
){
    ROOT::RVec<int> v;
    v.clear();
    for (size_t i = 0; i < eleTrkIdx.size(); i++){
        float tmp = 999.;
        int eleBCIdx = -1;
        
        if (eleTrkIdx[i] != -1){
            for (int j = 0; j < nBC; j++){
                // find the cloest bc to the gsf track
                const float dR = ROOT::VecOps::DeltaR(bcEta[j], gsfEta[eleTrkIdx[i]], bcPhi[j], gsfPhi[eleTrkIdx[i]]);
                if (dR < tmp){
                    tmp = dR;
                    eleBCIdx = j;
                }
            }
            if (tmp > 0.1)
                eleBCIdx = -1;
        }

        v.push_back(eleBCIdx);
    }

    return v;
}


ROOT::RVec<int> gsf::FindSubBC(
    const ROOT::RVec<int>& eleSubTrkIdx,
    const ROOT::RVec<float>& gsfEta,
    const ROOT::RVec<float>& gsfPhi,
    const int nBC,
    const ROOT::RVec<float>& bcEta,
    const ROOT::RVec<float>& bcPhi,
    const ROOT::RVec<int>& eleBCIdx
){
    ROOT::RVec<int> v;
    v.clear();
    for (size_t i = 0; i < eleSubTrkIdx.size(); i++){
        float tmp = 999.;
        int eleSubBC = -1;
        
        if (eleSubTrkIdx[i] != -1){
            for (int j = 0; j < nBC; j++){
                // bc idx for main gsf track
                if (j == eleBCIdx[i])
                    continue;

                // find the cloest bc to the gsf track
                const float dR = ROOT::VecOps::DeltaR(bcEta[j], gsfEta[eleSubTrkIdx[i]], bcPhi[j], gsfPhi[eleSubTrkIdx[i]]);
                if (dR < tmp){
                    tmp = dR;
                    eleSubBC = j;
                }
            }
            if (tmp > 0.1)
                eleSubBC = -1;
        }

        v.push_back(eleSubBC);
    }

    return v;
}
