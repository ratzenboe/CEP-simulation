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
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "THistMaker.h"

// class that makes TH1 and TGraphs that are passed to main.
// in main a TList object is filled and saved in a .root file
// ------------------------------ class --------------------------------------
class THistMaker; 

using namespace std;

ClassImp(THistMaker);

THistMaker::THistMaker(TString inputFilePath, TString title, 
                       Int_t nBins, Double_t xlo, Double_t xhi) : 
    fNbins(nBins), fXhi(xhi), fXlo(xlo), fInFile(inputFilePath),
    // initiate member variables with 0
    fFile(0), fTree(0), fOutList(0), fOutputFile(0), 
    fHist_onlyTPC_CD(0), fHist_fwd_CD(0), fHist_ad_CD(0),
    fHist_onlyTPC_feedDown(0), fHist_fwd_feedDown(0), fHist_ad_feedDown(0), fMassCompare(0),
    fDiffrCode(-1), fHitInForwardDets(false), fHitInAD(false), fEventNb(0), fInvarMass(0),
    fGammaInEMCal(false), fWholeEvtDetected(false), fHasRightParticlesInTPCITS(false)
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
    fTree->SetBranchAddress("fGammaInEMCal",              &fGammaInEMCal);

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
    else if (mode==2) title="AD";
    else title="EMCal";    
    Bool_t kaon = false, kapi = false, pion = false;
    if (fInFile.Contains("kaon_")){
        title += "_kaon";
        kaon = true;
    }
    else if (fInFile.Contains("pi_")) {
        title += "_pi";
        pion = true;
    } else if (fInFile.Contains("piKaon_")) {
        title += "_piKaon";
        kapi = true;
    }
    // we only care for one plot at a time
    TH1F hist_cd;
    TH1F hist_fd;
    if (mode==0){
        hist_cd = *((TH1F*)fHist_onlyTPC_CD->Clone());
        hist_fd = *((TH1F*)fHist_onlyTPC_feedDown->Clone());
    } else if (mode==1){
        hist_cd = *((TH1F*)fHist_fwd_CD->Clone());
        hist_fd = *((TH1F*)fHist_fwd_feedDown->Clone());
    } else if (mode==2){
        hist_cd = *((TH1F*)fHist_ad_CD->Clone());
        hist_fd = *((TH1F*)fHist_ad_feedDown->Clone());
    } else {
        hist_cd = *((TH1F*)fHist_emc_CD->Clone());
        hist_fd = *((TH1F*)fHist_emc_feedDown->Clone());
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
    // scale the hsitograms 
    Int_t bin_lo = signal_cd_fd.FindBin(fXlo);
    Int_t bin_hi = signal_cd_fd.FindBin(fXhi);
    Int_t entries = signal_cd_fd.Integral(bin_lo, bin_hi);
    Double_t scale_norm = 1./double(entries);
    signal_cd_fd.Scale(scale_norm);
    hist_cd.Scale(scale_norm);
    hist_fd.Scale(scale_norm);
    // Draw the histograms
    // later we care about the style
    signal_cd_fd.Draw("EP");
    hist_cd.Draw("EP same");
    hist_fd.Draw("EP same");
    /* signal_cd_fd.SetMaximum(1.5*signal_cd_fd.GetMaximum()); */
    signal_cd_fd.SetMaximum(1.);
    // maker style and color
    signal_cd_fd.SetMarkerStyle(kFullCircle);
    hist_cd.SetMarkerStyle(kFullSquare);
    hist_fd.SetMarkerStyle(kFullTriangleUp);

    signal_cd_fd.SetMarkerColor(1);
    signal_cd_fd.SetMarkerSize(1.2);
    hist_cd.SetMarkerColor(9);
    hist_cd.SetMarkerSize(1.2);
    hist_fd.SetMarkerColor(2);
    hist_fd.SetMarkerSize(1.2);

    signal_cd_fd.GetYaxis()->SetLabelSize(0.045);
    signal_cd_fd.GetXaxis()->SetLabelSize(0.045);

    signal_cd_fd.GetXaxis()->SetLabelOffset(0.004);
    signal_cd_fd.GetYaxis()->SetLabelOffset(0.004);

    signal_cd_fd.GetXaxis()->SetTitleSize(0.05);
    signal_cd_fd.GetYaxis()->SetTitleSize(0.05);

    signal_cd_fd.GetYaxis()->SetTitleOffset(0.89);
    signal_cd_fd.GetXaxis()->SetTitleOffset(0.88);

    TString xTitle, yTitle;
    if (kaon){
        xTitle = "2K";
        yTitle = "dN/dm_{KK}";
    }
    if (kapi){ 
        xTitle = "K#pi";
        yTitle = "dN/dm_{K#pi}";
    }
    signal_cd_fd.GetXaxis()->SetTitle((xTitle+" invariant mass [GeV/c^{2}]").Data());
    signal_cd_fd.GetYaxis()->SetTitle("N");
    signal_cd_fd.GetYaxis()->SetTitle(yTitle.Data());
    // SetTitle has to be called after setting the xAxis title!!!!

    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(12);
    latex.SetTextAngle(90);
    latex.SetTextColor(13);
    Double_t mult = 0.9;
    // for KK in 45 mio CEP sample
    Double_t add_toright = 0.05;
    if (kapi) add_toright = 0.06;
    Double_t fixedLineLength = 0.5;
    Double_t lineScale = 0.3;
    if (kapi) lineScale = 10e-5;
    Double_t xMax_phi   = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.02))*mult;
    Double_t xMax_f2    = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.27))*mult;
    Double_t xMax_f2w   = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.37))*mult;
    Double_t xMax_f2rho = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.56))*mult;
    if (kaon) {
        latex.DrawLatex(1.02+add_toright,  lineScale*fixedLineLength, "#phi(1020)");
        latex.DrawLatex(1.27+add_toright,  lineScale*fixedLineLength, "f2(1270)");
        latex.DrawLatex(1.42+add_toright,  lineScale*fixedLineLength, "f2, #omega(1420)");
        latex.DrawLatex(1.567+add_toright, lineScale*fixedLineLength, "f2, #rho(1570)");
    } else if(kapi) latex.DrawLatex(0.892+add_toright,lineScale*fixedLineLength,"K*(892)");

    TLine l;
    l.SetLineWidth(1.);
    l.SetLineStyle(7);
    l.SetLineColor(13);
    if (kaon) {
        l.DrawLine(1.02, 0.0, 1.02, fixedLineLength);
        l.DrawLine(1.27, 0.0, 1.27, fixedLineLength);
        l.DrawLine(1.42, 0.0, 1.42, fixedLineLength);
        l.DrawLine(1.56, 0.0, 1.56, fixedLineLength);
    } else if (kapi) l.DrawLine(0.892, 0.0, 0.892, fixedLineLength);

    TString fd_str   = "Feed down: ";
    TString full_str = "Full recon: ";
    Double_t fd_part  = double(hist_fd.Integral())/double(signal_cd_fd.Integral()) *100.;
    Double_t rec_part = double(hist_cd.Integral())/double(signal_cd_fd.Integral()) *100.;
    stringstream str_fd, str_cd;
    str_fd << fixed << setprecision(2) << fd_part;
    string fd_percent = str_fd.str();
    str_cd << fixed << setprecision(2) << rec_part;
    string cd_percent = str_cd.str();

    Double_t xleg0 = 0.560748;
    Double_t yleg0 = 0.523172;
    Double_t xleg1 = 0.714953;
    Double_t yleg1 = 0.873326;
    TLegend* leg = new TLegend(xleg0, yleg0, xleg1, yleg1);
    leg->SetHeader( "#splitline{pp #sqrt{s} = 14TeV}{Pythia - MBR (#varepsilon = 0.08)}" );
    leg->SetFillStyle(0);
    leg->SetTextSize(0.05);
    leg->AddEntry( &signal_cd_fd, "CEP total", "ep"); 
    leg->AddEntry( &hist_cd, (full_str+cd_percent+"%").Data(), "ep"); 
    leg->AddEntry( &hist_fd, (fd_str+fd_percent+"%").Data(), "ep"); 
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
    fMassCompare->GetXaxis()->SetTitle("2K_{Generated} [GeV/c^{2}]");
    fMassCompare->GetYaxis()->SetTitle("2K_{Detected} [GeV/c^{2}]");

    fMassCompare->GetXaxis()->SetTitleOffset(0.9);
    fMassCompare->GetXaxis()->SetTitleSize(0.05);

    fMassCompare->GetYaxis()->SetTitleOffset(0.9);
    fMassCompare->GetYaxis()->SetTitleSize(0.05);

    TLine feedLine1;
    feedLine1.SetLineColor(kRed);
    feedLine1.SetLineWidth(1);
    feedLine1.SetLineStyle(2);
    /* TLatex feedText; */
    /* feedText.SetTextSize(0.025); */
    /* feedText.SetTextAlign(22); */
    /* feedText.SetTextColor(kRed); */
    /* feedText.DrawLatex(2.5, 1.8, "Feed down events"); */
    /* feedLine1.DrawLine(1.05,0.95,0.7,0.6); */
    feedLine1.DrawLine(1.05,0.95,3.2*0.95,0.95);
    feedLine1.DrawLine(1.05,0.95,3.0,2.9);

    c->SaveAs((outpath+"massComparison.pdf").Data());

    delete c;
}



