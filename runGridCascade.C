/// **** to run the macro: ***********
// alien-token-init username
// source /tmp/gclient_env_XXX
// root -l runGridCascade.C

class AliAnalysisGrid;


void runGridCascade( Bool_t   eventselectionOff       = kFALSE,        // kTRUE if no selction on events is needed
                     Bool_t   useMC                   = kFALSE,       // kTRUE if analysing a MC sample 
                     Bool_t   relaunchV0CascVertexers = kTRUE,       // kTRUE if re-launch V0/cascade vertexers 
                     Float_t  centrlowlim             = 0.,           //
                     Float_t  centruplim              = 90.,          //
                     TString  centrest                = "V0M",        // pPb:"V0A" or PbPb:"V0M" 
                     Bool_t   kusecleaning            = kTRUE,        //
                     Float_t  vtxlim                  = 10.,          //
                     Int_t    minnTPCcls              = 70,           //
                     TString  collidingSystem         = "PbPb",       // "pp", "pPb" or "PbPb"
                     Bool_t   SDDSelection            = kFALSE,       // only for pp@2.76TeV
                     Bool_t   withSDD                 = kFALSE,       // only for pp@2.76TeV
                     Float_t  minptondaughtertracks   = 0.,           //
                     Float_t  etacutondaughtertracks  = .8,           //
                     TString  anatype                 = "ESD",        //"AOD" or "ESD"
                     TString  gridoutputdir           = "test",       //
                     const char *plugin_mode          = "test") {



   TString  datadir = "";
   TString  datapattern = "";
   if (!useMC) {
       if (collidingSystem == "pp") {
           datadir     = "/alice/data/2011/LHC11a";
           if      (anatype == "AOD") datapattern = "ESDs/pass4_with_SDD/AOD113/*/AliAOD.root";
           else if (anatype == "ESD") datapattern = "ESDs/pass4_with_SDD/*/*ESDs.root";
       } else if (collidingSystem == "pPb") {
           datadir     = "/alice/data/2013/LHC13c/";
           if      (anatype == "AOD") datapattern = "ESDs/pass2/AOD139/*/AliAOD.root";
           else if (anatype == "ESD") datapattern = "ESDs/pass2/*/*ESDs.root"; 
       } else if (collidingSystem == "PbPb") {
           datadir     = "/alice/data/2010/LHC10h";
           if      (anatype == "AOD") datapattern = "ESDs/pass2/AOD086/*/AliAOD.root";
           else if (anatype == "ESD") datapattern = "ESDs/pass2/*/*ESDs.root";
       } 
   } else {
       if (collidingSystem == "pp") {
           datadir     = "/alice/sim/LHC10f6a";
           if      (anatype == "AOD") datapattern = "/AOD161/*AOD.root";
           else if (anatype == "ESD") datapattern = "*ESDs.root";
       } else if (collidingSystem == "pPb") {
           datadir     = "/alice/sim/2013/LHC13d3"; 
           if      (anatype == "AOD") datapattern = "/AOD159/*AOD.root"; 
           else if (anatype == "ESD") datapattern = "*ESDs.root"; 
       } else if (collidingSystem == "PbPb") {
           datadir     = "/alice/sim/LHC11a10b_plus"; 
           if      (anatype == "AOD") datapattern = "AOD090/*AOD.root"; 
           else if (anatype == "ESD") datapattern = "*ESDs.root";
       }
   }


  // Load common libraries
  gSystem->Load("libCore.so");
  gSystem->Load("libTree.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("libVMC.so");
  gSystem->Load("libPhysics.so");
  gSystem->Load("libMinuit.so"); 
  gSystem->Load("libGui.so");
  gSystem->Load("libXMLParser.so");
  gSystem->Load("libSTEERBase.so");
  gSystem->Load("libESD.so");
  gSystem->Load("libCDB.so");
  gSystem->Load("libAOD");
  gSystem->Load("libANALYSIS");
  gSystem->Load("libANALYSISalice");
  gSystem->Load("libCORRFW");
  gSystem->Load("libProof.so");
  gSystem->Load("libRAWDatabase.so");
  gSystem->Load("libSTEER.so");

  //__________________________________________________________________________
  // Use AliRoot includes to compile our task
  gROOT->ProcessLine(".include $ALICE_ROOT/include");
  
  //__________________________________________________________________________
  // Create and configure the alien handler plugin
  AliAnalysisGrid *alienHandler = CreateAlienHandler(plugin_mode, eventselectionOff, useMC, anatype, gridoutputdir, datadir, datapattern,collidingSystem);
  if (!alienHandler) return;
 
  //__________________________________________________________________________
  // Create the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("CascadeQAanalysis");
  
  //__________________________________________________________________________
  // Connect plug-in to the analysis manager
  mgr->SetGridHandler(alienHandler);

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
  gROOT->LoadMacro("./AliAnalysisTaskQAMultistrange.cxx++g");
  gROOT->LoadMacro("./AddTaskQAMultistrange.C");
  AliAnalysisTaskQAMultistrange *task = AddTaskQAMultistrange(eventselectionOff, useMC, relaunchV0CascVertexers, minnTPCcls, centrlowlim, centruplim, centrest, kusecleaning, vtxlim, collidingSystem, SDDSelection, withSDD, minptondaughtertracks, etacutondaughtertracks);

  //__________________________________________________________________________
  // Disbale debug printouts
  mgr->SetDebugLevel(3);
  AliLog::SetGlobalLogLevel(AliLog::kFatal);
  AliLog::SetGlobalDebugLevel(0);
  
  //__________________________________________________________________________
  if (!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  // Start analysis in grid.
  mgr->StartAnalysis("grid");
};




AliAnalysisGrid* CreateAlienHandler(const char *plugin_mode, 
                                    Bool_t  eventselectionOff,
                                    Bool_t  useMC, 
                                    TString anatype, 
                                    TString gridoutputdir, 
                                    TString datadir, 
                                    TString datapattern,
                                    TString  collidingSystem 
                                    ) {

  //__________________________________________________________________________
  // Check if user has a valid token, otherwise make one. This has limitations.
  // One can always follow the standard procedure of calling alien-token-init then
  //   source /tmp/gclient_env_$UID in the current shell.
  AliAnalysisAlien *plugin= new AliAnalysisAlien();

  //__________________________________________________________________________
  // Set the run mode (can be "full", "test", "offline", "submit" or "terminate")
  plugin->SetRunMode(plugin_mode);
  plugin->SetNtestFiles(1);

  //__________________________________________________________________________
  // Set versions of used packages
  plugin->SetAPIVersion("V1.1x");

  //__________________________________________________________________________
  // On GRID - current
  plugin->SetROOTVersion("v5-34-08-6");
  plugin->SetAliROOTVersion("vAN-20140605"); 

  //__________________________________________________________________________
  // Declare input data to be processed.
  // Method 1: Create automatically XML collections using alien 'find' command.
  if (useMC) {
    //--------- old
    //plugin->SetGridDataDir("/alice/sim/LHC11a10a_bis");   // Define production directory
    // Set data search pattern
    //if (anatype == "ESD") plugin->SetDataPattern("*ESDs.root");
    //else plugin->SetDataPattern("AOD090/*AOD.root");  
    //plugin->AddRunNumber(137124);
    //--------- new
    plugin->SetGridDataDir(datadir.Data()); 
    plugin->SetDataPattern(datapattern.Data());
    if      (collidingSystem == "pPb")  plugin->AddRunNumber(195593);
    else if (collidingSystem == "PbPb") plugin->AddRunNumber(137752);
    else if (collidingSystem == "pp")   plugin->AddRunNumber(126424);
  } else {
    plugin->SetGridDataDir(datadir.Data());      // Define production directory LFN
    plugin->SetDataPattern(datapattern.Data());  // Set data search pattern
    plugin->SetRunPrefix("000");
    if      (collidingSystem == "pPb")  plugin->AddRunNumber(195568);
    else if (collidingSystem == "PbPb") plugin->AddRunNumber(138534);
    else if (collidingSystem == "pp")   plugin->AddRunNumber(146817);
  }
  // Method 2: Use your input collection 
  //plugin->AddDataFile("/alice/cern.ch/user/m/mnicassi/139105.xml");

  //__________________________________________________________________________
  // Define alien work directory where all files will be copied. Relative to alien $HOME.
  plugin->SetGridWorkingDir(gridoutputdir.Data());
  plugin->SetGridOutputDir("output");

  //__________________________________________________________________________
  plugin->SetAnalysisSource("AliAnalysisTaskQAMultistrange.cxx");

  //__________________________________________________________________________
  //Enable same others packages
  //plugin->EnablePackage("PWG3dielectron.par");

  //__________________________________________________________________________
  // Optionally set a name for the generated analysis macro (default MyAnalysis.C)
  plugin->SetAnalysisMacro("testmacro.C");
  //Add all extra files (task.cxx/.h)
  plugin->SetAdditionalLibs("AliAnalysisTaskQAMultistrange.h AliAnalysisTaskQAMultistrange.cxx ");
  // Optionally modify the executable name (default analysis.sh)
  plugin->SetExecutable("Cascade.sh");
  
  //__________________________________________________________________________
  // Declare the output file names separated by blancs.
  // (can be like: file.root or file.root@ALICE::Niham::File)
  //plugin->SetDefaultOutputs(kFALSE);
  //plugin->SetOutputFiles("Cascades.root");

  // Optionally define the files to be archived.
  //plugin->SetOutputArchive("root_archive.zip:*.root log_archive.zip:stdout,stderr");

  //__________________________________________________________________________
  // Optionally set maximum number of input files/subjob (default 100, put 0 to ignore)
  plugin->SetSplitMaxInputFileNumber(100);
  // Optionally set number of failed jobs that will trigger killing waiting sub-jobs.
  plugin->SetMasterResubmitThreshold(90);
  // Optionally set time to live (default 30000 sec)
  plugin->SetTTL(30000);
  // Optionally set input format (default xml-single)
  plugin->SetInputFormat("xml-single");
  // Optionally modify the name of the generated JDL (default analysis.jdl)
  plugin->SetJDLName("TaskCheckCascade.jdl");

  // Optionally modify job price (default 1)
  plugin->SetPrice(1);

  // Merge via JDL
  // comment out the next line when using the "terminate" option, unless
  // you want separate merged files for each run
/*  plugin->SetMergeViaJDL(kTRUE);  // run first in full mode, then in terminate
  plugin->SetOneStageMerging(kFALSE);
  plugin->SetMaxMergeFiles(50);
  plugin->SetMaxMergeStages(2);// to define the number of stages
*/
  // Optionally set number of runs per master
//  plugin->SetNrunsPerMaster(1);
  //
  plugin->SetOutputToRunNo();
  // Optionally modify split mode (default 'se')
  plugin->SetSplitMode("se");
  plugin->SetUser("dcolella");
  return plugin;
}

