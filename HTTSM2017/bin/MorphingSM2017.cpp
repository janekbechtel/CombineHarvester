#include <string>
#include <map>
#include <set>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdlib>
#include "boost/algorithm/string/predicate.hpp"
#include "boost/program_options.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
#include "CombineHarvester/CombineTools/interface/Algorithm.h"
#include "CombineHarvester/CombineTools/interface/AutoRebin.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "CombineHarvester/HTTSM2017/interface/HttSystematics_SMRun2.h"
#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "TH2.h"
#include "TF1.h"

using namespace std;
using boost::starts_with;
namespace po = boost::program_options;

template <typename T>
void To1Bin(T* proc)
{
    std::unique_ptr<TH1> originalHist = proc->ClonedScaledShape();
    TH1F *hist = new TH1F("hist","hist",1,0,1);
    double err = 0;
    double rate =
    originalHist->IntegralAndError(0, originalHist->GetNbinsX() + 1, err);
    hist->SetDirectory(0);
    hist->SetBinContent(1, rate);
    hist->SetBinError(1, err);
    proc->set_shape(*hist, true);  // True means adjust the process rate to the
    // integral of the hist
}

bool BinIsControlRegion(ch::Object const* obj)
{
    return (boost::regex_search(obj->bin(),boost::regex{"_cr$"}) || (obj->channel() == std::string("mm")));
}

// Useful to have the inverse sometimes too
bool BinIsNotControlRegion(ch::Object const* obj)
{
    return !BinIsControlRegion(obj);
}

int main(int argc, char** argv) {
    // Define program options
    string output_folder = "sm_run2";
    string input_folder_em="Vienna/";
    string input_folder_et="Vienna/";
    string input_folder_mt="Vienna/";
    string input_folder_tt="Vienna/";
    string input_folder_mm="Vienna/";
    string input_folder_ttbar="Vienna/";
    string chan="all";
    string postfix="-ML";
    bool auto_rebin = false;
    bool manual_rebin = false;
    bool real_data = false;
    bool mm_fit = false;
    bool ttbar_fit = false;
    bool do_jetfakes = true;
    po::variables_map vm;
    po::options_description config("configuration");
    config.add_options()
        ("input_folder_em", po::value<string>(&input_folder_em)->default_value("Vienna"))
        ("input_folder_et", po::value<string>(&input_folder_et)->default_value("Vienna"))
        ("input_folder_mt", po::value<string>(&input_folder_mt)->default_value("Vienna"))
        ("input_folder_tt", po::value<string>(&input_folder_tt)->default_value("Vienna"))
        ("postfix", po::value<string>(&postfix)->default_value(postfix))
        ("channel", po::value<string>(&chan)->default_value(chan))
        ("auto_rebin", po::value<bool>(&auto_rebin)->default_value(auto_rebin))
        ("real_data", po::value<bool>(&real_data)->default_value(real_data))
        ("manual_rebin", po::value<bool>(&manual_rebin)->default_value(manual_rebin))
        ("output_folder", po::value<string>(&output_folder)->default_value(output_folder))
        ("jetfakes", po::value<bool>(&do_jetfakes)->default_value(do_jetfakes));
    po::store(po::command_line_parser(argc, argv).options(config).run(), vm);
    po::notify(vm);

    typedef vector<string> VString;
    typedef vector<pair<int, string>> Categories;

    //! [part1]
    // First define the location of the "auxiliaries" directory where we can
    // source the input files containing the datacard shapes
    std::map<string, string> input_dir;
    input_dir["em"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_em+"/";
    input_dir["mt"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_mt+"/";
    input_dir["et"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_et+"/";
    input_dir["tt"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_tt+"/";
    input_dir["mm"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_mm+"/";
    input_dir["ttbar"]  = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTSM2017/shapes/"+input_folder_ttbar+"/";

    VString chns;
    if ( chan.find("mt") != std::string::npos ) chns.push_back("mt");
    if ( chan.find("et") != std::string::npos ) chns.push_back("et");
    if ( chan.find("em") != std::string::npos ) chns.push_back("em");
    if ( chan.find("tt") != std::string::npos ) chns.push_back("tt");
    if ( chan=="all" ) chns = {"mt","et","tt","em"};

    map<string, VString> bkg_procs;
    if (do_jetfakes){
      bkg_procs["et"] = {"ZTT",   "ZL", "TTT", "VVT", "EWKZ", "jetFakes"};
      bkg_procs["mt"] = {"ZTT",   "ZL", "TTT", "VVT", "EWKZ", "jetFakes"};
      bkg_procs["tt"] = {"ZTT",   "ZL", "TTT", "VVT", "EWKZ", "jetFakes"};
    }else{
      bkg_procs["et"] = {"W", "ZTT",   "QCD", "ZL", "ZJ","TTT","TTJ","VVJ","VVT"};
      bkg_procs["mt"] = {"W", "ZTT",   "QCD", "ZL", "ZJ","TTT","TTJ","VVJ","VVT"};
      bkg_procs["tt"] = {"W", "ZTT",   "QCD", "ZL", "ZJ","TTT","TTJ"};
    }
    bkg_procs["em"] = {"ZTT", "W", "QCD", "ZL", "TT", "VV", "EWKZ", "ggH_hww125", "qqH_hww125"};
    bkg_procs["mm"] = {"W", "ZL", "TT", "VV"};
    bkg_procs["ttbar"] = {"ZTT", "W", "QCD", "ZL", "TT", "VV", "EWKZ"};

    vector<double> binning = {0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};

    ch::CombineHarvester cb;

    map<string,Categories> cats;
    cats["et"] = {
        { 2, "et_ggh"},
        { 3, "et_qqh"},
        {11, "et_w"},
        {12, "et_ztt"},
        {13, "et_tt"},
        {14, "et_ss"},
        {15, "et_zll"},
        {16, "et_misc"},
    };

    cats["mt"] = {
        { 2, "mt_ggh"},
        { 3, "mt_qqh"},
        {11, "mt_w"},
        {12, "mt_ztt"},
        {13, "mt_tt"},
        {14, "mt_ss"},
        {15, "mt_zll"},
        {16, "mt_misc"},
    };

    cats["em"] = {
        { 2, "em_ggh"},
        { 3, "em_qqh"},
        {11, "em_w"},
        {12, "em_ztt"},
        {13, "em_tt"},
        {14, "em_ss"},
        {15, "em_zll"},
        {16, "em_misc"},
    };

    cats["tt"] = {
        { 2, "tt_ggh"},
        { 3, "tt_qqh"},
        {12, "tt_ztt"},
        {14, "tt_ss"},
        {16, "tt_misc"},
    };

    cats["mm"] = {
        {1, "mm_0jet"},
        {2, "mm_1jet"},
        {3, "mm_vbf"}
    };
    cats["ttbar"] = {
        {1, "ttbar_all"}
    };

    // Or equivalently, specify the mass points explicitly:
    vector<string> sig_procs = {"ggH","qqH"};
    vector<string> masses = {"125"};

    using ch::syst::bin_id;

    //! [part2]
    for (auto chn : chns) {
        cb.AddObservations({"*"}, {"htt"}, {"13TeV"}, {chn}, cats[chn]);
        cb.AddProcesses(   {"*"}, {"htt"}, {"13TeV"}, {chn}, bkg_procs[chn], cats[chn], false);
        cb.AddProcesses(masses,   {"htt"}, {"13TeV"}, {chn}, sig_procs, cats[chn], true);
    }

    int dummy_control_region = 0;
    ch::AddSMRun2Systematics(cb, dummy_control_region, mm_fit, ttbar_fit);

    //! [part7]
    for (string chn:chns){
        cb.cp().channel({chn}).backgrounds().ExtractShapes(
                                                            input_dir[chn] + "htt_"+chn+".inputs-sm-13TeV"+postfix+".root",
                                                            "$BIN/$PROCESS",
                                                            "$BIN/$PROCESS_$SYSTEMATIC");
        cb.cp().channel({chn}).process(sig_procs).ExtractShapes(
                                                            input_dir[chn] + "htt_"+chn+".inputs-sm-13TeV"+postfix+".root",
                                                            "$BIN/$PROCESS$MASS",
                                                            "$BIN/$PROCESS$MASS_$SYSTEMATIC");
    }

    //Now delete processes with 0 yield
    cb.FilterProcs([&](ch::Process *p) {
        bool null_yield = !(p->rate() > 0. || BinIsControlRegion(p));
        if (null_yield){
            std::cout << "[Null yield] Removing process with null yield: \n ";
            std::cout << ch::Process::PrintHeader << *p << "\n";
            cb.FilterSysts([&](ch::Systematic *s){
                bool remove_syst = (MatchingProcess(*p,*s));
                return remove_syst;
            });
        }
        return null_yield;
    });

    //Replacing observation with the sum of the backgrounds (asimov) - nice to ensure blinding
    // useful to be able to check this, so don't do the replacement
    // for these
    if(!real_data){
      for (auto b : cb.cp().FilterAll(BinIsControlRegion).bin_set()) {
	std::cout << " - Replacing data with asimov in bin " << b << "\n";
	cb.cp().bin({b}).ForEachObs([&](ch::Observation *obs) {
            obs->set_shape(cb.cp().bin({b}).backgrounds().GetShape()+cb.cp().bin({b}).signals().GetShape(), true);
          });
      }
    }

    if(manual_rebin) {
      for(auto b : cb.cp().FilterAll(BinIsControlRegion).bin_set()) {
	std::cout << "Rebinning by hand for bin: " << b <<  std::endl;
	cb.cp().bin({b}).VariableRebin(binning);
      }
    }

    // At this point we can fix the negative bins
    std::cout << "Fixing negative bins\n";
    cb.ForEachProc([](ch::Process *p) {
	if (ch::HasNegativeBins(p->shape())) {
	  auto newhist = p->ClonedShape();
	  ch::ZeroNegativeBins(newhist.get());
	  p->set_shape(std::move(newhist), false);
	}
      });

    cb.ForEachSyst([](ch::Systematic *s) {
	if (s->type().find("shape") == std::string::npos) return;
	if (ch::HasNegativeBins(s->shape_u()) || ch::HasNegativeBins(s->shape_d())) {
	  auto newhist_u = s->ClonedShapeU();
	  auto newhist_d = s->ClonedShapeD();
	  ch::ZeroNegativeBins(newhist_u.get());
	  ch::ZeroNegativeBins(newhist_d.get());
	  s->set_shapes(std::move(newhist_u), std::move(newhist_d), nullptr);
	}
      });

    //! [part8]
    auto bbb = ch::BinByBinFactory()
    .SetAddThreshold(0.05)
    .SetMergeThreshold(0.8)
    .SetFixNorm(false);
    bbb.MergeBinErrors(cb.cp().backgrounds());
    bbb.AddBinByBin(cb.cp().backgrounds(), cb);

    // This function modifies every entry to have a standardised bin name of
    // the form: {analysis}_{channel}_{bin_id}_{era}
    // which is commonly used in the htt analyses
    ch::SetStandardBinNames(cb);

    //Write out datacards. Naming convention important for rest of workflow. We
    //make one directory per chn-cat, one per chn and cmb. In this code we only
    //store the individual datacards for each directory to be combined later, but
    //note that it's also possible to write out the full combined card with CH
    string output_prefix = "output/";
    ch::CardWriter writer(output_prefix + output_folder + "/$TAG/$MASS/$BIN.txt",
                          output_prefix + output_folder + "/$TAG/common/htt_input.root");

    // We're not using mass as an identifier - which we need to tell the CardWriter
    // otherwise it will see "*" as the mass value for every object and skip it
    //    writer.SetWildcardMasses({});
    //    writer.SetVerbosity(1);

    // Write datacards
    writer.WriteCards("cmb", cb);
    cb.PrintAll();
    cout << " done\n";
}