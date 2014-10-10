//
// This is an example steering macro for running RSN analysis task
// in the VAF analysis facility to launch a multiple analysis.
//
//
// Notes:
//
//   - in case the source is an ESD, and if inputs are a MC production
//     the MC input handler is created by default
//
//
void runVAFCascade
(
   Bool_t   eventselectionOff       = kFALSE,        // kTRUE if no selction on events is needed
   Bool_t   useMC		    = kFALSE,	    // kTRUE if analysing a MC sample 
   Bool_t   relaunchV0CascVertexers = kTRUE,	   // kTRUE if re-launch V0/cascade vertexers 
   Float_t  centrlowlim 	    = 0.,	    //
   Float_t  centruplim  	    = 90.,	    //
   TString  centrest		    = "V0M",	    // pPb:"V0A" or PbPb:"V0M" 
   Bool_t   kusecleaning	    = kTRUE,	    //
   Float_t  vtxlim		    = 10.,	    //
   Int_t    minnTPCcls  	    = 70,	    //
   TString  collidingSystem	    = "PbPb",	    // "pp", "pPb" or "PbPb"
   Bool_t   SDDSelection	    = kFALSE,	    // only for pp@2.76TeV
   Bool_t   withSDD		    = kFALSE,	    // only for pp@2.76TeV
   Float_t  minptondaughtertracks   = 0.,	    //
   Float_t  etacutondaughtertracks  = .8,	    //
   TString  anatype		    = "AOD",	    //"AOD" or "ESD"
   TString  gridoutputdir	    = "test",	    //
   const char *plugin_mode	    = "test"
)
{


   TString  datadir = "";
   TString  datapattern = "";
   if (!useMC) {
       if (collidingSystem == "pp") {
           datadir     = "/alice/data/2011/LHC11a/000146746";
           if      (anatype == "AOD") datapattern = "ESDs/pass4_with_SDD/AOD113/*/AliAOD.root";
           else if (anatype == "ESD") datapattern = "ESDs/pass4_with_SDD/*/*ESDs.root";
       } else if (collidingSystem == "pPb") {
           datadir     = "/alice/data/2013/LHC13c/000195677";
           if      (anatype == "AOD") datapattern = "ESDs/pass2/AOD139/*/AliAOD.root";
           else if (anatype == "ESD") datapattern = "ESDs/pass2/*/*ESDs.root"; 
       } else if (collidingSystem == "PbPb") {
           datadir     = "/alice/data/2010/LHC10h/000139510";
           if      (anatype == "AOD") datapattern = "ESDs/pass2/AOD086/*/root_archive.zip";
           else if (anatype == "ESD") datapattern = "ESDs/pass2/*/*ESDs.root";
       } 
   } else {
       if (collidingSystem == "pp") {
           datadir     = "/alice/sim/LHC10f6a/126437";
           if      (anatype == "AOD") datapattern = "/AOD161/*AOD.root";
           else if (anatype == "ESD") datapattern = "*ESDs.root";
       } else if (collidingSystem == "pPb") {
           datadir     = "/alice/sim/2013/LHC13d3/195677"; 
           if      (anatype == "AOD") datapattern = "/AOD159/*AOD.root"; 
           else if (anatype == "ESD") datapattern = "*ESDs.root"; 
       } else if (collidingSystem == "PbPb") {
           datadir     = "/alice/sim/LHC11a10b_plus/139510"; 
           if      (anatype == "AOD") datapattern = "AOD090/*AOD.root"; 
           else if (anatype == "ESD") datapattern = "*ESDs.root";
       }
   }

   TString proofDataSet = "Find;BasePath="; proofDataSet+=datadir; proofDataSet+=";FileName=*"; proofDataSet+=datapattern; proofDataSet+=";Anchor=AliAOD.root";

   
   gSystem->AddIncludePath("-I$ALICE_ROOT/include");
   
   TList *list = new TList();
   list->Add(new TNamed("ALIROOT_MODE", "AliRoot"));

   // Here extra AliRoot libs
   TString
  
alilibs("Core:Proof:Tree:Geom:Physics:VMC:Eve:Minuit:Gui:XMLParser:RooFit:STEER:STEERBase:ANALYSIS:ESD:AOD:ANALYSIS:ANALYSISalice:CDB:RAWDatabase:STEER:libEventMixing:CORRFW:libPWGLFSTRANGENESS");
   

   TString libs("");
   libs.Form("%s", alilibs.Data());
   list->Add(new TNamed("ALIROOT_EXTRA_LIBS", libs.Data()));
  
   // Here extra include path
   TString includepath=("$ALICE_ROOT/include:$ALICE_ROOT/TOF"); 
   list->Add(new TNamed("ALIROOT_EXTRA_INCLUDES", includepath.Data()));
   //list->Add(new TNamed("ALIROOT_ENABLE_ALIEN", "1"));  // important: creates token on every PROOF worker

   // Open PROOF connection
   TProof::Open("pod://");
   
    // Check the dataset before running the analysis!
   gProof->ShowDataSet( proofDataSet );
  
   // Upload and enable package
   gProof->UploadPackage("AliRoot.par");
   gProof->EnablePackage("AliRoot.par", list);   


   // Define dataset
   TString dataset=(proofDataSet);
   
 

   //
   // === ANALYSIS EXECUTION =======================================================================
   //
   
   TStopwatch t;
   t.Start();
 
  //__________________________________________________________________________
  // Create the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("CascadeQAanalysis");
  
  //__________________________________________________________________________

  // Input handlers
  AliESDInputHandler* esdH = new AliESDInputHandler();
  AliAODInputHandler* aodH = new AliAODInputHandler();
  if      (anatype=="ESD") mgr->SetInputEventHandler(esdH);
  else if (anatype=="AOD") mgr->SetInputEventHandler(aodH);
  if (useMC && (anatype=="ESD")) {
    AliMCEventHandler* mcHandler = new AliMCEventHandler();
    mgr->SetMCtruthEventHandler(mcHandler);
  }

  //__________________________________________________________________________
  // Add tasks
  // -- Physics and centrality Task (only for PbPb (ESD) events)
  if (anatype=="ESD" && !eventselectionOff) {
      // Physics selection
      gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPhysicsSelection.C");
      AliPhysicsSelectionTask *physSel = AddTaskPhysicsSelection(useMC);
      // Centrality selection
      gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskCentrality.C");
      AliCentralitySelectionTask *taskCentr = AddTaskCentrality();
      if (useMC){
          taskCentr->SetMCInput();
          taskCentr->DontUseCleaning();
      }
  }
  // -- PID Task
  gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
  AliAnalysisTaskPIDResponse *pidTask = AddTaskPIDResponse(useMC);
  // -- Analysis Task
  gProof->Load("./AliAnalysisTaskQAMultistrange.cxx++g");
  gProof->Load("./AddTaskQAMultistrange.C");
  AliAnalysisTaskQAMultistrange *task = AddTaskQAMultistrange(eventselectionOff, useMC, relaunchV0CascVertexers, minnTPCcls, centrlowlim, centruplim, centrest, kusecleaning, vtxlim, collidingSystem, SDDSelection, withSDD, minptondaughtertracks, etacutondaughtertracks);

  //__________________________________________________________________________
  // Disbale debug printouts
  //mgr->SetDebugLevel(3);
  //AliLog::SetGlobalLogLevel(AliLog::kFatal);
  //AliLog::SetGlobalDebugLevel(0);
   
   // initialize and start analysis
   if (!mgr->InitAnalysis()) {
      ::Error("runVAF.C", "Failed to init analysis");
      return;
   }
   mgr->PrintStatus();
  
   mgr->StartAnalysis("proof",proofDataSet.Data());
   
   t.Stop();
   t.Print();
}
