/// ~~~{.cpp}
///  root > .x pythia8.C
/// ~~~
/// Note that before executing this script,
//,/
///  - the env variable PYTHIA8 must point to the pythia8100 (or newer) directory
///  - the env variable PYTHIA8DATA must be defined and it must point to $PYTHIA8/xmldoc
/// Note2: add this line to the rootlogon file. Otherwise the pythia library wont be loaded:
//- gInterpreter->AddIncludePath("/home/ratzenboe/alice/ali-master/AliRoot/PYTHIA8/pythia8/include")

#ifndef THistMaker_H
#define THistMaker_H

// in main a TList object is filled and saved in a .root file
class THistMaker
{
    public:
                            THistMaker(
                            TString inputFilePath="/media/hdd/InvariantMass_output/kaon_evt.root",
                            Int_t nBins=100, Double_t xlo=0.5, Double_t xhi=3.2);
                            ~THistMaker(void);
        void                SaveHistsInFile(Int_t mode=-1,
            TString outpath="/home/ratzenboe/Documents/Pythia-stuff/InvariantMass_study/HistSave/");
        void                Save2DMassHistInFile(
            TString outpath="/home/ratzenboe/Documents/Pythia-stuff/InvariantMass_study/HistSave/");
        void                SaveList(void);
    private: 
        TFile*              fFile;
        TTree*              fTree;
        // histograms
        // - full reconstructed events are the same for every detector arangement
        // -> therefore we only have to make detector individual hists for feed down
        TH1F*               fHist_CD;
        TH1F*               fHist_fromCEP;
        TH1F*               fHist_fromKStar;
        TH1F*               fHist_fromRho;
        // Feed-down histograms:
        TH1F*               fHist_TPC_feedDown;
        TH1F*               fHist_fwd_feedDown;
        TH1F*               fHist_ad_feedDown;
        TH1F*               fHist_emc_feedDown;
        // scaling factor of the histogram
        Int_t               fTPCITSsignalEnries;
        // mass comparison for the AD case
        // measured mass VS real mass
        TH2D*               fMassCompare;
        TString             fInFile;
        TString             fTitleSuffix;
        Bool_t              fShowCEPComponents;
        Int_t               fNbins;
        Double_t            fXhi;
        Double_t            fXlo;
        // Branches
        Int_t               fDiffrCode;
        Bool_t              fHitInForwardDets;
        Bool_t              fHitInAD;
        Int_t               fEventNb;
        Double_t            fInvarMass;
        Bool_t              fGammaInEMCal;
        Double_t            fRealInvarMass;
        Bool_t              fWholeEvtDetected;
        Bool_t              fHasRightParticlesInTPCITS;
        Int_t               fFromCEP;
        // histograms may be saved in a list
        TList*              fOutList;
        TFile*              fOutputFile;
};

#endif
