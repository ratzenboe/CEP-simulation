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
    fNpiP(0), fNpiM(0), fNkaP(0), fNkaM(0), fNprotM(0), fNprotP(0),
    // output to particle file
    fEta(0), fPhi(0), fPt(0), fVx(0), fVy(0), fVz(0), fPdg(0), fEventNb(0), fDiffrCode(-1),
    // output to event file (also with event number and diffractive code)
    fHasRightParticlesInTPCITS(false), fWholeEvtDetected(false), fHitInForwardDets(false), 
    fInvarMass(0), fHitInAD(0), fDetGammaEt(0), fGammaInEMCal(false), fFromCEP(-1),
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
    if (mode==2) op += "piKaon_";
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
    if (mode==2) op += "piKaon_";
    if (mode==3) op += "piProton_";
    if (mode==4) op += "antipProton_";
    TFile* o_fEvtFile  = new TFile((op+"evt.root").Data(),  "RECREATE");
    TTree* fOutTreeEvt  = new TTree("event", "event tree");

    printf("Generated new output file: %s\n", (op+"evt.root").Data());
    fOutTreeEvt->Branch("fDiffrCode",                 &fDiffrCode);
    fOutTreeEvt->Branch("fEventNb",                   &fEventNb); 
    fOutTreeEvt->Branch("fHasRightParticlesInTPCITS", &fHasRightParticlesInTPCITS); 
    fOutTreeEvt->Branch("fInvarMass",                 &fInvarMass); 
    fOutTreeEvt->Branch("fRealInvarMass",             &fRealInvarMass); 
    fOutTreeEvt->Branch("fWholeEvtDetected",          &fWholeEvtDetected); 
    fOutTreeEvt->Branch("fHitInForwardDets",          &fHitInForwardDets); 
    fOutTreeEvt->Branch("fHitInAD",                   &fHitInAD); 
    fOutTreeEvt->Branch("fGammaInEMCal",              &fGammaInEMCal); 
    // if the detected particles come directely from the CEP particle
    // or e.g. from CEP -> 2rho -> 4pi (makes only sense for full recon evts)
    fOutTreeEvt->Branch("fFromCEP",                   &fFromCEP); 
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
    fNkaP   = 0;
    fNkaM   = 0;
    fNpiP   = 0;
    fNpiM   = 0;
    fNprotP = 0;
    fNprotM = 0;

    TLorentzVector vTemp;
    TLorentzVector vtot;

    fWholeEvtDetected          = true;
    fHasRightParticlesInTPCITS = true;
    fHitInForwardDets          = false;
    fHitInAD                   = false;
    fGammaInEMCal              = false;

    fDiffrCode = 0;
    Double_t charge, phi_det;
    Int_t statCode = 0, pythiaStatCode = 0, idMom = 0;
    Double_t mass;
    // to assess if the particles come from the CEP or from a daughter of it
    // is neccessary for fFromCEP
    std::vector<Int_t> pdgMomVec;
    pdgMomVec.clear();
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
            idMom = fPart->GetMother(0);
            mother1 = (abs(idMom) < 10 || idMom==21) ? 1 : idMom;
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
            idMom = (*fPythiaEvent)[(*fPythiaEvent)[i].mother1()].id();
            // either a quark <10 or a gluon==21
            mother1 = (abs(idMom) < 10 || idMom==21) ? 1 : idMom;
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
        if (abs(fEta) < 1.7 && fPt <= 0.12 && charge != 0.) {
            fWholeEvtDetected = false;
            continue; 
        }
        if (charge != 0.) fIsDetected();
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
            if (!setPDGval() || fPt > 3.) fHasRightParticlesInTPCITS = false;
            else{
                vTemp.SetPtEtaPhiM(fPt, fEta, fPhi, mass);
                vtot += vTemp;
                pdgMomVec.push_back(mother1);
            }
        } 
        // ########################## end ###########################################
        // ##########################################################################
        /* if (tree->GetBranch("fEta")) tree->Fill(); */
        PrintDebug("got to end of loop");
    }
    Int_t Npi = 0, Nka = 0, Nprot = 0;
    fHasRightNumber(Npi, Nka, Nprot);
    // only if has right particles is still true we can set it to false
    if (fHasRightParticlesInTPCITS){
        switch(mode){
            case pipi: fHasRightParticlesInTPCITS = (Npi==2 && Nka==0 && Nprot==0) ? true : false;
                       break;
            case kaka: fHasRightParticlesInTPCITS = (Npi==0 && Nka==2 && Nprot==0) ? true : false;
                       break;
            case pika: fHasRightParticlesInTPCITS = (Npi==1 && Nka==1 && Nprot==0) ? true : false;
                       break;
            case piPr: fHasRightParticlesInTPCITS = (Npi==1 && Nka==0 && Nprot==1) ? true : false;
                       break;
            case pp:   fHasRightParticlesInTPCITS = (Npi==0 && Nka==0 && Nprot==2) ? true : false;
                       break;
            case fourpi:   
                       fHasRightParticlesInTPCITS = (Npi==4 && Nka==0 && Nprot==0) ? true : false;
                       break;
            case fourka:   
                       fHasRightParticlesInTPCITS = (Npi==4 && Nka==0 && Nprot==0) ? true : false;
                       break;        
            case pikafour:   
                       fHasRightParticlesInTPCITS = (Npi==2 && Nka==2 && Nprot==0) ? true : false;
                       break; 
        }
    }
    fInvarMass = vtot.M();
    if (saveEvtInfo && fPyt && fHasRightParticlesInTPCITS && fWholeEvtDetected){
        os << "fWholeEvtDetected: " << fWholeEvtDetected << endl;
        os << "fDiffrCode:        " << fDiffrCode        << endl;
        os << "fHitInAD:          " << fHitInAD          << endl;
        os << "fHitInForwardDets: " << fHitInForwardDets << endl;
        (*fPythiaEvent).list(os);
    }
    // checkout if the particles come from the CEP or from another particle
    Int_t vectorSum(0), CEPMomCounter(0);

    for (Unsigned i(0); i < pdgMomVec.size(); i++){
        vectorSum += pdgMomVec[i];
        if (pdgMomVec==1) CEPMomCounter++;
    }
    // if all come from CEP: 0
    if (pdgMomVec.size() > 1 && fWholeEvtDetected){
        if (CEPMomCounter==pdgMomVec.size()) fFromCEP = 0;
        // if all come from a different particle we put the particles pdg code
        // the adjacent_find with not_equal_to looks if all the elements in a vector are the same  
        else if (CEPMomCounter==0 && 
                std::adjacent_find( pdgMomVec.begin(), pdgMomVec.end(), 
                    std::not_equal_to<Int_t>() ) == pdgMomVec.end() ) fFromCEP = pdgMomVec[0];
        else fFromCEP = -1;
    }
    // fill the tree
    if (fHasRightParticlesInTPCITS && tree->GetBranch("fHitInAD")) tree->Fill();
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
void EventHandler::fIsDetected(void)
{
    if ( abs( fEta ) < 1.7 && abs( fEta ) > 0.9 )               fHasRightParticlesInTPCITS = false;
    else if((fEta>-3.4 && fEta<-1.7) || (fEta>1.7 && fEta<5.1)) fHitInForwardDets = true;
    else if((fEta>-3.7 && fEta<-1.7) || (fEta>2.8 && fEta<5.1)) fHitInForwardDets = true;
    else if((fEta>-7.0 && fEta<-4.9) || (fEta>4.8 && fEta<6.3)) fHitInAD = true;
    else return ;
}
//_____________________________________________________________________________
void EventHandler::fHasRightNumber(Int_t& Npi, Int_t& Nka, Int_t &Npro)
{
    if ( fNprotP==0 && fNprotM==0 ){
        if ( (fNpiP == 1 && fNpiM == 1) && fNkaP == 0 && fNkaM == 0 ){ Npi = 2; return ;}
        else if ((fNpiP == 2 && fNpiM == 2) && fNkaP == 0 && fNkaM == 0){ Npi = 4; return ;}
        else if ((fNkaP == 1 && fNkaM == 1) && fNpiP == 0 && fNpiM == 0){ Nka = 2; return ;}
        else if ((fNkaP == 2 && fNkaM == 2) && fNpiP == 0 && fNpiM == 0){ Nka = 4; return ;}
        else if ((fNkaP == 1 && fNkaM == 0) && fNpiP == 0 && fNpiM == 1){ Npi = 1; Nka = 1; return ;}
        else if ((fNkaP == 0 && fNkaM == 1) && fNpiP == 1 && fNpiM == 0){ Npi = 1; Nka = 1; return ;}
        else if ((fNkaP == 0 && fNkaM == 1) && fNpiP == 1 && fNpiM == 0){ Npi = 1; Nka = 1; return ;}
        else if ((fNkaP == 0 && fNkaM == 0) && fNpiP == 2 && fNpiM == 2){ Npi = 4; return ;}
        else if ((fNkaP == 2 && fNkaM == 2) && fNpiP == 0 && fNpiM == 0){ Nka = 4; return ;}
        else if ((fNkaP == 1 && fNkaM == 1) && fNpiP == 1 && fNpiM == 1){ 
            Nka = 2; Npi = 2; return ;}
        else return ;
    } 
    else if ( fNkaM == 0 && fNkaP == 0 ) {
        if ((fNprotP == 1 && fNprotM == 0) && (fNpiP==0 && fNpiM==1)){ Npro = 1; Npi = 1; return ;}
        else if ((fNprotP == 1 && fNprotM == 0) && (fNpiP==0 && fNpiM==1)){ 
            Npro = 1; Npi = 1; return ;}
        else if ((fNprotP == 1 && fNprotM == 1) && (fNpiP==0 && fNpiM==0)){ Npro = 2; return ;}
        else return ;
    } else return ;
}
//_____________________________________________________________________________
Bool_t EventHandler::setPDGval(void)
{
    switch(fPdg){
        case      pion: fNpiP++;   break;
        case   (-pion): fNpiM++;   break;
        case      kaon: fNkaP++;   break;
        case   (-kaon): fNkaM++;   break;
        case    proton: fNprotP++; break;
        case (-proton): fNprotM++; break;

        default: return false; 
    }
    return true;
}
//_____________________________________________________________________________
