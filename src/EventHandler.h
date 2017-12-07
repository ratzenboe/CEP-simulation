/// ~~~{.cpp}

#ifndef EventHandler_H 
#define EventHandler_H 

/* #include <TSting> */
/* #include <TParticle> */
/* #include <TTree> */
/* #include <TFile> */

#include "ReadDirectoryTree.h"
#include "ReadPythiaTree.h"

class EventHandler : public ReadDirectoryTree, public ReadPythiaTree
{
        public:
                                EventHandler(TString inFilename, 
                                             TString outpath="/media/hdd/InvariantMass_output/");
                                ~EventHandler(void);
            /* void                CreateOutputObjects(Int_t maxEvts=-1); */
                                // mode 1 = KK, mode 0 = 2pi
            void                EventInitilizer(    Int_t mode=1, Int_t maxEvts=-1, 
                                                                  Bool_t saveEvtInfo=false);
            void                ParticleInitializer(Int_t mode=1, Int_t maxEvts=-1);
            TString             GetOutputPath(Int_t mode=0);
        private:
            // output checks: event listing saved in txt file 
            // in the output directory
            ofstream            fb;
            ostream             &os;
            // event specific contrainers
            TParticle*          fPart;              // multiple particles in the tree
            Pythia8::Pythia     fPythia;
            Pythia8::Event*     fPythiaEvent;       // pyhtia event
            TString             fOutpath;
            Bool_t              fKin;
            Bool_t              fPyt;
            TTree*              fEvtTree;
            Int_t               fNpiP, fNpiM, fNkaP, fNkaM, fNprotP, fNprotM;
            // output variables
            /* TFile*              o_fEvtFile; */
            /* TFile*              o_fPartFile; */
            /* TTree*              fOutTreeEvt; */
            /* TTree*              fOutTreePart; */
            // variables saved in the tree
            Double_t            fEta;                   
            Double_t            fPhi;                   
            Double_t            fPt;                   
            Double_t            fVx;                   
            Double_t            fVy;                   
            Double_t            fVz;                   
            Double_t            fInvarMass;                   
            Double_t            fRealInvarMass;                   
            Double_t            fDetGammaEt;                   
            Int_t               fPdg;
            Int_t               fEventNb;
            Int_t               fDiffrCode;
            Bool_t              fHasRightParticlesInTPCITS;
            Bool_t              fWholeEvtDetected;
            Bool_t              fHitInForwardDets;
            Bool_t              fHitInAD;
            Bool_t              fGammaInEMCal;                   
            // variables for tree size control
            Int_t               fLoopCounter;
            // private member functions
            // overloaded if other particles than 2pi are of interest
            void                fHasRightNumber(Int_t& Npi, Int_t& Nka, Int_t& Npro);
            void                fIsDetected( Double_t eta );
            Bool_t              setPDGval(void);
            Bool_t              EventLooper(Int_t maxEvts);
            void                AnalyseEvent(Int_t iEvent, TTree* tree, Int_t mode, 
                                                                        Bool_t saveEvtInfo=false);
            enum                fModes         {2pi=0, 2ka=1, pika=2, piPro=3, 2pro=4};
            enum                fParticleCodes {pion = 211, kaon = 321, proton = 2212};
};

#endif

