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

THistMaker::THistMaker(TString inputFilePath, Int_t nBins, Double_t xlo, Double_t xhi) : 
    fNbins(nBins), fXhi(xhi), fXlo(xlo), fInFile(inputFilePath), fTitleSuffix(""),
    // initiate member variables with 0
    fFile(0), fTree(0), fOutList(0), fOutputFile(0), fTPCITSsignalEnries(0),
    fHist_TPC_CD(0), fHist_TPC_feedD(0), fHist_TPC_fromCEP(0), fHist_TPC_fromDiff(0),
    fHist_fwd_CD(0), fHist_fwd_feedD(0), fHist_fwd_fromCEP(0), fHist_fwd_fromDiff(0),
    fHist_ad_CD(0),  fHist_ad_feedD(0),  fHist_ad_fromCEP(0),  fHist_ad_fromDiff(0),
    fHist_emc_CD(0), fHist_emc_feedD(0), fHist_emc_fromCEP(0), fHist_emc_fromDiff(0),
    fMassCompare(0), fDiffrCode(-1), fHitInForwardDets(false), fHitInAD(false), fEventNb(0), 
    fInvarMass(0),  fGammaInEMCal(false), fWholeEvtDetected(false), fFromCEP(-1),
    fHasRightParticlesInTPCITS(false), fShowCEPComponents(false)
{
    fOutList = new TList();
    TString outFileFolder = "/home/ratzenboe/Documents/Pythia-stuff/InvariantMass_study/HistSave/";
    TString title = "histList";
    if (fInFile.Contains("kaon_"))        fTitleSuffix += "_kaon";
    if (fInFile.Contains("pi_"))          fTitleSuffix += "_pi";
    if (fInFile.Contains("piKaon_"))      fTitleSuffix += "_piKaon";
    if (fInFile.Contains("piProton_"))    fTitleSuffix += "_piProton";
    if (fInFile.Contains("antipProton_")) fTitleSuffix += "_antiProton";
    if (fInFile.Contains("fourka_")){
        fTitleSuffix += "_fourKa";
        fShowCEPComponents = true;
    }
    if (fInFile.Contains("fourpi_")){
        fTitleSuffix += "_fourPi";
        fShowCEPComponents = true;
    }    
    fOutputFile = new TFile( (outFileFolder+title+fTitleSuffix+".root").Data(), "RECREATE" );

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
    fTree->SetBranchAddress("fFromCEP",                   &fFromCEP);

    // create the histograms (true CD)
    fHist_TPC_CD       = new TH1F("onlyITS_TPC_CD", "only ITS TPC",           nBins, xlo, xhi);
    fHist_fwd_CD       = new TH1F("forwardDet_CD", "forward detectors on",    nBins, xlo, xhi);
    fHist_ad_CD        = new TH1F("ADDet_CD", "AD detectors on",              nBins, xlo, xhi);
    fHist_emc_CD       = new TH1F("EMC_det", "EMCal on",                      nBins, xlo, xhi);
    // feed down histograms
    fHist_TPC_feedD    = new TH1F("onlyITS_TPC_FeedD", "only ITS TPC",        nBins, xlo, xhi);
    fHist_fwd_feedD    = new TH1F("forwardDet_FeedD", "forward detectors on", nBins, xlo, xhi);
    fHist_ad_feedD     = new TH1F("ADDet_FeedD", "AD detectors on",           nBins, xlo, xhi);
    fHist_emc_feedD    = new TH1F("emc_FeedD", "EMCal on",                    nBins, xlo, xhi);
    // final paricles coming directly from the CEP particles
    fHist_TPC_fromCEP  = new TH1F("onlyITS_TPC_fromCEP", "only ITS TPC",        nBins, xlo, xhi);
    fHist_fwd_fromCEP  = new TH1F("forwardDet_fromCEP", "forward detectors on", nBins, xlo, xhi);
    fHist_ad_fromCEP   = new TH1F("ADDet_fromCEP", "AD detectors on",           nBins, xlo, xhi);
    fHist_emc_fromCEP  = new TH1F("emc_fromCEP", "EMCal on",                    nBins, xlo, xhi);
    // final paricles coming all from the same (pdg wise) particles (e.g. 2 rhos)
    fHist_TPC_fromDiff = new TH1F("onlyITS_TPC_fromDiff", "only ITS TPC",        nBins, xlo, xhi);
    fHist_fwd_fromDiff = new TH1F("forwardDet_fromDiff", "forward detectors on", nBins, xlo, xhi);
    fHist_ad_fromDiff  = new TH1F("ADDet_fromDiff", "AD detectors on",           nBins, xlo, xhi);
    fHist_emc_fromDiff = new TH1F("emc_fromDiff", "EMCal on",                    nBins, xlo, xhi);
     // 2D mass comparison histogram
    fMassCompare           = new TH2D("Mass comparison", "AD det mass comp",
                                                                 nBins, xlo, xhi, nBins, xlo, xhi);
    for (unsigned i(0); i<fTree->GetEntries(); i++){
        fTree->GetEntry(i);
        if (fDiffrCode != 3) continue;
        if (!fHasRightParticlesInTPCITS) continue;
        // fill the TPC/ITS histograms
        if (!fWholeEvtDetected)                     fHist_TPC_feedD->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==0) fHist_TPC_fromCEP->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP>0)  fHist_TPC_fromDiff->Fill(fInvarMass);
        else fHist_TPC_CD->Fill(fInvarMass);
        // fill hits in FWD 
        if (fHitInForwardDets) continue;
        if (!fWholeEvtDetected) fHist_fwd_feedD->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==0) fHist_fwd_fromCEP->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP>0)  fHist_fwd_fromDiff->Fill(fInvarMass);
        else fHist_fwd_CD->Fill(fInvarMass);
        // additional AD detector
        if (fHitInAD) continue;
        if (!fWholeEvtDetected) fHist_ad_feedD->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==0) fHist_ad_fromCEP->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP>0)  fHist_ad_fromDiff->Fill(fInvarMass);        
        else fHist_ad_CD->Fill(fInvarMass);
        fMassCompare->Fill(fRealInvarMass, fInvarMass);
        // additional EMCal
        if (fGammaInEMCal) continue;
        if (!fWholeEvtDetected) fHist_emc_feedD->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==0) fHist_emc_fromCEP->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP>0)  fHist_emc_fromDiff->Fill(fInvarMass);        
        else fHist_emc_CD->Fill(fInvarMass);

    }
    fTPCITSsignalEnries += fHist_TPC_CD->GetEntries();
    fTPCITSsignalEnries += fHist_TPC_feedD->GetEntries();
    if (fShowCEPComponents) {
        fTPCITSsignalEnries += fHist_TPC_fromCEP->GetEntries();
        fTPCITSsignalEnries += fHist_TPC_fromDiff->GetEntries();
    }
}
//_______________________________________________________________________________
THistMaker::~THistMaker(void)
{
    delete fHist_TPC_CD;
    delete fHist_TPC_feedD;
    delete fHist_TPC_fromCEP;
    delete fHist_TPC_fromDiff;
    delete fHist_fwd_CD;
    delete fHist_fwd_feedD;
    delete fHist_fwd_fromCEP;
    delete fHist_fwd_fromDiff;
    delete fHist_ad_CD;
    delete fHist_ad_feedD;
    delete fHist_ad_fromCEP;
    delete fHist_ad_fromDiff;
    delete fHist_emc_CD;
    delete fHist_emc_feedD;
    delete fHist_emc_fromCEP;
    delete fHist_emc_fromDiff;

    fOutputFile->Close();
    delete fOutputFile;
    /* fFile->Close(); */
    /* delete fFile; */

    delete fOutList;
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
    Bool_t kaon = false, kapi = false, pion = false, piPro = false, pp = false,
           fourPi = false, fourKa = false;
    if (fInFile.Contains("kaon_"))        kaon = true;
    if (fInFile.Contains("pi_"))          pion = true;
    if (fInFile.Contains("piKaon_"))      kapi = true;
    if (fInFile.Contains("piProton_"))    piPro = true;
    if (fInFile.Contains("antipProton_")) pp = true;
    // we only care for one plot at a time
    TH1F hist_cd;
    TH1F hist_fd;
    TH1F hist_fromCEP;
    TH1F hist_fromDiff;
    if (mode==0){
        hist_cd       = *((TH1F*)fHist_TPC_CD->Clone());
        hist_fd       = *((TH1F*)fHist_TPC_feedD->Clone());
        hist_fromCEP  = *((TH1F*)fHist_TPC_fromCEP->Clone());
        hist_fromDiff = *((TH1F*)fHist_TPC_fromDiff->Clone());
    } else if (mode==1){
        hist_cd       = *((TH1F*)fHist_fwd_CD->Clone());
        hist_fd       = *((TH1F*)fHist_fwd_feedD->Clone());
        hist_fromCEP  = *((TH1F*)fHist_fwd_fromCEP->Clone());
        hist_fromDiff = *((TH1F*)fHist_fwd_fromDiff->Clone());
     } else if (mode==2){
        hist_cd       = *((TH1F*)fHist_ad_CD->Clone());
        hist_fd       = *((TH1F*)fHist_ad_feedD->Clone());
        hist_fromCEP  = *((TH1F*)fHist_ad_fromCEP->Clone());
        hist_fromDiff = *((TH1F*)fHist_ad_fromDiff->Clone());
     } else {
        hist_cd       = *((TH1F*)fHist_emc_CD->Clone());
        hist_fd       = *((TH1F*)fHist_emc_feedD->Clone());
        hist_fromCEP  = *((TH1F*)fHist_emc_fromCEP->Clone());
        hist_fromDiff = *((TH1F*)fHist_emc_fromDiff->Clone());
     }

    TCanvas* c = new TCanvas( ("c"+title).Data(), "c", 1500, 1000 );
    gStyle->SetOptStat(0);
    gPad->SetLogy();
    // first we make the signal histograms = add CD + feeddown (as we only regard CEP)
    Double_t xmin  = hist_cd.GetXaxis()->GetXmin();
    Double_t xmax  = hist_cd.GetXaxis()->GetXmax();
    Int_t    nBins = hist_cd.GetNbinsX(); 
    TH1F signal_cd_fd(("signal"+title).Data(), "", nBins, xmin, xmax);
    signal_cd_fd.Add(&hist_cd);
    signal_cd_fd.Add(&hist_fd);
    // scale the hsitograms 
    Double_t scale_norm = 1./double(fTPCITSsignalEnries);
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
    } else if (kapi){ 
        xTitle = "K#pi";
        yTitle = "dN/dm_{K#pi}";
    } else if (piPro) {
        xTitle = "p#pi";
        yTitle = "dN/dm_{p#pi}";
    } else if (fourPi) {
        xTitle = "4#pi";
        yTitle = "dN/dm_{4#pi}";
    } else if (fourKa) {
        xTitle = "4K";
        yTitle = "dN/dm_{4K}";
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
    Double_t add_toright = 0.06;
    /* if (kapi || piPro) add_toright = 0.06; */
    Double_t fixedLineLength = 0.5;
    Double_t lineScale = 0.3*10e-4;
    if (kaon && mode > 0) lineScale = 0.1;
    if (kapi || piPro) lineScale = 10e-5;
    Double_t xMax_phi   = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.02))*mult;
    Double_t xMax_f2    = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.27))*mult;
    Double_t xMax_f2w   = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.37))*mult;
    Double_t xMax_f2rho = signal_cd_fd.GetBinContent(signal_cd_fd.GetXaxis()->FindBin(1.56))*mult;
    if (kaon) {
        latex.DrawLatex(1.02+add_toright,  lineScale*fixedLineLength, "#phi(1020)");
        /* latex.DrawLatex(1.27+add_toright,  lineScale*fixedLineLength, "f2(1270)"); */
        /* latex.DrawLatex(1.42+add_toright,  lineScale*fixedLineLength, "f2, #omega(1420)"); */
        latex.DrawLatex(1.525+add_toright, lineScale*fixedLineLength, "f'1(1525)");
    } 
    if(kapi)  latex.DrawLatex(0.892+add_toright,lineScale*fixedLineLength,"K*(892)");
    if(piPro) latex.DrawLatex(1.115+add_toright,lineScale*fixedLineLength,"#Lambda^{0}(1115)");

    TLine l;
    l.SetLineWidth(1.);
    l.SetLineStyle(7);
    l.SetLineColor(13);
    if (kaon) {
        l.DrawLine(1.02, 0.0, 1.02, fixedLineLength);
        /* l.DrawLine(1.27, 0.0, 1.27, fixedLineLength); */
        /* l.DrawLine(1.42, 0.0, 1.42, fixedLineLength); */
        l.DrawLine(1.525, 0.0, 1.525, fixedLineLength);
    } 
    if (kapi)  l.DrawLine(0.892, 0.0, 0.892, fixedLineLength);
    if (piPro) l.DrawLine(1.115, 0.0, 1.115, fixedLineLength);

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

    c->SaveAs((outpath+title+fTitleSuffix+".pdf").Data());

    delete c;
}
//________________________________________________________________________________________
void THistMaker::Save2DMassHistInFile(TString outpath)
{
    TCanvas* c = new TCanvas("Mass comparison", "", 900, 700);
    gPad->SetLogz();
    fMassCompare->SetStats( false );
    fMassCompare->Draw("colz");
    fMassCompare->GetXaxis()->SetTitle("2p_{gen} [GeV/c^{2}]");
    fMassCompare->GetYaxis()->SetTitle("2p_{det} [GeV/c^{2}]");

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
    feedLine1.DrawLine(1.05*fXlo, 1.02*fXlo,0.95*fXhi,1.02*fXlo);
    feedLine1.DrawLine(1.05*fXlo, 1.02*fXlo,0.95*fXhi,0.93*fXhi);
    /* feedLine1.DrawLine(1.05,0.95,3.0,2.9); */

    c->SaveAs((outpath+"massComparison"+fTitleSuffix+".pdf").Data());

    delete c;
}



