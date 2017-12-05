/// ~~~{.cpp}
///  root > .x pythia8.C
/// ~~~
/// Note that before executing this script,
//,/
///  - the env variable PYTHIA8 must point to the pythia8100 (or newer) directory
///  - the env variable PYTHIA8DATA must be defined and it must point to $PYTHIA8/xmldoc
/// Note2: add this line to the rootlogon file. Otherwise the pythia library wont be loaded:
//   - gInterpreter->AddIncludePath("/home/ratzenboe/alice/ali-master/AliRoot/PYTHIA8/pythia8/include")

#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TString.h>
#include <TLatex.h>
#include <TLine.h>
#include <TLegend.h>
#include <TStyle.h>
#include "THistMaker.h"

// class that makes TH1 and TGraphs that are passed to main.
// in main a TList object is filled and saved in a .root file
// ------------------------------ class --------------------------------------
class THistMaker; 

using namespace std;

ClassImp(THistMaker);

THistMaker::THistMaker(TString inputFilePath, TString title, Int_t nBins, Double_t xlo, Double_t xhi) : 
    // initiate member variables with 0
    fFile(0), fTree(0), fOutList(0), fOutputFile(0), 
    fHist_onlyTPC_CD(0), fHist_fwd_CD(0), fHist_ad_CD(0),
    fHist_onlyTPC_feedDown(0), fHist_fwd_feedDown(0), fHist_ad_feedDown(0), fMassCompare(0),
    fDiffrCode(-1), fHitInForwardDets(false), fHitInAD(false), fEventNb(0), fInvarMass(0),
    fWholeEvtDetected(false), fHasRightParticlesInTPCITS(false)
{
    fOutList = new TList();
    TString outFileFolder = "/home/ratzenboe/Documents/Pythia-stuff/InvariantMass_study/HistSave/";
    fOutputFile = new TFile( (outFileFolder+title).Data(), "RECREATE" );

    fFile = TFile::Open(inputFilePath);
    fTree = (TTree*)fFile->Get("event");
    // get the tree from the EventHandler
    fTree->SetBranchAddress("fDiffrCode",                 &fDiffrCode);
    fTree->SetBranchAddress("fEventNb",                   &fEventNb);
    fTree->SetBranchAddress("fHasRightParticlesInTPCITS", &fHasRightParticlesInTPCITS);
    fTree->SetBranchAddress("fInvarMass",                 &fInvarMass);
    fTree->SetBranchAddress("fRealInvarMass",             &fRealInvarMass);
    fTree->SetBranchAddress("fWholeEvtDetected",          &fWholeEvtDetected);
    fTree->SetBranchAddress("fHitInForwardDets",          &fHitInForwardDets);
    fTree->SetBranchAddress("fHitInAD",                   &fHitInAD);

    // create the histograms (true CD)
    fHist_onlyTPC_CD       = new TH1F("onlyITS_TPC_CD", "only ITS TPC",           nBins, xlo, xhi);
    fHist_fwd_CD           = new TH1F("forwardDet_CD", "forward detectors on",    nBins, xlo, xhi);
    fHist_ad_CD            = new TH1F("ADDet_CD", "AD detectors on",              nBins, xlo, xhi);
    fHist_emc_CD           = new TH1F("EMC_det", "EMCal on",                      nBins, xlo, xhi);
    // feed down histograms
    fHist_onlyTPC_feedDown = new TH1F("onlyITS_TPC_FeedD", "only ITS TPC",        nBins, xlo, xhi);
    fHist_fwd_feedDown     = new TH1F("forwardDet_FeedD", "forward detectors on", nBins, xlo, xhi);
    fHist_ad_feedDown      = new TH1F("ADDet_FeedD", "AD detectors on",           nBins, xlo, xhi);
    fHist_emc_feedDown     = new TH1F("emc_FeedD", "EMCal on",                    nBins, xlo, xhi);
    // 2D mass comparison histogram
    fMassCompare           = new TH2D("Mass comparison", "AD det mass comp",
                                                                 nBins, xlo, xhi, nBins, xlo, xhi);


    for (unsigned i(0); i<fTree->GetEntries(); i++){
        fTree->GetEntry(i);
        if (fDiffrCode != 3) continue;
        if (!fHasRightParticlesInTPCITS) continue;
        // fill the TPC/ITS histograms
        if (!fWholeEvtDetected) fHist_onlyTPC_feedDown->Fill(fInvarMass);
        else fHist_onlyTPC_CD->Fill(fInvarMass);
        // fill hits in FWD 
        if (fHitInForwardDets) continue;
        if (!fWholeEvtDetected) fHist_fwd_feedDown->Fill(fInvarMass);
        else fHist_fwd_CD->Fill(fInvarMass);
        // additional AD detector
        if (fHitInAD) continue;
        if (!fWholeEvtDetected) fHist_ad_feedDown->Fill(fInvarMass);
        else fHist_ad_CD->Fill(fInvarMass);
        fMassCompare->Fill(fRealInvarMass, fInvarMass);
        // additional EMCal
        if (fGammaInEMCal) continue;
        if (!fWholeEvtDetected) fHist_emc_feedDown->Fill(fInvarMass);
        else fHist_emc_CD->Fill(fInvarMass);

    }
}
//_______________________________________________________________________________
THistMaker::~THistMaker(void)
{
    delete fHist_onlyTPC_CD;
    delete fHist_onlyTPC_feedDown;
    delete fHist_fwd_CD;
    delete fHist_fwd_feedDown;
    delete fHist_ad_CD;
    delete fHist_ad_feedDown;

    fOutputFile->Close();
    delete fOutputFile;
}
//_______________________________________________________________________________
void THistMaker::SaveList(void)
{
    fOutputFile->cd();
    fOutList->Write();
    fOutList->Clear();
}
//_______________________________________________________________________________
void THistMaker::SaveHistsInFile(Int_t mode, TString outpath)
{
    TString title;
    if (mode==0) title="TPC_only";
    else if (mode==1) title="Forward";
    else title="AD";    
    // we only care for one plot at a time
    TH1F hist_cd;
    TH1F hist_fd;
    if (mode==0){
        hist_cd = *((TH1F*)fHist_onlyTPC_CD->Clone());
        hist_fd = *((TH1F*)fHist_onlyTPC_feedDown->Clone());
    } else if (mode==1){
        hist_cd = *((TH1F*)fHist_fwd_CD->Clone());
        hist_fd = *((TH1F*)fHist_fwd_feedDown->Clone());
    } else {
        hist_cd = *((TH1F*)fHist_ad_CD->Clone());
        hist_fd = *((TH1F*)fHist_ad_feedDown->Clone());
    } 

    TCanvas* c = new TCanvas( ("c"+title).Data(), "c", 1500, 1000 );
    gStyle->SetOptStat(0);
    gPad->SetLogy();
    /* // first we make the signal histograms = add CD + feeddown (as we only regard CEP) */
    Double_t xmin  = hist_cd.GetXaxis()->GetXmin();
    Double_t xmax  = hist_cd.GetXaxis()->GetXmax();
    Int_t    nBins = hist_cd.GetNbinsX(); 
    TH1F signal_cd_fd(("signal"+title).Data(), "", nBins, xmin, xmax);
    signal_cd_fd.Add(&hist_cd);
    signal_cd_fd.Add(&hist_fd);
    /* // scale the hsitograms */ 
    Int_t entries = signal_cd_fd.GetEntries();
    Double_t scale_norm = 1./double(entries);
    signal_cd_fd.Scale(scale_norm);
    hist_cd.Scale(scale_norm);
    hist_fd.Scale(scale_norm);
    // Draw the histograms
    // later we care about the style
    signal_cd_fd.Draw("EP");
    hist_cd.Draw("EP same");
    hist_fd.Draw("EP same");
    // maker style and color
    signal_cd_fd.SetMarkerStyle(kFullCircle);
    hist_cd.SetMarkerStyle(kFullSquare);
    hist_fd.SetMarkerStyle(kFullTriangleUp);

    signal_cd_fd.SetMarkerColor(1);
    signal_cd_fd.SetMarkerSize(1.2);
    hist_cd.SetMarkerColor(9);
    hist_cd.SetMarkerSize(1.2);
    hist_fd.SetMarkerColor(8);
    hist_fd.SetMarkerSize(1.2);

    signal_cd_fd.GetYaxis()->SetLabelSize(0.045);
    signal_cd_fd.GetXaxis()->SetLabelSize(0.045);

    signal_cd_fd.GetXaxis()->SetLabelOffset(0.004);
    signal_cd_fd.GetYaxis()->SetLabelOffset(0.004);

    signal_cd_fd.GetXaxis()->SetTitleSize(0.05);
    signal_cd_fd.GetYaxis()->SetTitleSize(0.05);

    signal_cd_fd.GetYaxis()->SetTitleOffset(0.89);
    signal_cd_fd.GetXaxis()->SetTitleOffset(0.88);

    signal_cd_fd.GetXaxis()->SetTitle("2K invariant mass [GeV/c^{2}]");
    signal_cd_fd.GetYaxis()->SetTitle("N");
    signal_cd_fd.GetYaxis()->SetTitle("dN/dm_{KK}");
    // SetTitle has to be called after setting the xAxis title!!!!

    /* TLatex latex; */
    /* latex.SetTextSize(0.06); */
    /* latex.SetTextAlign(12); */
    /* Double_t mult = 2; */
    /* Double_t xMax_K0 = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(0.497))  *mult; */
    /* Double_t xMax_rho = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(0.775)) *mult; */
    /* latex.DrawLatex(0.45, xMax_K0 , "K^{0}_{S}"); */
    /* latex.DrawLatex(0.72, xMax_rho, "#rho, #omega"); */

    /* TLine l; */
    /* l.SetLineWidth(1.); */
    /* l.SetLineStyle(7); */
    /* l.SetLineColor(1); */
    /* l.DrawLine(0.497,0.0, 0.497, xMax_K0*0.5); */
    /* l.DrawLine(0.77, 0.0, 0.77, xMax_rho*0.5); */

    Double_t xleg0 = 0.576769;
    Double_t yleg0 = 0.523172;
    Double_t xleg1 = 0.827103;
    Double_t yleg1 = 0.873326;
    TLegend* leg = new TLegend(xleg0, yleg0, xleg1, yleg1);
    leg->SetHeader( "#splitline{pp #sqrt{s} = 14TeV}{Pythia - MBR (#varepsilon = 0.08)}" );
    leg->SetFillStyle(0);
    leg->SetTextSize(0.05);
    leg->AddEntry( &signal_cd_fd, "CEP total", "ep"); 
    leg->AddEntry( &hist_cd, "CEP full recon", "ep"); 
    leg->AddEntry( &hist_fd, "CEP feed down", "ep"); 
    leg->SetBorderSize(0);
    leg->Draw();

    c->SaveAs((outpath+title+".pdf").Data());

    delete c;
}
//________________________________________________________________________________________
void THistMaker::Save2DMassHistInFile(TString outpath)
{
    TCanvas* c = new TCanvas("Mass comparison", "", 900, 700);
    gPad->SetLogz();
    fMassCompare->SetStats( false );
    fMassCompare->Draw("colz");
    fMassCompare->GetXaxis()->SetTitle("M_{Gen}/N_{Gen}(M) [GeV/c^{2}]");
    fMassCompare->GetYaxis()->SetTitle("M_{Detected} [GeV/c^{2}]");

    fMassCompare->GetXaxis()->SetTitleOffset(0.9);
    fMassCompare->GetXaxis()->SetTitleSize(0.05);

    fMassCompare->GetYaxis()->SetTitleOffset(0.9);
    fMassCompare->GetYaxis()->SetTitleSize(0.05);

    TLine feedLine1;
    feedLine1.SetLineColor(kRed);
    feedLine1.SetLineWidth(1);
    feedLine1.SetLineStyle(2);
    TLatex feedText;
    feedText.SetTextSize(0.025);
    feedText.SetTextAlign(22);
    feedText.SetTextColor(kRed);
    feedText.DrawLatex(4.5, 2.3, "Feed down events");
    feedLine1.DrawLine(0.7,0.2,0.7,0.6);
    feedLine1.DrawLine(0.7,0.2,3.2*0.9,0.2);
    feedLine1.DrawLine(0.7,0.6,3.0,2.9);

    c->SaveAs((outpath+"massComparison.pdf").Data());

    delete c;
}



