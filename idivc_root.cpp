/**
  \author Matthew Strait
  \brief Handles most ROOT interaction for ROVER.
*/

using namespace std;

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE // for safe basename()
#endif
#include <string.h>
#include <vector>
#include "TSystem.h"
#include "TChain.h"
#include "TFile.h"
#include "TError.h"
#include "TClonesArray.h"
#include "idivc_cont.h"


namespace {
  idivc_input_event inevent;
  idivc_output_event outevent;

  vector<TTree *> hitchain;
  vector<uint64_t> hitchain_entries;

  // Needed for writing the output file
  TFile * outfile;
  TTree * recotree;
}; 

static void get_hits(const uint64_t current_event)
{
  // Go through some contortions for speed. Favor TBranch::GetEntry over
  // TTree::GetEntry, which loops through unused branches on every call.
  // Avoid using TChain to find the TTrees' branches on every call.
  static TBranch * tbranch = 0;

  static uint64_t offset = 0, nextbreak = 0;

  // This allows reading randomly around in the current TTree or
  // resetting to the first TTree, but random reads from one TTree to
  // another will fail catastrophically.
  if(current_event == 0 || current_event == nextbreak){
    static TTree * curtree = 0;
    static int curtreeindex = -1;
    if(current_event == 0){
      offset = nextbreak = 0;
      curtreeindex = -1;
    }

    curtree = hitchain[++curtreeindex];
    nextbreak = hitchain_entries[curtreeindex+1];
    offset = hitchain_entries[curtreeindex];

    curtree->SetMakeClass(1);
    tbranch   = curtree->GetBranch("PulseSlideWinInfoBranch.fTstart_raw");
    int dummy;
    curtree->SetBranchAddress("OVHitInfoBranch", &dummy);
    curtree->SetBranchAddress("OVHitInfoBranch.fTstart_raw", inevent.tstart);
  }

  const uint64_t localentry = current_event - offset;

  tbranch->GetEntry(localentry);
}

/** Make inevent the eventn'th event in the chain. */
idivc_input_event get_event(const uint64_t current_event)
{
  memset(&inevent, 0, sizeof(inevent));
  get_hits(current_event);

  return inevent;
}

void write_event(const idivc_output_event & out)
{
  outevent = out;
  recotree->Fill();
}

static uint64_t root_init_input(const char * const * const filenames,
                                const int nfiles)
{
  TChain mctestchain("PulseSlideWinInfoTree");

  uint64_t totentries_hit = 0, totentries_reco = 0;

  for(int i = 0; i < nfiles; i++){
    const char * const fname = filenames[i];
    if(strlen(fname) < 9){
      fprintf(stderr, "%s doesn't have the form *base*.root\n", fname);
      _exit(1);
    }
    if(!strstr(fname, "base")){
      fprintf(stderr, "File name %s does not contain \"base\"\n", fname);
      _exit(1);
    }
    if(strstr(fname, ".root") != fname + strlen(fname) - 5){
      fprintf(stderr, "File name %s does not end in \".root\"\n", fname);
      _exit(1);
    }

    TFile * inputfile = new TFile(fname, "read");
    if(!inputfile || inputfile->IsZombie()){
      fprintf(stderr, "%s became a zombie when ROOT tried to read it.\n",fname);
      _exit(1);
    }

    TTree * temp=dynamic_cast<TTree*>(inputfile->Get("PulseSlideWinInfoTree"));
    if(!temp){
      fprintf(stderr, "%s does not have a PulseSlideWinInfoTree tree\n", fname);
      _exit(1);
    }

    hitchain_entries.push_back(totentries_hit);
    hitchain.push_back(temp);
    totentries_hit += temp->GetEntries();

    printf("Loaded %s\n", fname);
  }

  if(totentries_hit != totentries_reco){
    fprintf(stderr, "ERROR: hit tree has %ld entries, but reco tree has %ld\n",
            totentries_hit, totentries_reco);
    exit(1);
  }

  return totentries_hit;
}

static void root_init_output(const bool clobber,
                             const char * const outfilename)
{
  outfile = new TFile(outfilename, clobber?"RECREATE":"CREATE","",9);

  if(!outfile || outfile->IsZombie()){
    fprintf(stderr, "Could not open output file %s. Does it exist?  "
            "Use -c to overwrite existing output.\n", outfilename);
    exit(1);
  }

  // Name and title same as in old EnDep code
  recotree = new TTree("idivc", "ID and IV time correction tree tree");

  recotree->Branch("timeid", &outevent.timeid);
  recotree->Branch("timeiv", &outevent.timeiv);
  recotree->Branch("firstidpmt", &outevent.firstidpmt);
  recotree->Branch("firstivpmt", &outevent.firstivpmt);
}

void root_finish()
{
  gErrorIgnoreLevel = kError;
  outfile->cd();
  recotree->Write();
  outfile->Close();
}

/* Sets up the ROOT input and output. */
uint64_t root_init(const uint64_t maxevent, const bool clobber,
                   const char * const outfilenm,
                   const char * const * const infiles, const int nfiles)
{
  // ROOT warnings are usually not helpful to the user, so we'll try
  // to catch warning conditions ourselves.  However, I know of at
  // least one error that can't be caught before a seg fault (!), so
  // let ROOT spew about that.
  gErrorIgnoreLevel = kError; 

  root_init_output(clobber, outfilenm);

  const uint64_t nevents = root_init_input(infiles, nfiles);
  uint64_t neventstouse = nevents;
  if(maxevent && nevents > maxevent) neventstouse = maxevent;

  return neventstouse;
}
