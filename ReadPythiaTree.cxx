/// ~~~{.cpp}
///  root > .x pythia8.C
/// ~~~

#include "Pythia8/Pythia.h"
/* #include "Pythia8/LHEF3.h" */
#include "TPythia8.h"
#include <TInterpreter.h>
#include <TSystem.h>
#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include "ReadPythiaTree.h"

class ReadPythiaTree;        // this class reads a 

using namespace std;

ClassImp(ReadPythiaTree);     // classimp: necessary for root

ReadPythiaTree::ReadPythiaTree(TString filename) : 
    // we initialize all private member variables pointing to 0, so it
    // does not get randwomly allocated
    fPythiaFile(0), fTree(0), fEntries(0), fiEntry(0),
    fEvent(0) 
{
    // constructor
    LoadLibraries();
    fEvent = &fPythia.event;
    fPythiaFile = TFile::Open(filename.Data());
    fTree = (TTree*)fPythiaFile->Get("T");
    if (fTree){
        fTree->SetBranchAddress("event",&fEvent);
        fEntries = fTree->GetEntries();
    }
}
//____________________________________________________________________________
Bool_t ReadPythiaTree::PythiaExists()
{
    if (fTree) return true;
    else return false;
}
//____________________________________________________________________________
ReadPythiaTree::~ReadPythiaTree()
{
    // destructor
    if(fPythiaFile){
        fPythiaFile->Close();
        delete fPythiaFile;
    }
    if(fTree) delete fTree;
}
//____________________________________________________________________________
Bool_t ReadPythiaTree::NextEvt()
{
    fTree->GetEntry(fiEntry);
    if(fiEntry+1 < fEntries){
        fiEntry++;
        return true;
    } else return false;
}
//____________________________________________________________________________

void ReadPythiaTree::LoadLibraries(void)
{
   const char *p8dataenv = gSystem->Getenv("PYTHIA8DATA");
   if (!p8dataenv) {
      const char *p8env = gSystem->Getenv("PYTHIA8");
      if (!p8env) {
            std::cout << "Environment variable PYTHIA8 must contain path to pythia directory!" 
                << std::endl;
            return;
      }
      TString p8d = p8env;
      p8d += "/xmldoc";
      gSystem->Setenv("PYTHIA8DATA", p8d);
   }

   const char* path = gSystem->ExpandPathName("$PYTHIA8DATA");
   if (gSystem->AccessPathName(path)) {
        std::cout << "Environment variable PYTHIA8DATA must contain path to $PYTHIA8/xmldoc directory !" << std::endl;
        return;
   }

// Load libraries
    gSystem->Load("libEG");
    gSystem->Load("libEGPythia8");

}
