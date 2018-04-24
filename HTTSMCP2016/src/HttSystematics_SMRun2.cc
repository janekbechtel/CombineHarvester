	#include "CombineHarvester/HTTSMCP2016/interface/HttSystematics_SMRun2.h"
#include <vector>
#include <string>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"

using namespace std;

namespace ch {
    
    using ch::syst::SystMap;
    using ch::syst::SystMapAsymm;
    using ch::syst::era;
    using ch::syst::channel;
    using ch::syst::bin_id;
    using ch::syst::process;
    using ch::syst::bin;
    using ch::JoinStr;
    
    void __attribute__((optimize("O0"))) AddSMRun2Systematics(CombineHarvester & cb, int control_region, bool ttbar_fit, bool no_jec_split) {
        // Create a CombineHarvester clone that only contains the signal
        // categories
        //
        // cb_sig is unused at the moment, (was it ever used in this analysis?) delete?
        //CombineHarvester cb_sig = cb.cp();
        //
        //
        
        
        std::vector<std::string> sig_procs = {"ggH_htt","qqH_htt","WH_htt","ZH_htt","ggHsm_htt", "ggHps_htt", "ggHmm_htt","qqHsm_htt", "qqHps_htt", "qqHmm_htt"};
        std::vector<std::string> ggH_sig_procs = {"ggH_htt","ggHsm_htt", "ggHps_htt", "ggHmm_htt"};
        std::vector<std::string> qqH_sig_procs = {"qqH_htt""qqHsm_htt", "qqHps_htt", "qqHmm_htt"};
        
        // N.B. when adding this list of backgrounds to a nuisance, only
        // the backgrounds that are included in the background process
        // defined in MorphingSM2016.cpp are included in the actual DCs
        // This is a list of all MC based backgrounds
        // QCD is explicitly excluded
        std::vector<std::string> all_mc_bkgs = {
            "ZL","ZLL","ZJ","ZTT","TTJ","TTT","TT",
            "W","VV","VVT","VVJ",
            "ggH_hww125","qqH_hww125","EWKZ"};
        std::vector<std::string> all_mc_bkgs_no_W = {
            "ZL","ZLL","ZJ","ZTT","TTJ","TTT","TT",
            "VV","VVT","VVJ",
            "ggH_hww125","qqH_hww125","EWKZ"};
        std::vector<std::string> all_mc_bkgs_no_TTJ = {
            "ZL","ZLL","ZJ","ZTT","TTT","TT",
            "VV","VVT","VVJ",
            "ggH_hww125","qqH_hww125","EWKZ"};
        std::vector<std::string> embed = {"EmbedZTT"};
        std::vector<std::string> real_tau_mc_bkgs = {"ZTT","TTT","TT","VV","VVT","EWKZ"};
            
        //##############################################################################
        //  lumi
        //##############################################################################
        

        cb.cp().process(JoinStr({sig_procs, {"VV","VVT","VVJ","ggH_hww125","qqH_hww125"}})).AddSyst(cb,
                                            "lumi_13TeV", "lnN", SystMap<>::init(1.025));
        
        //Add luminosity uncertainty for W in em, tt, ttbar and the mm region as norm is from MC
        cb.cp().process({"W"}).channel({"tt","em","ttbar"}).AddSyst(cb,
                                            "lumi_13TeV", "lnN", SystMap<>::init(1.025));

	    if (!ttbar_fit){
          cb.cp().process({"TT","TTT","TTJ"}).AddSyst(cb,"lumi_13TeV", "lnN", SystMap<>::init(1.025));
	    }
	    if(control_region==0){
          cb.cp().process({"W"}).channel({"et","mt"}).AddSyst(cb,
                                            "lumi_13TeV", "lnN", SystMap<>::init(1.025));    
        }
        
        //##############################################################################
        //  trigger   
        //##############################################################################
        
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs_no_W, embed})).channel({"mt"}).AddSyst(cb,
                                             "CMS_eff_trigger_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.02));
        
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs_no_W, embed})).channel({"et"}).AddSyst(cb,
                                             "CMS_eff_trigger_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.02));
        
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, embed})).channel({"em","ttbar"}).AddSyst(cb,
                                             "CMS_eff_trigger_em_$ERA", "lnN", SystMap<>::init(1.02));

        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, embed})).channel({"tt"}).AddSyst(cb,
                                            "CMS_eff_trigger_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.10));


        
        
        //##############################################################################
        //  Electron, muon and tau Id  efficiencies
        //##############################################################################
        cb.cp().AddSyst(cb, "CMS_eff_m", "lnN", SystMap<channel, process>::init
                        ({"mt"}, JoinStr({sig_procs, all_mc_bkgs_no_W}),  1.02)
                        ({"em","ttbar"}, JoinStr({sig_procs, all_mc_bkgs}),  1.02));
        
        cb.cp().AddSyst(cb, "CMS_eff_m_embedsel", "lnN", SystMap<channel, process>::init
                        ({"et","tt","em","mt"}, embed,  1.04)); 
        
        cb.cp().AddSyst(cb, "CMS_eff_e", "lnN", SystMap<channel, process>::init
                        ({"et"}, JoinStr({sig_procs, all_mc_bkgs_no_W, embed}),  1.02)
                        ({"em","ttbar"}, JoinStr({sig_procs, all_mc_bkgs, embed}),       1.02));
        

        // Tau Efficiency applied to all MC
        // in tautau channel the applied value depends on the number of taus which is determined by
        // gen match. WJets for example is assumed to have 1 real tau and 1 fake as is TTJ
        // compared to ZTT which has 2 real taus.
        // We also have channel specific components and fully correlated components
        //
        // ETau & MuTau
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, embed})).channel({"et","mt"}).AddSyst(cb,
                                             "CMS_eff_t_$ERA", "lnN", SystMap<>::init(1.045));
        
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, embed})).channel({"et","mt"}).AddSyst(cb,
                                             "CMS_eff_t_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.02));

        // TauTau - 2 real taus
        cb.cp().process(JoinStr({sig_procs, {"ZTT","VVT","TTT","EWKZ"}, embed})).channel({"tt"}).AddSyst(cb,
                                             "CMS_eff_t_$ERA", "lnN", SystMap<>::init(1.09));
        
        cb.cp().process(JoinStr({sig_procs, {"ZTT","VVT","TTT","EWKZ"}, embed})).channel({"tt"}).AddSyst(cb,
                                             "CMS_eff_t_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.04));

        // TauTau - 1+ jet to tau fakes
        cb.cp().process({"TTJ","ZJ","VVJ","W"}).channel({"tt"}).AddSyst(cb,
                                             "CMS_eff_t_$ERA", "lnN", SystMap<>::init(1.06));
        
        cb.cp().process({"TTJ","ZJ","VVJ","W"}).channel({"tt"}).AddSyst(cb,
                                             "CMS_eff_t_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.02));
        
        
        //##############################################################################
        //  b tag and mistag rate  efficiencies (update me)
        //##############################################################################
        
        cb.cp().AddSyst(cb, "CMS_htt_eff_b_$ERA", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em","et","mt"}, {1}, {"TTJ","TTT","TT"}, 1.035));
        cb.cp().AddSyst(cb, "CMS_htt_eff_b_$ERA", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em","et","mt"}, {2,3,4}, {"TTJ","TTT","TT"}, 1.05));

        cb.cp().AddSyst(cb, "CMS_htt_eff_b_$ERA", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em","et","mt"}, {2,3,4}, {"VV","VVT","VVJ"}, 1.015)); // Mainly SingleTop
        // need to update numbers for mt and et channles just set these the same as em channel for nowt
        
        //##############################################################################
        //  Electron, muon and tau energy Scale
        //##############################################################################
        
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, {"QCD"}})).channel({"em"}).AddSyst(cb,
                                             "CMS_scale_e_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process(embed).channel({"em","et"}).AddSyst(cb,
                                             "CMS_scale_e_$ERA", "shape", SystMap<>::init(1.00));
        
        cb.cp().process(embed).channel({"em","mt"}).AddSyst(cb,
                                             "CMS_scale_m_$ERA", "shape", SystMap<>::init(1.00));
        
        // Decay Mode based TES Settings
        cb.cp().process(JoinStr({sig_procs, real_tau_mc_bkgs, embed})).channel({"et","mt","tt"}).AddSyst(cb,
                                                "CMS_scale_t_1prong_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process(JoinStr({sig_procs, real_tau_mc_bkgs, embed})).channel({"et","mt","tt"}).AddSyst(cb,
                                                "CMS_scale_t_1prong1pizero_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process(JoinStr({sig_procs, real_tau_mc_bkgs, embed})).channel({"et","mt","tt"}).AddSyst(cb,
                                                "CMS_scale_t_3prong_$ERA", "shape", SystMap<>::init(1.00));
        
        //##############################################################################
        //  Embedded uncertainty on ttbar contamination
        //##############################################################################        
        // not sure we should use this uncertainty..
        //cb.cp().process({"EmbedZTT"}).AddSyst(cb,"CMS_ttbar_embeded_$ERA", "shape", SystMap<>::init(1.00));
 

        //##############################################################################
        //  jet and met energy Scale
        //##############################################################################
 
        // MET Systematic shapes (update me) - recoil uncertainties for recoil corrected met, unclustered for no recoil, jes uncertainties propogated to met
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"et","mt","tt","em"}).bin_id({1,2,3,4}).AddSyst(cb,
                                                  "CMS_scale_met_clustered_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"et","mt","em","tt"}).bin_id({1,2,3,4}).AddSyst(cb,
                                                  "CMS_scale_met_unclustered_$ERA", "shape", SystMap<>::init(1.00));
 

        // JES factorization test tautau  
        std::vector< std::string > uncertNames;
        if (!no_jec_split) {      
            uncertNames = {
              "AbsoluteFlavMap",
              "AbsoluteMPFBias",
              "AbsoluteScale",
              "AbsoluteStat",
              "FlavorQCD",
              "Fragmentation",
              "PileUpDataMC",
              "PileUpPtBB",
              "PileUpPtEC1",
              "PileUpPtEC2",
              "PileUpPtHF",
              "PileUpPtRef",
              "RelativeBal",
              "RelativeFSR",
              "RelativeJEREC1",
              "RelativeJEREC2",
              "RelativeJERHF",
              "RelativePtBB",
              "RelativePtEC1",
              "RelativePtEC2",
              "RelativePtHF",
              "RelativeStatEC",
              "RelativeStatFSR",
              "RelativeStatHF",
              "SinglePionECAL",
              "SinglePionHCAL",
              "TimePtEta"
            }; // end uncertNames
        } else { uncertNames = { "Total" }; }
        
        for (string uncert:uncertNames){
            
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"et"}).bin_id({10,11,14}).AddSyst(cb,
                                                                                                "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
        
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs_no_TTJ})).channel({"et"}).bin_id({13}).AddSyst(cb,
                                                                                                "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
            
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"mt"}).bin_id({10,11,13,14}).AddSyst(cb,
                                                                                                "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
            
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"tt"}).bin_id({10,11,12}).AddSyst(cb,
                                                                                                 "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
        
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).channel({"tt","mt","et"}).bin_id({1,2,3,4,5,6}).AddSyst(cb,
                                           "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
            cb.cp().process(JoinStr({sig_procs, all_mc_bkgs, {"QCD"}})).bin_id({1,2,3,4,5,6}).channel({"em"}).AddSyst(cb,
                                           "CMS_scale_j_"+uncert+"_$ERA", "shape", SystMap<>::init(1.00));
        }
        
        
        //// uncomment for regional JES uncertainties
        //cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).AddSyst(cb,"CMS_scale_j_eta0to5_$ERA", "shape", SystMap<>::init(1.00));
        //cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).AddSyst(cb,"CMS_scale_j_eta0to3_$ERA", "shape", SystMap<>::init(1.00));
        //cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).AddSyst(cb,"CMS_scale_j_eta3to5_$ERA", "shape", SystMap<>::init(1.00)); 
        //cb.cp().process(JoinStr({sig_procs, all_mc_bkgs})).AddSyst(cb,"CMS_scale_j_RelativeBal_$ERA", "shape", SystMap<>::init(1.00));
        
        cb.cp().AddSyst(cb,
                        "CMS_htt_scale_met_$ERA", "lnN", SystMap<channel, bin_id, process>::init
                        ({"ttbar"}, {1, 2, 3,4}, {all_mc_bkgs}, 1.01));
        
        if (control_region > 0) {
            // Add to all CRs, don't include QCD or WJets in et/mt which have CRs, or QCD in tt
            
            cb.cp().process(all_mc_bkgs).channel({"mt"}).bin_id({10,11,12,13,14,15,16,17,18,20}).AddSyst(cb,
                                                            "CMS_scale_met_clustered_$ERA", "shape", SystMap<>::init(1.00));
            cb.cp().process({"ZJ","ZTT","TTJ","TTT","W","VVT","VVJ","EWKZ"}).channel({"mt"}).bin_id({10,11,12,13,14,15,16,17,18,20}).AddSyst(cb,
                                                            "CMS_scale_met_unclustered_$ERA", "shape", SystMap<>::init(1.00));
            
            cb.cp().process({"ZL"}).channel({"mt"}).bin_id({10,11,12,13,14,15,16,17,20}).AddSyst(cb,
                                                            "CMS_scale_met_unclustered_$ERA", "shape", SystMap<>::init(1.00));
            
            cb.cp().process(all_mc_bkgs).channel({"et"}).bin_id({10,11,12,13,14,15,16,17,18,20}).AddSyst(cb,
                                                            "CMS_scale_met_clustered_$ERA", "shape", SystMap<>::init(1.00));
            cb.cp().process(all_mc_bkgs).channel({"et"}).bin_id({10,11,12,13,14,15,16,17,18,20}).AddSyst(cb,
                                                            "CMS_scale_met_unclustered_$ERA", "shape", SystMap<>::init(1.00));
        }

        
        //##############################################################################
        //  Background normalization uncertainties
        //##############################################################################
        
        //   Diboson  Normalisation - fully correlated
        cb.cp().process({"VV","VVT","VVJ"}).AddSyst(cb,
                                        "CMS_htt_vvXsec_13TeV", "lnN", SystMap<>::init(1.05));
        if (! ttbar_fit){
        //   ttbar Normalisation - fully correlated
	    cb.cp().process({"TT","TTT","TTJ"}).AddSyst(cb,
					  "CMS_htt_tjXsec_13TeV", "lnN", SystMap<>::init(1.06));}

        // W norm, just for em, tt and the mm region where MC norm is from MC
        
        cb.cp().process({"W"}).channel({"em"}).AddSyst(cb,
                                                       "CMS_htt_jetFakeLep_13TeV", "lnN", SystMap<>::init(1.20));
        
        cb.cp().process({"W"}).channel({"tt"}).AddSyst(cb,
                                                       "CMS_htt_wjXsec_13TeV", "lnN", SystMap<>::init(1.04));
        
        if(control_region==0){
          cb.cp().process({"W"}).channel({"et","mt"}).AddSyst(cb,
                                                       "CMS_htt_wjXsec_13TeV", "lnN", SystMap<>::init(1.04));    
        }
        
        // QCD norm, just for em  decorrelating QCD BG for differenet categories (update me)
        // iso->noniso extrapolation uncertainty is 1% for 0jet and 14% for dijet and boosted (from comparrison of OS/SS in (anti)iso regions in QCD MC) -> add this in quadrature to OS/SS unceratinty!
        
        cb.cp().process({"QCD"}).channel({"em"}).bin_id({1}).AddSyst(cb,
                                             "CMS_htt_QCD_0jet_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.10));
        cb.cp().process({"QCD"}).channel({"em"}).bin_id({2}).AddSyst(cb,
                                             "CMS_htt_QCD_boosted_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.10));
        cb.cp().process({"QCD"}).channel({"em"}).bin_id({3,4}).AddSyst(cb,
                                             "CMS_htt_QCD_dijet_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.20));
        
        
        // QCD norm, just for tt 
        cb.cp().process({"QCD"}).channel({"tt"}).bin_id({1}).AddSyst(cb,
                                             "CMS_htt_QCD_0jet_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.02));
        cb.cp().process({"QCD"}).channel({"tt"}).bin_id({2}).AddSyst(cb,
                                             "CMS_htt_QCD_boosted_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.04));
        cb.cp().process({"QCD"}).channel({"tt"}).bin_id({3}).AddSyst(cb,
                                             "CMS_htt_QCD_dijet_lowboost_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.08));
        cb.cp().process({"QCD"}).channel({"tt"}).bin_id({4}).AddSyst(cb,
                                             "CMS_htt_QCD_dijet_boosted_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.48));
        
        
        // QCD OS/SS uncertainty should include also extrapolation from non-iso -> iso (update me)
        cb.cp().process({"QCD"}).channel({"et","mt"}).bin_id({1,10}).AddSyst(cb,
                                             "QCD_OSSS_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.1));
        cb.cp().process({"QCD"}).channel({"et","mt"}).bin_id({2,13}).AddSyst(cb,
                                             "QCD_OSSS_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.15));
        cb.cp().process({"QCD"}).channel({"et","mt"}).bin_id({3,16}).AddSyst(cb,
                                             "QCD_OSSS_dijet_lowboost_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.20));
        cb.cp().process({"QCD"}).channel({"et","mt"}).bin_id({4}).AddSyst(cb,
                                             "QCD_OSSS_dijet_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.30));
        
        
        // based on the Ersatz study
        cb.cp().process({"W"}).channel({"et","mt"}).bin_id({1,11}).AddSyst(cb,
                                             "WHighMTtoLowMT_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.033));
        cb.cp().process({"W"}).channel({"et","mt"}).bin_id({2,14}).AddSyst(cb,
                                             "WHighMTtoLowMT_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.067));
        cb.cp().process({"W"}).channel({"et","mt"}).bin_id({3,4,17,20}).AddSyst(cb,
                                             "WHighMTtoLowMT_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.182));
        
        cb.cp().process({"W"}).channel({"et","mt"}).bin_id({4,20}).AddSyst(cb,
                                             "WlowPTtoHighPT_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.279));
        
        // W OS/SS systematic uncertainties 
        cb.cp().process({"W"}).channel({"et"}).bin_id({1,10}).AddSyst(cb,
                                             "WOSSS_syst_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.002));
        cb.cp().process({"W"}).channel({"et"}).bin_id({2,13}).AddSyst(cb,
                                             "WOSSS_syst_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.029));
        cb.cp().process({"W"}).channel({"et"}).bin_id({3,4,16}).AddSyst(cb,
                                             "WOSSS_syst_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.131));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({1,10}).AddSyst(cb,
                                             "WOSSS_syst_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.012));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({2,13}).AddSyst(cb,
                                             "WOSSS_syst_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.049));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({3,4,16}).AddSyst(cb,
                                             "WOSSS_syst_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.086));
        
        // W OS/SS statistical uncertainties
        cb.cp().process({"W"}).channel({"et"}).bin_id({1,10}).AddSyst(cb,
                                             "WOSSS_stat_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.035));
        cb.cp().process({"W"}).channel({"et"}).bin_id({2,13}).AddSyst(cb,
                                             "WOSSS_stat_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.026));
        cb.cp().process({"W"}).channel({"et"}).bin_id({3,4,16}).AddSyst(cb,
                                             "WOSSS_stat_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.082));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({1,10}).AddSyst(cb,
                                             "WOSSS_stat_0jet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.026));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({2,13}).AddSyst(cb,
                                             "WOSSS_stat_boosted_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.020));
        cb.cp().process({"W"}).channel({"mt"}).bin_id({3,4,16}).AddSyst(cb,
                                             "WOSSS_stat_dijet_$CHANNEL_$ERA", "lnN", SystMap<>::init(1.066));
        
        //##############################################################################
        //  DY LO->NLO reweighting, Between no and twice the correction.
        //##############################################################################
        
        cb.cp().process( {"ZTT","ZJ","ZL","ZLL"}).channel({"et","mt","tt"}).AddSyst(cb,
                                             "CMS_htt_dyShape_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process( {"ZTT","ZL","ZLL"}).channel({"em"}).AddSyst(cb,
                                             "CMS_htt_dyShape_$ERA", "shape", SystMap<>::init(1.00));
        
        
        //##############################################################################
        // Ttbar shape reweighting, Between no and twice the correction
        //##############################################################################
        
        cb.cp().process( {"TTJ","TTT"}).channel({"tt"}).AddSyst(cb,
                                        "CMS_htt_ttbarShape_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process( {"TTJ","TTT"}).channel({"et","mt"}).AddSyst(cb,
                                        "CMS_htt_ttbarShape_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process( {"TT"}).channel({"em"}).AddSyst(cb,
                                        "CMS_htt_ttbarShape_$ERA", "shape", SystMap<>::init(1.00));
        
        //##############################################################################
        // ZL shape  and electron/muon  to tau fake only in  mt and et channels (updated March 22)
        //##############################################################################

        
        cb.cp().process( {"ZL"}).channel({"mt","et"}).AddSyst(cb,
                                                         "CMS_ZLShape_$CHANNEL_1prong_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process( {"ZL"}).channel({"mt"}).AddSyst(cb,
                                                         "CMS_ZLShape_$CHANNEL_1prong1pizero_$ERA", "shape", SystMap<>::init(1.00));

        cb.cp().process( {"ZL"}).channel({"et"}).bin_id({1,2,3,4,10,11,12,13,14,15,16,17,18}).AddSyst(cb,
                                                         "CMS_ZLShape_$CHANNEL_1prong1pizero_$ERA", "shape", SystMap<>::init(1.00));        

        // weighted avarages of recommended tau POG uncertainties provided in bins of eta
        cb.cp().process({"ZL"}).channel({"mt"}).AddSyst(cb,
                                                        "CMS_htt_mFakeTau_13TeV", "lnN", SystMap<>::init(1.07));
        cb.cp().process({"ZL"}).channel({"et"}).AddSyst(cb,
                                                        "CMS_htt_eFakeTau_13TeV", "lnN", SystMap<>::init(1.10));

        
        //##############################################################################
        // jet  to tau fake only in tt, mt and et channels
        //##############################################################################
        
        
        cb.cp().process({"W"}).channel({"tt"}).AddSyst(cb,
                                                       "CMS_htt_jetFakeTau_13TeV", "lnN", SystMap<>::init(1.20)); 
        cb.cp().process({"VVJ","TTJ","ZJ"}).channel({"et","mt","tt"}).AddSyst(cb,
                                                       "CMS_htt_jetFakeTau_13TeV", "lnN", SystMap<>::init(1.20));
        if(control_region==0){
          cb.cp().process({"W"}).channel({"et","mt"}).AddSyst(cb,
                                                       "CMS_htt_jetFakeTau_13TeV", "lnN", SystMap<>::init(1.20));    
        }
        
        cb.cp().process( {"W"}).channel({"tt","mt","et"}).bin_id({1,2,3,4,10,11,12,13,14,15,16,17,18,20}).AddSyst(cb,
                                                                "CMS_htt_jetToTauFake_$ERA", "shape", SystMap<>::init(1.00));
        
        
        //##############################################################################
        // Theoretical Uncertainties on signal (update me)
        //##############################################################################
        // don't use acceptance uncertainties on VBF as there isn't an easy way to get these for the JHU samples (and they are expected to be small for this process)
        // For now just use the ggH uncertainties computed for the POWHEG samples - update this for MG5
        // Check ST uncertainties for ggH
        
        //scale_gg on signal
        cb.cp().process( ggH_sig_procs).channel({"et","mt","tt","em"}).AddSyst(cb,
                                             "CMS_scale_gg_$ERA", "shape", SystMap<>::init(1.00));
        
        // Scale uncertainty on signal Applies to ggH in boosted and VBF. Event-by-event weight applied as a func(on of pth or mjj. Fully correlated between categories and final states.
        
        
        // Covered by CMS_scale_gg above
        //cb.cp().AddSyst(cb, "CMS_ggH_QCDUnc", "lnN", SystMap<channel, bin_id, process>::init
        //                ({"em"},{1},ggH_sig_procs, 0.93)
        //                ({"et"},{1},ggH_sig_procs, 0.93)
        //                ({"mt"},{1},ggH_sig_procs, 0.93)
        //                ({"tt"},{1},ggH_sig_procs, 0.93)
        //                
        //                ({"em"},{2},ggH_sig_procs, 1.15)
        //                ({"et"},{2},ggH_sig_procs, 1.18)
        //                ({"mt"},{2},ggH_sig_procs, 1.18)
        //                ({"tt"},{2},ggH_sig_procs, 1.20)
        //                
        //                
        //                ({"em"},{3,4},ggH_sig_procs, 1.25)
        //                ({"et"},{3,4},ggH_sig_procs, 1.15)
        //                ({"mt"},{3,4},ggH_sig_procs, 1.08)
        //                ({"tt"},{3,4},ggH_sig_procs, 1.10)
        //                );
                        
                        
                        
//            cb.cp().AddSyst(cb, "CMS_qqH_QCDUnc", "lnN", SystMap<channel, bin_id, process>::init
//                        ({"em"},{1},qqH_sig_procs, 0.997)
//                        ({"et"},{1},qqH_sig_procs, 1.003)
//                        ({"mt"},{1},qqH_sig_procs, 0.998)
//                        ({"tt"},{1},qqH_sig_procs, 0.997)
//                        
//                        ({"em"},{2},qqH_sig_procs, 1.004)
//                        ({"et"},{2},qqH_sig_procs, 1.004)
//                        ({"mt"},{2},qqH_sig_procs, 1.002)
//                        ({"tt"},{2},qqH_sig_procs, 1.003)
//                        
//                        
//                        ({"em"},{3,4},qqH_sig_procs, 1.005)
//                        ({"et"},{3,4},qqH_sig_procs, 1.005)
//                        ({"mt"},{3,4},qqH_sig_procs, 1.002)
//                        ({"tt"},{3,4},qqH_sig_procs, 1.003)
//                        );
       
        
        
        
        cb.cp().AddSyst(cb, "CMS_ggH_PDF", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em"},{1},ggH_sig_procs, 1.007)
                        ({"et"},{1},ggH_sig_procs, 1.007)
                        ({"mt"},{1},ggH_sig_procs, 1.007)
                        ({"tt"},{1},ggH_sig_procs, 1.009)
                        
                        ({"em"},{2},ggH_sig_procs, 1.007)
                        ({"et"},{2},ggH_sig_procs, 1.007)
                        ({"mt"},{2},ggH_sig_procs, 1.007)
                        ({"tt"},{2},ggH_sig_procs, 1.009)
                        
                        
                        ({"em"},{3,4},ggH_sig_procs, 1.007)
                        ({"et"},{3,4},ggH_sig_procs, 1.007)
                        ({"mt"},{3,4},ggH_sig_procs, 1.007)
                        ({"tt"},{3,4},ggH_sig_procs, 1.009)
                        );
        
        
        
//        cb.cp().AddSyst(cb, "CMS_qqH_PDF", "lnN", SystMap<channel, bin_id, process>::init
//                        ({"em"},{1},qqH_sig_procs, 1.011)
//                        ({"et"},{1},qqH_sig_procs, 1.005)
//                        ({"mt"},{1},qqH_sig_procs, 1.005)
//                        ({"tt"},{1},qqH_sig_procs, 1.008)
//                        
//                        ({"em"},{2},qqH_sig_procs, 1.005)
//                        ({"et"},{2},qqH_sig_procs, 1.002)
//                        ({"mt"},{2},qqH_sig_procs, 1.002)
//                        ({"tt"},{2},qqH_sig_procs, 1.003)
//                        
//                        
//                        ({"em"},{3,4},qqH_sig_procs, 1.005)
//                        ({"et"},{3,4},qqH_sig_procs, 1.005)
//                        ({"mt"},{3,4},qqH_sig_procs, 1.005)
//                        ({"tt"},{3,4},qqH_sig_procs, 1.005)
//                        );
        
        
        
        
        cb.cp().AddSyst(cb, "CMS_ggH_UEPS", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em"},{1},ggH_sig_procs, 1.015)
                        ({"et"},{1},ggH_sig_procs, 1.015)
                        ({"mt"},{1},ggH_sig_procs, 1.015)
                        ({"tt"},{1},ggH_sig_procs, 1.015)
                        
                        ({"em"},{2},ggH_sig_procs, 0.945)
                        ({"et"},{2},ggH_sig_procs, 0.945)
                        ({"mt"},{2},ggH_sig_procs, 0.945)
                        ({"tt"},{2},ggH_sig_procs, 0.945)
                        
                        ({"em"},{3,4},ggH_sig_procs, 1.03)
                        ({"et"},{3,4},ggH_sig_procs, 1.03)
                        ({"mt"},{3,4},ggH_sig_procs, 1.03)
                        ({"tt"},{3,4},ggH_sig_procs, 1.03)
                        );
        
        
        
//        cb.cp().AddSyst(cb, "CMS_qqH_UEPS", "lnN", SystMap<channel, bin_id, process>::init
//                        ({"em"},{1},qqH_sig_procs, 1.015)
//                        ({"et"},{1},qqH_sig_procs, 1.015)
//                        ({"mt"},{1},qqH_sig_procs, 1.015)
//                        ({"tt"},{1},qqH_sig_procs, 1.015)
//                        
//                        ({"em"},{2},qqH_sig_procs, 0.945)
//                        ({"et"},{2},qqH_sig_procs, 0.945)
//                        ({"mt"},{2},qqH_sig_procs, 0.945)
//                        ({"tt"},{2},qqH_sig_procs, 0.945)
//                        
//                        ({"em"},{3,4},qqH_sig_procs, 1.03)
//                        ({"et"},{3,4},qqH_sig_procs, 1.03)
//                        ({"mt"},{3,4},qqH_sig_procs, 1.03)
//                        ({"tt"},{3,4},qqH_sig_procs, 1.03)
//                        );
        
        
        //    Uncertainty on BR for HTT @ 125 GeV
        cb.cp().process(sig_procs).AddSyst(cb,"BR_htt_THU", "lnN", SystMap<>::init(1.017));
        cb.cp().process(sig_procs).AddSyst(cb,"BR_htt_PU_mq", "lnN", SystMap<>::init(1.0099));
        cb.cp().process(sig_procs).AddSyst(cb,"BR_htt_PU_alphas", "lnN", SystMap<>::init(1.0062));
        
        //    Uncertainty on BR of HWW @ 125 GeV
        cb.cp().process({"ggH_hww125","qqH_hww125"}).AddSyst(cb,"BR_hww_THU", "lnN", SystMap<>::init(1.0099));
        cb.cp().process({"ggH_hww125","qqH_hww125"}).AddSyst(cb,"BR_hww_PU_mq", "lnN", SystMap<>::init(1.0099));
        cb.cp().process({"ggH_hww125","qqH_hww125"}).AddSyst(cb,"BR_hww_PU_alphas", "lnN", SystMap<>::init(1.0066));
        
        
        cb.cp().process(JoinStr({ggH_sig_procs, {"ggH_hww125"}})).AddSyst(cb,"QCDScale_ggH", "lnN", SystMap<>::init(1.039));
        cb.cp().process(JoinStr({qqH_sig_procs, {"qqH_hww125"}})).AddSyst(cb,"QCDScale_qqH", "lnN", SystMap<>::init(1.004));
        cb.cp().process({"WH_htt"}).AddSyst(cb,"QCDScale_VH", "lnN", SystMap<>::init(1.007));
        cb.cp().process({"ZH_htt"}).AddSyst(cb,"QCDScale_VH", "lnN", SystMap<>::init(1.038));
        
        cb.cp().process(JoinStr({ggH_sig_procs, {"ggH_hww125"}})).AddSyst(cb,"pdf_Higgs_gg", "lnN", SystMap<>::init(1.032));
        cb.cp().process(JoinStr({qqH_sig_procs, {"qqH_hww125"}})).AddSyst(cb,"pdf_Higgs_qq", "lnN", SystMap<>::init(1.021));
        cb.cp().process({"WH_htt"}).AddSyst(cb,"pdf_Higgs_VH", "lnN", SystMap<>::init(1.019));
        cb.cp().process({"ZH_htt"}).AddSyst(cb,"pdf_Higgs_VH", "lnN", SystMap<>::init(1.016));
        
        
        
        cb.cp().AddSyst(cb, "CMS_ggH_STXSmig01", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em"},{1},ggH_sig_procs, 0.959)
                        ({"et"},{1},ggH_sig_procs, 0.959)
                        ({"mt"},{1},ggH_sig_procs, 0.959)
                        ({"tt"},{1},ggH_sig_procs, 0.959)
                        
                        ({"em"},{2},ggH_sig_procs, 1.079)
                        ({"et"},{2},ggH_sig_procs, 1.079)
                        ({"mt"},{2},ggH_sig_procs, 1.079)
                        ({"tt"},{2},ggH_sig_procs, 1.079)
                        
                        ({"em"},{3,4},ggH_sig_procs, 1.039)
                        ({"et"},{3,4},ggH_sig_procs, 1.039)
                        ({"mt"},{3,4},ggH_sig_procs, 1.039)
                        ({"tt"},{3,4},ggH_sig_procs, 1.039)
                        );
        
        
        cb.cp().AddSyst(cb, "CMS_ggH_STXSmig12", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em"},{1},ggH_sig_procs, 1.000)
                        ({"et"},{1},ggH_sig_procs, 1.000)
                        ({"mt"},{1},ggH_sig_procs, 1.000)
                        ({"tt"},{1},ggH_sig_procs, 1.000)
                        
                        ({"em"},{2},ggH_sig_procs, 0.932)
                        ({"et"},{2},ggH_sig_procs, 0.932)
                        ({"mt"},{2},ggH_sig_procs, 0.932)
                        ({"tt"},{2},ggH_sig_procs, 0.932)
                        
                        ({"em"},{3,4},ggH_sig_procs, 1.161)
                        ({"et"},{3,4},ggH_sig_procs, 1.161)
                        ({"mt"},{3,4},ggH_sig_procs, 1.161)
                        ({"tt"},{3,4},ggH_sig_procs, 1.161)
                        );
        
        cb.cp().AddSyst(cb, "CMS_ggH_STXSVBF2j", "lnN", SystMap<channel, bin_id, process>::init
                        ({"em"},{1},ggH_sig_procs, 1.000)
                        ({"et"},{1},ggH_sig_procs, 1.000)
                        ({"mt"},{1},ggH_sig_procs, 1.000)
                        ({"tt"},{1},ggH_sig_procs, 1.000)
                        
                        ({"em"},{2},ggH_sig_procs, 1.000)
                        ({"et"},{2},ggH_sig_procs, 1.000)
                        ({"mt"},{2},ggH_sig_procs, 1.000)
                        ({"tt"},{2},ggH_sig_procs, 1.000)
                        
                        ({"em"},{3,4},ggH_sig_procs, 1.200)
                        ({"et"},{3,4},ggH_sig_procs, 1.200)
                        ({"mt"},{3,4},ggH_sig_procs, 1.200)
                        ({"tt"},{3,4},ggH_sig_procs, 1.200)
                        );
        
                        
        
        //  // Recoil corrections
        //  // ------------------
        //  // These should not be applied to the W in all control regions becasuse we should
        //  // treat it as an uncertainty on the low/high mT factor.
        //  // For now we also avoid applying this to any of the high-mT control regions
        //  // as the exact (anti-)correlation with low mT needs to be established
        //  // CHECK THIS
        //  cb.cp().AddSyst(cb,
        //    "CMS_htt_boson_scale_met_$ERA", "lnN", SystMap<channel, bin_id, process>::init
        //    ({"et", "mt", "em", "tt"}, {1, 2, 3, 4,5,6}, JoinStr({signal, {"ZTT", "W"}}), 1.02));
        //
        
        
        // Z->mumu CR normalization propagation
        // remove these and just take MC estimate? (adding back lumi and zjxs uncertainty?)
        // 0jet normalization only
        cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ", "ZLL"}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_0jet_$CHANNEL_$ERA", "lnN",
                                         SystMap<channel, bin_id>::init({"em","tt"},{1}, 1.07));
        cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ"}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_0jet_lt_$ERA", "lnN",
                                         SystMap<channel, bin_id>::init({"et","mt"},{1}, 1.07));
        
        // boosted normalization only
        cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ", "ZLL"}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_boosted_$CHANNEL_$ERA", "lnN",
                                         SystMap<channel, bin_id>::init({"em","tt"},{2}, 1.07));
        cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ"}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_boosted_lt_$ERA", "lnN",
                                         SystMap<channel, bin_id>::init({"et","mt"},{2}, 1.07));
        
        // VBF norm and shape for et/mt/tt
        cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ", "ZLL"}).channel({"em"}).bin_id({3}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_lowboost_em_$ERA", "lnN", SystMap<>::init(1.15));
        cb.cp().process( {"ZL","ZTT","ZJ",  "EWKZ"}).channel({"et","mt"}).bin_id({3}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_lowboost_lt_$ERA", "lnN", SystMap<>::init(1.15));
        cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ"}).channel({"tt"}).bin_id({3}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_lowboost_tt_$ERA", "lnN", SystMap<>::init(1.10));
        
        cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ", "ZLL"}).channel({"em"}).bin_id({4}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_boosted_em_$ERA", "lnN", SystMap<>::init(1.15));
        cb.cp().process( {"ZL","ZTT","ZJ",  "EWKZ"}).channel({"et","mt"}).bin_id({4}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_boosted_lt_$ERA", "lnN", SystMap<>::init(1.15));
        cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ"}).channel({"tt"}).bin_id({4}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_boosted_tt_$ERA", "lnN", SystMap<>::init(1.10));


        cb.cp().process( {"ZL","ZTT","ZJ","EWKZ","ZLL"}).channel({"tt","et","mt"}).bin_id({3,4}).AddSyst(cb,
                                        "CMS_htt_zmumuShape_VBF_$ERA", "shape", SystMap<>::init(1.00));
        cb.cp().process( {"ZL","ZTT","ZJ","ZLL"}).channel({"em"}).bin_id({3,4}).AddSyst(cb,
                                            "CMS_htt_zmumuShape_VBF_$ERA", "shape", SystMap<>::init(1.00));


        // Add the zmumu extrapolation uncertainties to Drell-Yan in CRs
        // if applicable
        if(control_region > 0)
        {
            // Z->mumu CR normalization propagation
            // 0jet normalization only
            cb.cp().process({"ZTT", "ZL", "ZJ","EWKZ"}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_0jet_$CHANNEL_$ERA", "lnN",
                                             SystMap<channel, bin_id>::init({"tt"},{10}, 1.07));
            cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ"}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_0jet_lt_$ERA", "lnN",
                                             SystMap<channel, bin_id>::init({"et","mt"},{10,11,12}, 1.07));
            
            // boosted normalization only
            cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ"}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_boosted_$CHANNEL_$ERA", "lnN",
                                             SystMap<channel, bin_id>::init({"tt"},{11}, 1.07));
            cb.cp().process({"ZTT", "ZL", "ZJ", "EWKZ"}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_boosted_lt_$ERA", "lnN",
                                             SystMap<channel, bin_id>::init({"et","mt"},{13,14,15}, 1.07));
            
            // VBF norm and shape for et/mt/tt
            
            cb.cp().process( {"ZL","ZTT","ZJ",  "EWKZ"}).channel({"et","mt"}).bin_id({16,17,18}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_lowboost_lt_$ERA", "lnN", SystMap<>::init(1.15));
            cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ"}).channel({"tt"}).bin_id({12}).AddSyst(cb,
                                         "CMS_htt_zmm_norm_extrap_dijet_lowboost_tt_$ERA", "lnN", SystMap<>::init(1.10));
            
            cb.cp().process( {"ZL","ZTT","ZJ",  "EWKZ"}).channel({"et","mt"}).bin_id({20}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_dijet_boosted_lt_$ERA", "lnN", SystMap<>::init(1.15));
            cb.cp().process( {"ZL","ZTT","ZJ", "EWKZ"}).channel({"tt"}).bin_id({13}).AddSyst(cb,
                                             "CMS_htt_zmm_norm_extrap_dijet_boosted_tt_$ERA", "lnN", SystMap<>::init(1.10));

            cb.cp().process( {"ZL","ZTT","ZJ"}).channel({"mt","et"}).bin_id({16,17,18,20}).AddSyst(cb,
                                             "CMS_htt_zmumuShape_VBF_$ERA", "shape", SystMap<>::init(1.00));
            cb.cp().process( {"ZL","ZTT","ZJ", "ZJ_rest", "EWKZ"}).channel({"tt"}).bin_id({12,13}).AddSyst(cb,
                                             "CMS_htt_zmumuShape_VBF_$ERA", "shape", SystMap<>::init(1.00));

        }
        
        

        if (control_region > 0) {
            // Create rateParams for control regions:
            //  - [x] 1 rateParam for all W in every region
            //  - [x] 1 rateParam for QCD in low mT
            //  - [x] 1 rateParam for QCD in high mT
            //  - [x] lnNs for the QCD OS/SS ratio
            //         * should account for stat + syst
            //         * systs should account for: extrap. from anti-iso to iso region,
            //  - [x] lnNs for the W+jets OS/SS ratio
            //         * should account for stat only if not being accounted for with bbb,
            //           i.e. because the OS/SS ratio was measured with a relaxed selection
            //         * systs should account for: changes in low/high mT and OS/SS due to experimental effects e.g JES,
            //           OS/SS being wrong in the MC 
            //           low/high mT being wrong in the MC 
            
            // W rate params added for all et/mt signal-regions/control-regions. The same W rate param is used for dijet_lowboost and dijet_boosted since the W for the dijet_boosted category is estimated using dijet_lowboost control-region and extraplated to dijet_boosted using MC
            cb.cp().bin({"mt_0jet","mt_0jet_wjets_cr","mt_0jet_qcd_cr","mt_0jet_wjets_ss_cr"}).process({"W"}).AddSyst(cb, "rate_W_mt_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"mt_boosted","mt_boosted_wjets_cr","mt_boosted_qcd_cr","mt_boosted_wjets_ss_cr"}).process({"W"}).AddSyst(cb, "rate_W_mt_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"mt_dijet_lowboost","mt_dijet_lowboost_wjets_cr","mt_dijet_lowboost_qcd_cr","mt_dijet_lowboost_wjets_ss_cr","mt_dijet_boosted","mt_dijet_boosted_qcd_cr"}).process({"W"}).AddSyst(cb, "rate_W_mt_dijet", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_0jet","et_0jet_wjets_cr","et_0jet_qcd_cr","et_0jet_wjets_ss_cr"}).process({"W"}).AddSyst(cb, "rate_W_et_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"et_boosted","et_boosted_wjets_cr","et_boosted_qcd_cr","et_boosted_wjets_ss_cr"}).process({"W"}).AddSyst(cb, "rate_W_et_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_lowboost","et_dijet_lowboost_wjets_cr","et_dijet_lowboost_qcd_cr","et_dijet_lowboost_wjets_ss_cr","et_dijet_boosted","et_dijet_boosted_qcd_cr"}).process({"W"}).AddSyst(cb, "rate_W_et_dijet", "rateParam", SystMap<>::init(1.0));
            
            // QCD rate params added for low-mT region   
            cb.cp().bin({"mt_0jet","mt_0jet_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_mt_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"mt_boosted","mt_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_mt_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"mt_dijet_lowboost","mt_dijet_lowboost_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_mt_dijet_lowboost", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"mt_dijet_boosted","mt_dijet_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_mt_dijet_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_0jet","et_0jet_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_et_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"et_boosted","et_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_et_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_lowboost","et_dijet_lowboost_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_et_dijet_lowboost", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_boosted","et_dijet_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_lowmT_et_dijet_boosted", "rateParam", SystMap<>::init(1.0));
            
            // QCD rate params added for high-mT regions    
            cb.cp().bin({"mt_0jet_wjets_cr","mt_0jet_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_mt_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"mt_boosted_wjets_cr","mt_boosted_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_mt_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"mt_dijet_lowboost_wjets_cr","mt_dijet_lowboost_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_mt_dijet_lowboost", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_0jet_wjets_cr","et_0jet_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_et_0jet", "rateParam", SystMap<>::init(1.0)); 
            cb.cp().bin({"et_boosted_wjets_cr","et_boosted_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_et_boosted", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_lowboost_wjets_cr","et_dijet_lowboost_wjets_ss_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_highmT_et_dijet_lowboost", "rateParam", SystMap<>::init(1.0));

            // tt QCD rate params
            cb.cp().bin({"tt_0jet","tt_0jet_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_cr_0jet_tt", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_boosted","tt_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_cr_boosted_tt", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_dijet_lowboost","tt_dijet_lowboost_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_cr_dijet_lowboost_tt", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_dijet_boosted","tt_dijet_boosted_qcd_cr"}).process({"QCD"}).AddSyst(cb, "rate_QCD_cr_dijet_boosted_tt", "rateParam", SystMap<>::init(1.0));  

            // Should set a sensible range for our rateParams
            for (auto sys : cb.cp().syst_type({"rateParam"}).syst_name_set()) {
                cb.GetParameter(sys)->set_range(0.0, 5.0);
            }
        }

        
        if (ttbar_fit) {
            cb.SetFlag("filters-use-regex", true);
            
            cb.cp().bin({"mt_0jet"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_0jet"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_0jet"}).process({"TTJ","TTT","TTJ_rest"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"em_0jet"}).process({"TT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0)); 
            
            cb.cp().bin({"mt_boosted"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_boosted"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_boosted"}).process({"TTJ","TTT","TTJ_rest"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"em_boosted"}).process({"TT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            
            cb.cp().bin({"mt_dijet_lowboost"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_lowboost"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_dijet_lowboost"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"em_dijet_lowboost"}).process({"TT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            
            cb.cp().bin({"mt_dijet_boosted"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"et_dijet_boosted"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"tt_dijet_boosted"}).process({"TTJ","TTT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            cb.cp().bin({"em_dijet_boosted"}).process({"TT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            
            cb.cp().bin({"em_ttbar"}).process({"TT"}).AddSyst(cb, "rate_ttbar", "rateParam", SystMap<>::init(1.0));
            
            cb.GetParameter("rate_ttbar")->set_range(0.80, 1.20);
            
            cb.SetFlag("filters-use-regex", false);
        }
        
    }
}