/// ~~~{.cpp}

#ifndef ReadDirectoryTree_H
#define ReadDirectoryTree_H 

/* #include <TString> */
/* #include <TDirectory> */
/* #include <TTree> */
/* #include <TFile> */


class ReadDirectoryTree
{
        public:
                                ReadDirectoryTree(TString filename);
                                ~ReadDirectoryTree(void);
            Bool_t              NextEvtKinematic(void);
            TTree*              GetEventTree(void){ return fTree; }
            Int_t               GetIEvt(void){ return fiEntry; }
            Bool_t              KinematicsExists(void);
            void                SetIEntryKin(Int_t iEvt){ fiEntry = iEvt; }

        private:
            TFile*              fKinematicsFile;    // the input-file
            TDirectory*         fDir;               // structure of the file
            TString             fEvtString;         // will be initialized as "Event"
            TTree*              fTree;              // in every directory
                                                    // there is a TTree 
            Int_t               fiEntry;
};

#endif

