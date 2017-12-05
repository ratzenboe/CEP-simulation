/// EventHandler.cxx
///


/* #define PRINT_DEBUG */

#ifdef PRINT_DEBUG
void PrintDebug(const char *message){
    printf("%s \ni", message);
}
#else
#define PrintDebug(...) 0;
#endif

#include <iostream>
#include <fstream>
#include <ostream>
#include "Pythia8/Pythia.h"
#include "TPythia8.h"
#include <TROOT.h>
#include <TParticle.h>
#include <TLorentzVector.h>
#include <TSystem.h>
#include <TTree.h>
#include <TFile.h>
#include <TMath.h>
#include <TDatabasePDG.h>
#include "EventHandler.h"
#include "ReadDirectoryTree.h"
#include "ReadPythiaTree.h"

class EventHandler;         // calss name

using namespace std;

ClassImp(EventHandler);     // necessary for root
// afterwards we write a second constructor which takes inherits from a 
// class reading pure pyhtia root files (with pyhtia event objects inside)
EventHandler::EventHandler(TString inFilename, TString outpath) :
    ReadDirectoryTree(inFilename), ReadPythiaTree(inFilename),
    // initialize the outstream object
    fb((outpath+"eventInfo.txt").Data()), os(fb),
    // Particles from kinematics file, Pythia event from Pythia file
    fPart(0), fPythiaEvent(0), fOutpath(outpath),
    // initialisation of all private mem-vars
    /* fEvtTree(0), */ 
    /* o_fEvtFile(0), o_fPartFile(0), fOutTreeEvt(0), fOutTreePart(0), */
    fNpiP(0), fNpiM(0), fNkaP(0), fNkaM(0),
    // output to particle file
    fEta(0), fPhi(0), fPt(0), fVx(0), fVy(0), fVz(0), fPdg(0), fEventNb(0), fDiffrCode(-1),
    // output to event file (also with event number and diffractive code)
    fHasRightParticlesInTPCITS(false), fWholeEvtDetected(false), fHitInForwardDets(false), 
    fInvarMass(0), fHitInAD(0), fDetGammaEt(0), fGammaInEMCal(false),
    fLoopCounter(0), fKin(false), fPyt(false)
{
    // constructor
    fPythiaEvent = &fPythia.event;
    fPart = new TParticle();

    if (KinematicsExists())  fKin = true;
    else if (PythiaExists()){
        cout << "Detected Pythia Tree" << endl;
        fPyt = true;
    }
}
//_____________________________________________________________________________
EventHandler::~EventHandler(void)
{
    // destructor
    // fist we close fb
    fb.close();
    if (fPart) delete fPart;
    if (fEvtTree) delete fEvtTree;
}
//_____________________________________________________________________________
bool EventHandler::EventLooper(Int_t maxEvts)
{
    // NextEvent & GetEventTree from ReadDirectoryTree
    if (fKin){
        if (NextEvtKinematic() && fLoopCounter < abs(maxEvts)) {
            fEvtTree = GetEventTree(); 
            fEvtTree->SetBranchAddress("Particles", &fPart);
            if (maxEvts != -1) fLoopCounter++;
            return true;
        } else return false;
    } else if (fPyt) {
        if (NextEvt() && fLoopCounter < abs(maxEvts)){
            fPythiaEvent = GetEvent(); 
            if (maxEvts != -1) fLoopCounter++;
            return true;
        } else return false;
    } else return false;
}
//_____________________________________________________________________________
void EventHandler::ParticleInitializer(Int_t mode, Int_t maxEvts)
{
    TString op = fOutpath;
    if (mode==0) op += "pi_";
    if (mode==1) op += "kaon_";
    TFile* o_fPartFile  = new TFile((op+"part.root").Data(),  "RECREATE");
    TTree* fOutTreePart = new TTree("particles", "part tree");

    fOutTreePart->Branch("fEta",       &fEta);
    fOutTreePart->Branch("fPhi",       &fPhi);
    fOutTreePart->Branch("fPt",        &fPt);
    fOutTreePart->Branch("fVx",        &fVx);
    fOutTreePart->Branch("fVy",        &fVy);
    fOutTreePart->Branch("fVz",        &fVz);
    fOutTreePart->Branch("fPdg",       &fPdg);
    fOutTreePart->Branch("fEventNb",   &fEventNb);
    fOutTreePart->Branch("fDiffrCode", &fDiffrCode);

    Int_t iEvent = 0;
    if (fPyt) SetIEntryPythia(0);
    if (fKin) SetIEntryKin(0);
     while(EventLooper(maxEvts)){
        AnalyseEvent(iEvent, fOutTreePart, mode);
        iEvent++;
    }
    o_fPartFile->cd();
    fOutTreePart->Write();
    o_fPartFile->Close();
    delete o_fPartFile;
}
//_____________________________________________________________________________
// mode determines what particles are searched for (pipi=0, KK=1)
void EventHandler::EventInitilizer(Int_t mode, Int_t maxEvts, Bool_t saveEvtInfo)
{
    TString op = fOutpath;
    if (mode==0) op += "pi_";
    if (mode==1) op += "kaon_";    
    TFile* o_fEvtFile  = new TFile((op+"evt.root").Data(),  "RECREATE");
    TTree* fOutTreeEvt  = new TTree("event", "event tree");

    fOutTreeEvt->Branch("fDiffrCode",                 &fDiffrCode);
    fOutTreeEvt->Branch("fEventNb",                   &fEventNb); 
    fOutTreeEvt->Branch("fHasRightParticlesInTPCITS", &fHasRightParticlesInTPCITS); 
    fOutTreeEvt->Branch("fInvarMass",                 &fInvarMass); 
    fOutTreeEvt->Branch("fRealInvarMass",             &fRealInvarMass); 
    fOutTreeEvt->Branch("fWholeEvtDetected",          &fWholeEvtDetected); 
    fOutTreeEvt->Branch("fHitInForwardDets",          &fHitInForwardDets); 
    fOutTreeEvt->Branch("fHitInAD",                   &fHitInAD); 
    fOutTreeEvt->Branch("fGammaInEMCal",              &fGammaInEMCal); 
    /* fOutTreeEvt->Branch("fDetGammaEt",                &fDetGammaEt); */ 

    Int_t iEvent = 0;
    if (fPyt) SetIEntryPythia(0);
    if (fKin) SetIEntryKin(0);
    while(EventLooper(maxEvts)){
        if (iEvent%1000 == 0) cout << iEvent << " events processed" << endl;
        AnalyseEvent(iEvent, fOutTreeEvt, mode, saveEvtInfo);
        iEvent++;
    }
    o_fEvtFile->cd();
    fOutTreeEvt->Write();
    o_fEvtFile->Close();
    delete o_fEvtFile;
}
//_____________________________________________________________________________
void EventHandler::AnalyseEvent(Int_t iEvent, TTree* tree, Int_t mode, Bool_t saveEvtInfo)
{
   // event preparation
    fEventNb = iEvent;
    Int_t evtsize;
    if (fKin)      evtsize = fEvtTree->GetEntries();
    else if (fPyt) evtsize = fPythiaEvent->size();
    else evtsize = 0;
    fNkaP = 0;
    fNkaM = 0;
    fNpiP = 0;
    fNpiM = 0;

    TLorentzVector vTemp;
    TLorentzVector vtot;

    fWholeEvtDetected          = true;
    fHasRightParticlesInTPCITS = true;
    fHitInForwardDets          = false;
    fHitInAD                   = false;
    fGammaInEMCal              = false;

    fDiffrCode = 0;
    Double_t charge, phi_det;
    Int_t statCode = 0, pythiaStatCode = 0;
    Double_t mass;
    for (Int_t i = 1; i < evtsize; i++) {
        // default value, if no gamma is in emcal, this way we can 
        // check what the gamma-et distribution is
        if (fKin){
            fEvtTree->GetEntry(i);
            fPdg = fPart->GetPdgCode();
            statCode = fPart->GetStatusCode();
            fEta = fPart->Eta();
            fPhi = fPart->Phi();
            fPt  = fPart->Pt();
        }
        if (fPyt){     
            fPdg = (*fPythiaEvent)[i].id();
            // we select the status code in a similar manner to the Kinematics 
            // status code
            statCode = (*fPythiaEvent)[i].isFinal() ? 1 : 0;
            fEta = (*fPythiaEvent)[i].eta();
            fPhi = (*fPythiaEvent)[i].phi();
            fPt  = (*fPythiaEvent)[i].pT();
            pythiaStatCode = (*fPythiaEvent)[i].status();
            PrintDebug("no problem accessing fPythia");
        } 
        if ( fPdg == 9900110 ){
            fDiffrCode = 3; 
            if (fKin) fRealInvarMass = fPart->GetCalcMass();
            if (fPyt) fRealInvarMass = (*fPythiaEvent)[i].m();
        }
        if ( fPdg == 9902210 ) fDiffrCode++;
        if ( statCode != 1 || pythiaStatCode == 14 ) continue;
        // now we have final particles
        charge = TDatabasePDG::Instance()->GetParticle(fPdg)->Charge();
        mass   = TDatabasePDG::Instance()->GetParticle(fPdg)->Mass();
        // ##########################################################################
        // ######################## Analysis ########################################
        // status code of 14 means a elastically scattered proton
        // if we continue the eventloop here we have a good event
        // if fWholeEvtDetected & fHasRightParticlesInTPCITS are true
        // is detected can be called multiple times as it just sets a bool
        // fIsDetected can only be called by charged particles
        if (abs(fEta) < 1.7 && fPt <= 0.12 && charge != 0. && pythiaStatCode!=14){
            fWholeEvtDetected = false;
            continue; 
        }
        if (charge != 0.) fIsDetected(fEta);
        // emcal hit
        phi_det = fPhi * 180./ TMath::Pi();
        if ( phi_det < 0. ) phi_det += 360.;
        if ( fPdg == 22 && abs(fEta) < 0.7 ){
            if ( (phi_det > 80. && phi_det < 187.) || (phi_det > 260. && phi_det < 327.) ){
            // if ( evt[i].eT() > 0.3 ) { 
                fGammaInEMCal = true;        
                /* fDetGammaEt = (*fPythiaEvent)[i].eT(); */
            }
        }
        if ( charge == 0. || abs(fEta) > 0.9 ) {
            fWholeEvtDetected = false;
            continue;
        }        // check if the right particles are in the evt
        // if ( !setPDGval(pdg) ) hasRightParticlesInTPCITS=false;

        if (abs(fEta) < 0.9){
            // setPDGval: 1 KK mode
            //            0 pp mode
            if (!setPDGval(mode) || fPt > 3.) fHasRightParticlesInTPCITS = false;
            else{
                vTemp.SetPtEtaPhiM(fPt, fEta, fPhi, mass);
                vtot += vTemp;
            }
        } 
        // ########################## end ###########################################
        // ##########################################################################
        /* if (tree->GetBranch("fEta")) tree->Fill(); */
        PrintDebug("got to end of loop");
        if (saveEvtInfo && fPyt) (*fPythiaEvent).list(os);
    }
    Int_t Npi = 0, Nka = 0;
    fHasRightNumber(Npi, Nka);
    // only if has right particles is still true we can set it to false
    if (fHasRightParticlesInTPCITS && mode==1) 
        fHasRightParticlesInTPCITS = (Npi==0 && Nka==2) ? true : false;
    if (fHasRightParticlesInTPCITS && mode==0) 
        fHasRightParticlesInTPCITS = (Npi==2 && Nka==0) ? true : false;
    fInvarMass = vtot.M();
    // fill the tree
    if (tree->GetBranch("fHitInAD")) tree->Fill();
    PrintDebug("evt tree fill");
    return ;
}
//_____________________________________________________________________________
TString EventHandler::GetOutputPath(Int_t mode)
{
    if (mode==0) return fOutpath+"evt.root";
    else return fOutpath+"part.root";
}
//_____________________________________________________________________________
void EventHandler::fIsDetected( Double_t eta )
{
    if ( abs( eta ) < 1.7 && abs( eta ) > 0.9 ) fHasRightParticlesInTPCITS = false;
    else if((eta>-3.4 && eta<-1.7) || (eta>1.7 && eta<5.1)) fHitInForwardDets = true;
    else if((eta>-3.7 && eta<-1.7) || (eta>2.8 && eta<5.1)) fHitInForwardDets = true;
    else if((eta>-7.0 && eta<-4.9) || (eta>4.8 && eta<6.3)) fHitInAD = true;
    else return ;
}
//_____________________________________________________________________________
Bool_t EventHandler::fHasRightNumber(Int_t& Npi, Int_t& Nka)
{
    if ( (fNpiP == 1 && fNpiM == 1) && fNkaP == 0 && fNkaM == 0 ){
        Npi = 2;
        Nka = 0;
        return true;
    } else if ( (fNpiP == 2 && fNpiM == 2) && fNkaP == 0 && fNkaM == 0 ){
        Npi = 4;
        Nka = 0;
        return true; 
    } else if ( (fNkaP == 1 && fNkaM == 1) && fNpiP == 0 && fNpiM == 0 ){
        Npi = 0;
        Nka = 2;
        return true;
    } else if ( (fNkaP == 2 && fNkaM == 2) && fNpiP == 0 && fNpiM == 0 ){
        Npi = 0;
        Nka = 4;
        return true;
    } else return false;
}
//_____________________________________________________________________________
Bool_t EventHandler::setPDGval(Int_t mode)
{
    // if mode = 0 we look only for 2pi
    // if mode = 1 we look only for 2ka
    if (mode==0){
        if ( fPdg == 211 ){
            fNpiP++;
            return true;
        } else if ( fPdg == -211 ){
            fNpiM++;
            return true;
        } else return false;
    } else if (mode>=1){
        if ( fPdg == 321 ){
            fNkaP++;
            return true;
        } else if ( fPdg == -321 ){
            fNkaM++;
            return true;
        } else return false;
    } else return false;
}
//_____________________________________________________________________________
