/**
  Author: Matthew Strait
*/

using namespace std;

#include <signal.h>
#include <errno.h>
#include <vector>
#include "idivc_cont.h"
#include "idivc_root.h"
#include "idivc_progress.cpp"

static void printhelp()
{
  printf(
  "IDIVC: The Inner Detector Inner Veto Event Time Corrector\n"
  "\n"
  "Basic syntax: idivc -o [output file] [one or more base.root files]\n"
  "\n"
  "-c: Overwrite existing output file\n"
  "-n [number] Process at most this many events\n"
  "-h: This help text\n");
}

/** Parses the command line and returns the position of the first file
name (i.e. the first argument not parsed). */
static int handle_cmdline(int argc, char ** argv, bool & clobber,
                          unsigned int & nevents, char * & outfile,
                          char * & timingfile)
{
  const char * const opts = "o:chn:t:";
  bool done = false;
 
  while(!done){
    char whatwegot;
    switch(whatwegot = getopt(argc, argv, opts)){
      case -1:
        done = true;
        break;
      case 'n':
        errno = 0;
        char * endptr;
        nevents = strtol(optarg, &endptr, 10);
        if((errno == ERANGE && (nevents == UINT_MAX)) || 
           (errno != 0 && nevents == 0) || 
           endptr == optarg || *endptr != '\0'){
          fprintf(stderr,
            "%s (given with -n) isn't a number I can handle\n", optarg);
          exit(1);
        }
        break;
      case 'o':
        outfile = optarg;
        break;
      case 'c':
        clobber = true;
        break;
      case 'h':
        printhelp();
        exit(0);
      case 't':
        timingfile = optarg;
        break;
      default:
        printhelp();
        exit(1);
    }
  }  

  if(!timingfile){
    fprintf(stderr, "You must give an timing file name with -o\n");
    printhelp();
    exit(1);
  }

  if(!outfile){
    fprintf(stderr, "You must give an output file name with -o\n");
    printhelp();
    exit(1);
  }

  if(argc <= optind){
    fprintf(stderr, "Please give at least one base.root file.\n\n");
    printhelp();
    exit(1);
  }
  return optind;
}

static void on_segv_or_bus(const int signal)
{
  fprintf(stderr, "Got %s. Exiting.\n", signal==SIGSEGV? "SEGV": "BUS");
  // Use _exit() instead of exit() to avoid calling atexit() functions
  // and/or other signal handlers. Something, presumably in the bowels
  // of ROOT, must be doing one of these since a call to exit() can take
  // several minutes (!) to complete, but _exit() finishes more quickly.
  _exit(1);
}

/** To be called when the user presses Ctrl-C or something similar
happens. */
static void endearly(__attribute__((unused)) int signal)
{
  fprintf(stderr, "Got Ctrl-C or similar.  Exiting.\n");
  _exit(1); // See comment above
}

static idivc_out_event doit(const idivc_inevent & ev)
{
  idivc_out_event out;
  memset(&out, 0, sizeof(out));

  return out;
}

static void doit_loop(const unsigned int nevent)
{
  printf("Working...\n");
  initprogressindicator(nevent, 4);

  // NOTE: Do not attempt to start anywhere but on event zero.
  // For better performance, we don't allow random seeks.
  for(unsigned int i = 0; i < nevent; i++)
    write_event(doit(get_event(i))), // never do this
    progressindicator(i, "IDIVC");
  printf("All done working.\n");
}

static double * getfidoconsts(const char * const timingfilename)
{
  TFile * hey = new TFile(timingfilename.c_str(), "read");

  if(!hey || hey->IsZombie()){
    printf("Could not open timing file %s\n", timingfilename);
    exit(1);
  }
  
  const TGraphErrors * const calgraph = 
    dynamic_cast<TGraphErrors *>(hey.Get("finalt0table_caliter01"));

  if(!calgraph){
    printf("Couldn't get finalt0table_caliter01 from timing file\n");
    exit(1);
  }

  const int NPMT = 468;

  double * const consts = (double*)malloc(NPMT*sizeof(double));
  memset(consts, 0, NPMT*sizeof(double));
  
  for(int i = 0; i < calgraph->GetN(); i++){
    double pmt, time;
    calgraph->GetPoint(i, pmt, time);
    const double timee = calgraph->GetErrorY(i);

    // means tube wasn't fit, probably because it was powered off
    if(time == 0 || timee == 0 || timee == 1) continue;
    
    // Very few hits in this run?  Shouldn't really happen.
    if(timee > 1){ printf("error of %f...\n", timee); continue; }

    if(pmt >= NPMT){ printf("bad PMT number %d\n", pmt); exit(1); }

    consts[pmt] = time;
  }

  delete calgraph;
  delete hey;
  
  return consts;
}

int main(int argc, char ** argv)
{
  signal(SIGSEGV, on_segv_or_bus);
  signal(SIGBUS,  on_segv_or_bus);
  signal(SIGINT, endearly);
  signal(SIGHUP, endearly);

  char * outfile = NULL, * timingfile = NULL;
  bool clobber = false; // Whether to overwrite existing output
                         
  unsigned int maxevent = 0;
  const int file1 =
    handle_cmdline(argc, argv, clobber, maxevent, outfile, timingfile);

  const double * const fido_consts = getfidoconsts(timingfile);

  const unsigned int nevent = root_init(maxevent, clobber, outfile, 
                                        argv + file1, argc - file1);
  doit_loop(nevent);

  root_finish();
  
  return 0;
}
