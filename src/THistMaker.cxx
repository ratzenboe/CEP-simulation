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
#include <TROOT.h>
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
    fHist_CD(0), fHist_fromCEP(0), fHist_fromRho(0), fHist_fromKStar(0),
    fHist_TPC_feedDown(0), fHist_fwd_feedDown(0), fHist_ad_feedDown(0), fHist_emc_feedDown(0), 
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
    if (fInFile.Contains("fourka_"))      fTitleSuffix += "_fourKa";
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
    fHist_CD           = new TH1F("CEP_full_recon", "full recon in TPC",      nBins, xlo, xhi);
    fHist_fromCEP      = new TH1F("CEP_origin",     "parts from CEP",         nBins, xlo, xhi);
    fHist_fromRho      = new TH1F("Rho_origin",     "parts from rho",         nBins, xlo, xhi);
    fHist_fromKStar    = new TH1F("KStar_origin",   "parts from K-star",      nBins, xlo, xhi);
    // feed down histograms
    fHist_TPC_feedDown = new TH1F("onlyITS_TPC_FeedD", "only ITS TPC",        nBins, xlo, xhi);
    fHist_fwd_feedDown = new TH1F("forwardDet_FeedD", "forward detectors on", nBins, xlo, xhi);
    fHist_ad_feedDown  = new TH1F("ADDet_FeedD", "AD detectors on",           nBins, xlo, xhi);
    fHist_emc_feedDown = new TH1F("emc_FeedD", "EMCal on",                    nBins, xlo, xhi);
    // 2D mass comparison histogram
    fMassCompare           = new TH2D("Mass comparison", "AD det mass comp",
                                                                 nBins, xlo, xhi, nBins, xlo, xhi);
    for (unsigned i(0); i<fTree->GetEntries(); i++){
        fTree->GetEntry(i);
        if (fDiffrCode != 3) continue;
        if (!fHasRightParticlesInTPCITS) continue;
        // fill the TPC/ITS histograms
        if (!fWholeEvtDetected)                       fHist_TPC_feedDown->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==0)   fHist_fromCEP->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==113) fHist_fromRho->Fill(fInvarMass);
        else if (fShowCEPComponents && fFromCEP==310) fHist_fromKStar->Fill(fInvarMass);
        else                                          fHist_CD->Fill(fInvarMass);
        // fill hits in FWD 
        if (fHitInForwardDets) continue;
        if (!fWholeEvtDetected)                       fHist_fwd_feedDown->Fill(fInvarMass);
        // additional AD detector
        if (fHitInAD) continue;
        if (!fWholeEvtDetected)                       fHist_ad_feedDown->Fill(fInvarMass);
        fMassCompare->Fill(fRealInvarMass, fInvarMass);
        // additional EMCal
        if (fGammaInEMCal) continue;
        if (!fWholeEvtDetected)                       fHist_emc_feedDown->Fill(fInvarMass);
    }
    fTPCITSsignalEnries += fHist_CD->GetEntries();
    fTPCITSsignalEnries += fHist_TPC_feedDown->GetEntries();
    if (fShowCEPComponents) {
        fTPCITSsignalEnries += fHist_fromRho->GetEntries();
        fTPCITSsignalEnries += fHist_fromKStar->GetEntries();
        fTPCITSsignalEnries += fHist_fromCEP->GetEntries();
    }
}
//_______________________________________________________________________________
THistMaker::~THistMaker(void)
{
    delete fHist_CD;
    delete fHist_fromCEP;
    delete fHist_fromKStar;
    delete fHist_fromRho;
    delete fHist_TPC_feedDown;
    delete fHist_fwd_feedDown;
    delete fHist_ad_feedDown;
    delete fHist_emc_feedDown;

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
    if (fInFile.Contains("kaon_"))        kaon   = true;
    if (fInFile.Contains("pi_"))          pion   = true;
    if (fInFile.Contains("piKaon_"))      kapi   = true;
    if (fInFile.Contains("piProton_"))    piPro  = true;
    if (fInFile.Contains("antipProton_")) pp     = true;
    if (fInFile.Contains("fourpi_"))       fourPi = true;
    if (fInFile.Contains("fourka_"))       fourKa = true;
    // we only care for one plot at a time
    TH1F hist_cd  = *((TH1F*)fHist_CD->Clone());
    TH1F hist_fd;
    if      (mode==0) hist_fd = *((TH1F*)fHist_TPC_feedDown->Clone());
    else if (mode==1) hist_fd = *((TH1F*)fHist_fwd_feedDown->Clone());
    else if (mode==2) hist_fd = *((TH1F*)fHist_ad_feedDown->Clone());
    else              hist_fd = *((TH1F*)fHist_emc_feedDown->Clone());
    TH1F hist_fromCEP;
    TH1F hist_fromRho;
    TH1F hist_fromKStar;
    if (fShowCEPComponents){
        // fill fromRho, from Kstar and from CEP histograms 
        hist_fromCEP    = *((TH1F*)fHist_fromCEP->Clone());
        hist_fromRho    = *((TH1F*)fHist_fromRho->Clone());
        hist_fromKStar  = *((TH1F*)fHist_fromKStar->Clone());
    }
    /* gROOT->ForceStyle(); */
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadLeftMargin(0.15); 
    gStyle->SetOptStat(0);
    gStyle->SetTextSizePixels(215);
    gStyle->SetTextFont(42); 
 
    TCanvas* c = new TCanvas( ("c"+title).Data(), "c", 1500, 1000 );
    gPad->SetLogy();
    // first we make the signal histograms = add CD + feeddown (as we only regard CEP)
    Double_t xmin  = hist_cd.GetXaxis()->GetXmin();
    Double_t xmax  = hist_cd.GetXaxis()->GetXmax();
    Int_t    nBins = hist_cd.GetNbinsX(); 
    TH1F signal_cd_fd(("signal"+title).Data(), "", nBins, xmin, xmax);
    // in this case hist_cd are all fully recon CD evts 
    // that come from at least 2 different mother particles (pdg wise)
    signal_cd_fd.Add(&hist_cd);
    signal_cd_fd.Add(&hist_fd);
    if (fShowCEPComponents){
        signal_cd_fd.Add(&hist_fromRho); 
        signal_cd_fd.Add(&hist_fromKStar); 
        signal_cd_fd.Add(&hist_fromCEP); 
    }
    // scale the hsitograms 
    Double_t scale_norm = 1./double(fTPCITSsignalEnries);
    signal_cd_fd.Scale(scale_norm);
    hist_cd.Scale(scale_norm);
    hist_fd.Scale(scale_norm);
    if (fShowCEPComponents){
        hist_fromRho.Scale(scale_norm);
        hist_fromKStar.Scale(scale_norm);
        hist_fromCEP.Scale(scale_norm);
    } 
    // Draw the histograms
    // later we care about the style
    signal_cd_fd.Draw("EP");
    hist_fd.Draw("EP same");
    hist_cd.Draw("EP same");
    if (fShowCEPComponents){
        hist_fromRho.Draw("EP same"); 
        hist_fromKStar.Draw("EP same"); 
        hist_fromCEP.Draw("EP same"); 
    } 
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
    if (fShowCEPComponents){
        // rho
        hist_fromRho.SetMarkerStyle(kFullTriangleDown);
        hist_fromRho.SetMarkerColor(7);
        hist_fromRho.SetMarkerSize(1.2);
        // kstar
        hist_fromKStar.SetMarkerStyle(kCircle);
        hist_fromKStar.SetMarkerColor(8);
        hist_fromKStar.SetMarkerSize(1.2);
        // from CEP
        hist_fromCEP.SetMarkerStyle(kFullCross);
        hist_fromCEP.SetMarkerColor(6);
        hist_fromCEP.SetMarkerSize(1.2);
    } 

    // axis and title sizes 
    signal_cd_fd.GetYaxis()->SetLabelSize(0.045);
    signal_cd_fd.GetXaxis()->SetLabelSize(0.045);

    signal_cd_fd.GetXaxis()->SetLabelOffset(0.005);
    signal_cd_fd.GetYaxis()->SetLabelOffset(0.005);

    signal_cd_fd.GetXaxis()->SetTitleSize(0.05);
    signal_cd_fd.GetYaxis()->SetTitleSize(0.05);

    signal_cd_fd.GetYaxis()->SetTitleOffset(1.);
    signal_cd_fd.GetXaxis()->SetTitleOffset(1.);

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
    if (kapi)  latex.DrawLatex(0.892+add_toright,lineScale*fixedLineLength,"K*(892)");
    if (piPro) latex.DrawLatex(1.115+add_toright,lineScale*fixedLineLength,"#Lambda^{0}(1115)");

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

    TString fd_str      = "Feed down: ";
    TString cd_str      = "Full recon";
    TString fromCEP_str = "From CEP: ";
    TString fromRho_str = "From #rho: ";
    TString fromKS_str  = "From K^{0}_{S}: ";
    if (fShowCEPComponents) cd_str += " rest: ";
    else cd_str += ": ";
    Double_t fd_part = double(hist_fd.Integral())/double(signal_cd_fd.Integral()) *100.;
    Double_t cd_part = double(hist_cd.Integral())/double(signal_cd_fd.Integral()) *100.;     
    Double_t rho_part, kstar_part, fromCEP_part;
    if (fShowCEPComponents) {
        rho_part     = double(hist_fromRho.Integral())/double(signal_cd_fd.Integral()) *100.;
        kstar_part   = double(hist_fromKStar.Integral())/double(signal_cd_fd.Integral()) *100.;
        fromCEP_part = double(hist_fromCEP.Integral())/double(signal_cd_fd.Integral()) *100.;
    } 
    stringstream str_fd, str_cd, str_fromCEP, str_rho, str_ks;
    string fd_percent, cd_percent, fromCEP_percent, rho_percent, ks_percent;
    str_fd << fixed << setprecision(2) << fd_part;
    fd_percent = str_fd.str();
    str_cd << fixed << setprecision(2) << cd_part;
    cd_percent = str_cd.str();
    if (fShowCEPComponents){
        str_fromCEP << fixed << setprecision(2) << fromCEP_part;
        fromCEP_percent = str_fromCEP.str();
        str_rho << fixed << setprecision(2) << rho_part;
        rho_percent = str_rho.str();
        str_ks << fixed << setprecision(2) << kstar_part;
        ks_percent = str_ks.str();
    }

    Double_t xleg0 = 0.560748;
    Double_t yleg0 = 0.523172;
    Double_t xleg1 = 0.714953;
    Double_t yleg1 = 0.873326;
    if (fShowCEPComponents){
        xleg0 = 0.626168;
        yleg0 = 0.61792;
        xleg1 = 0.763017;
        yleg1 = 0.889804;
    }
    TLegend* leg = new TLegend(xleg0, yleg0, xleg1, yleg1);
    if (!fShowCEPComponents) leg->SetHeader( "#splitline{pp #sqrt{s} = 14TeV}{Pythia - MBR (#varepsilon = 0.08)}" );
    else {
        latex.SetTextSize(0.05);
        latex.SetTextColor(kBlack);
        latex.SetTextAngle(0);
        latex.DrawLatex(0.8, 0.2, "#splitline{pp #sqrt{s} = 14TeV}{Pythia - MBR (#varepsilon = 0.08)}" );
    }
    leg->SetFillStyle(0);
    leg->SetTextSize(0.04);
    leg->AddEntry( &signal_cd_fd, "CEP total", "ep"); 
    leg->AddEntry( &hist_fd, (fd_str+fd_percent+"%").Data(), "ep"); 
    if (fShowCEPComponents){
        leg->AddEntry( &hist_fromCEP,   (fromCEP_str+fromCEP_percent+"%").Data(), "ep"); 
        leg->AddEntry( &hist_fromRho,   (fromRho_str+rho_percent+"%").Data(), "ep"); 
        leg->AddEntry( &hist_fromKStar, (fromKS_str +ks_percent+"%").Data(), "ep"); 
    } 
    leg->AddEntry( &hist_cd, (cd_str+cd_percent+"%").Data(), "ep"); 
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



