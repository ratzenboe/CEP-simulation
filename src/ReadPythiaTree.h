/// ~~~{.cpp}

#ifndef ReadPythiaTree_H
#define ReadPythiaTree_H 

class ReadPythiaTree
{
        public:
                                ReadPythiaTree(TString filename);
                                ~ReadPythiaTree(void);
            Bool_t              NextEvt(void);
            Pythia8::Event*     GetEvent(void){ return fEvent; }
            Int_t               GetIEvt(void){ return fiEntry; }
            Bool_t              PythiaExists(void);
            void                SetIEntryPythia(Int_t iEvt){ fiEntry=iEvt; }
        private:
            TFile*              fPythiaFile;        // the input-file
            TTree*              fTree;              // in every directory
                                                    // there is a TTree 
            Int_t               fiEntry;
            Int_t               fEntries;

            Pythia8::Pythia     fPythia;
            Pythia8::Event*     fEvent;

            void                LoadLibraries(void);

};

#endif

