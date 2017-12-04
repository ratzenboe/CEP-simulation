/// ~~~{.cpp}
///  root > .x pythia8.C
/// ~~~

#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TString.h>
#include "ReadDirectoryTree.h"

class ReadDirectoryTree;        // this class reads a 

using namespace std;

ClassImp(ReadDirectoryTree)     // classimp: necessary for root

ReadDirectoryTree::ReadDirectoryTree(TString filename) : 
    // we initialize all private member variables pointing to 0, so it
    // does not get randwomly allocated
    fKinematicsFile(0), fDir(0), fEvtString("Event"), fTree(0), fiEntry(0)
{
    // constructor
    fKinematicsFile = TFile::Open(filename);
}
//____________________________________________________________________________
ReadDirectoryTree::~ReadDirectoryTree()
{
    // destructor
    if (fDir) delete fDir;
    // I think that the tree gets deleted alongside the file (maybe not)
    // have to check that out!!!
    if(fKinematicsFile){
        fKinematicsFile->Close();
        delete fKinematicsFile;
    }
}
//____________________________________________________________________________
bool ReadDirectoryTree::KinematicsExists()
{
    if (fKinematicsFile->GetDirectory("Event1")) return true;
    else return false;
}
//____________________________________________________________________________
bool ReadDirectoryTree::NextEvtKinematic()
{
    // if there the entry is above 0 we have already created a fDir, so we delete
    // it and get a new one
    TString output;
    output.Form("%d", fiEntry);
    if ( fiEntry > 0 ) delete fDir;
    fDir = fKinematicsFile->GetDirectory( fEvtString+output );
    //cout << (m__evtString+to_string(m__iEntry)).c_str() << endl;
    fiEntry++;
    if ( fDir ){
        fDir->GetObject("TreeK", fTree);
        // perform the next step in the EventHandler where we get the fTree
        // and we loop through the event, there make this fPart variable
        // T->SetBranchAddress("Particles", &fPart);
        return true;
    } else return false;
}
//____________________________________________________________________________
