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
#include "TFile.h"
#include "TGraphErrors.h"


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

static int ch2pmt(const int ch)
{
  switch (ch){
    case 0: return 94;
    case 1: return 92;
    case 2: return 90;
    case 3: return 124;
    case 4: return 122;
    case 5: return 154;
    case 6: return 152;
    case 7: return 150;
    case 8: return 180;
    case 9: return 64;
    case 10: return 62;
    case 11: return 36;
    case 12: return 38;
    case 13: return 18;
    case 14: return 20;
    case 15: return 0;
    case 16: return 98;
    case 17: return 96;
    case 18: return 130;
    case 19: return 128;
    case 20: return 126;
    case 21: return 158;
    case 22: return 156;
    case 23: return 184;
    case 24: return 70;
    case 25: return 68;
    case 26: return 66;
    case 27: return 40;
    case 28: return 42;
    case 29: return 22;
    case 30: return 8;
    case 31: return 9;
    case 32: return 104;
    case 33: return 102;
    case 34: return 100;
    case 35: return 134;
    case 36: return 132;
    case 37: return 164;
    case 38: return 162;
    case 39: return 160;
    case 40: return 186;
    case 41: return 74;
    case 42: return 72;
    case 43: return 44;
    case 44: return 46;
    case 45: return 24;
    case 46: return 26;
    case 47: return 2;
    case 48: return 108;
    case 49: return 106;
    case 50: return 140;
    case 51: return 138;
    case 52: return 136;
    case 53: return 168;
    case 54: return 166;
    case 55: return 188;
    case 56: return 80;
    case 57: return 78;
    case 58: return 76;
    case 59: return 48;
    case 60: return 50;
    case 61: return 28;
    case 62: return 12;
    case 63: return 13;
    case 64: return 114;
    case 65: return 112;
    case 66: return 110;
    case 67: return 144;
    case 68: return 142;
    case 69: return 174;
    case 70: return 172;
    case 71: return 170;
    case 72: return 190;
    case 73: return 84;
    case 74: return 82;
    case 75: return 52;
    case 76: return 54;
    case 77: return 30;
    case 78: return 32;
    case 79: return 4;
    case 80: return 118;
    case 81: return 116;
    case 82: return 148;
    case 83: return 146;
    case 84: return 120;
    case 85: return 178;
    case 86: return 176;
    case 87: return 194;
    case 88: return 88;
    case 89: return 86;
    case 90: return 60;
    case 91: return 56;
    case 92: return 58;
    case 93: return 34;
    case 94: return 16;
    case 95: return 17;
    case 96: return 93;
    case 97: return 91;
    case 98: return 125;
    case 99: return 123;
    case 100: return 121;
    case 101: return 153;
    case 102: return 151;
    case 103: return 181;
    case 104: return 65;
    case 105: return 63;
    case 106: return 61;
    case 107: return 37;
    case 108: return 39;
    case 109: return 19;
    case 110: return 6;
    case 111: return 7;
    case 112: return 99;
    case 113: return 97;
    case 114: return 95;
    case 115: return 129;
    case 116: return 127;
    case 117: return 159;
    case 118: return 157;
    case 119: return 155;
    case 120: return 183;
    case 121: return 69;
    case 122: return 67;
    case 123: return 41;
    case 124: return 43;
    case 125: return 21;
    case 126: return 23;
    case 127: return 1;
    case 128: return 103;
    case 129: return 101;
    case 130: return 135;
    case 131: return 133;
    case 132: return 131;
    case 133: return 163;
    case 134: return 161;
    case 135: return 185;
    case 136: return 75;
    case 137: return 73;
    case 138: return 71;
    case 139: return 45;
    case 140: return 47;
    case 141: return 25;
    case 142: return 10;
    case 143: return 11;
    case 144: return 109;
    case 145: return 107;
    case 146: return 105;
    case 147: return 139;
    case 148: return 137;
    case 149: return 169;
    case 150: return 167;
    case 151: return 165;
    case 152: return 189;
    case 153: return 79;
    case 154: return 77;
    case 155: return 49;
    case 156: return 51;
    case 157: return 27;
    case 158: return 29;
    case 159: return 3;
    case 160: return 113;
    case 161: return 111;
    case 162: return 145;
    case 163: return 143;
    case 164: return 141;
    case 165: return 173;
    case 166: return 171;
    case 167: return 191;
    case 168: return 85;
    case 169: return 83;
    case 170: return 81;
    case 171: return 53;
    case 172: return 55;
    case 173: return 31;
    case 174: return 14;
    case 175: return 15;
    case 176: return 119;
    case 177: return 117;
    case 178: return 115;
    case 179: return 149;
    case 180: return 147;
    case 181: return 179;
    case 182: return 177;
    case 183: return 175;
    case 184: return 193;
    case 185: return 89;
    case 186: return 87;
    case 187: return 57;
    case 188: return 59;
    case 189: return 33;
    case 190: return 35;
    case 191: return 5;
    case 192: return 197;
    case 193: return 213;
    case 194: return 211;
    case 195: return 245;
    case 196: return 243;
    case 197: return 241;
    case 198: return 273;
    case 199: return 271;
    case 200: return 305;
    case 201: return 303;
    case 202: return 301;
    case 203: return 353;
    case 204: return 351;
    case 205: return 371;
    case 206: return 369;
    case 207: return 389;
    case 208: return 199;
    case 209: return 219;
    case 210: return 217;
    case 211: return 215;
    case 212: return 249;
    case 213: return 247;
    case 214: return 279;
    case 215: return 277;
    case 216: return 275;
    case 217: return 309;
    case 218: return 307;
    case 219: return 349;
    case 220: return 347;
    case 221: return 367;
    case 222: return 381;
    case 223: return 380;
    case 224: return 201;
    case 225: return 223;
    case 226: return 221;
    case 227: return 255;
    case 228: return 253;
    case 229: return 251;
    case 230: return 283;
    case 231: return 281;
    case 232: return 315;
    case 233: return 313;
    case 234: return 311;
    case 235: return 345;
    case 236: return 343;
    case 237: return 365;
    case 238: return 363;
    case 239: return 387;
    case 240: return 203;
    case 241: return 229;
    case 242: return 227;
    case 243: return 225;
    case 244: return 259;
    case 245: return 257;
    case 246: return 289;
    case 247: return 287;
    case 248: return 285;
    case 250: return 317;
    case 251: return 341;
    case 252: return 339;
    case 253: return 361;
    case 254: return 377;
    case 255: return 376;
    case 256: return 207;
    case 257: return 233;
    case 258: return 231;
    case 259: return 265;
    case 260: return 263;
    case 261: return 261;
    case 262: return 293;
    case 263: return 291;
    case 264: return 325;
    case 265: return 323;
    case 266: return 321;
    case 267: return 337;
    case 268: return 335;
    case 269: return 359;
    case 270: return 357;
    case 271: return 385;
    case 272: return 209;
    case 273: return 239;
    case 274: return 237;
    case 275: return 235;
    case 276: return 269;
    case 277: return 267;
    case 278: return 299;
    case 279: return 297;
    case 280: return 295;
    case 281: return 329;
    case 282: return 327;
    case 283: return 333;
    case 284: return 331;
    case 285: return 355;
    case 286: return 373;
    case 287: return 372;
    case 288: return 192;
    case 289: return 182;
    case 290: return 200;
    case 291: return 187;
    case 292: return 205;
    case 293: return 195;
    case 294: return 65535;
    case 295: return 65535;
    case 296: return 196;
    case 297: return 214;
    case 298: return 212;
    case 299: return 210;
    case 300: return 244;
    case 301: return 242;
    case 302: return 274;
    case 303: return 272;
    case 304: return 270;
    case 305: return 304;
    case 306: return 302;
    case 307: return 352;
    case 308: return 350;
    case 309: return 370;
    case 310: return 383;
    case 311: return 382;
    case 312: return 198;
    case 313: return 218;
    case 314: return 216;
    case 315: return 250;
    case 316: return 248;
    case 317: return 246;
    case 318: return 278;
    case 319: return 276;
    case 320: return 310;
    case 321: return 308;
    case 322: return 306;
    case 323: return 348;
    case 324: return 346;
    case 325: return 368;
    case 326: return 366;
    case 327: return 388;
    case 328: return 202;
    case 329: return 224;
    case 330: return 222;
    case 331: return 220;
    case 332: return 254;
    case 333: return 252;
    case 334: return 284;
    case 335: return 282;
    case 336: return 280;
    case 337: return 314;
    case 338: return 312;
    case 339: return 344;
    case 340: return 342;
    case 341: return 364;
    case 342: return 379;
    case 343: return 378;
    case 344: return 204;
    case 345: return 228;
    case 346: return 226;
    case 347: return 260;
    case 348: return 258;
    case 349: return 256;
    case 350: return 288;
    case 351: return 286;
    case 352: return 320;
    case 353: return 318;
    case 354: return 316;
    case 355: return 340;
    case 356: return 338;
    case 357: return 362;
    case 358: return 360;
    case 359: return 386;
    case 360: return 206;
    case 361: return 234;
    case 362: return 232;
    case 363: return 230;
    case 364: return 264;
    case 365: return 262;
    case 366: return 294;
    case 367: return 292;
    case 368: return 290;
    case 369: return 324;
    case 370: return 322;
    case 371: return 336;
    case 372: return 334;
    case 373: return 358;
    case 374: return 375;
    case 375: return 374;
    case 376: return 208;
    case 377: return 238;
    case 378: return 236;
    case 379: return 268;
    case 380: return 266;
    case 381: return 240;
    case 382: return 298;
    case 383: return 296;
    case 384: return 328;
    case 385: return 326;
    case 386: return 300;
    case 387: return 332;
    case 388: return 330;
    case 389: return 356;
    case 390: return 354;
    case 391: return 384;
    case 1000: return 393;
    case 1001: return 433;
    case 1002: return 435;
    case 1003: return 417;
    case 1004: return 418;
    case 1005: return 397;
    case 1006: return 421;
    case 1007: return 422;
    case 1008: return 395;
    case 1009: return 419;
    case 1010: return 420;
    case 1011: return 437;
    case 1012: return 439;
    case 1013: return 441;
    case 1014: return 443;
    case 1015: return 65535;
    case 1016: return 399;
    case 1017: return 423;
    case 1018: return 424;
    case 1019: return 445;
    case 1020: return 447;
    case 1021: return 453;
    case 1022: return 450;
    case 1023: return 465;
    case 1024: return 401;
    case 1025: return 414;
    case 1026: return 425;
    case 1027: return 427;
    case 1028: return 449;
    case 1029: return 462;
    case 1030: return 459;
    case 1031: return 456;
    case 1032: return 428;
    case 1033: return 430;
    case 1034: return 452;
    case 1035: return 454;
    case 1036: return 432;
    case 1037: return 434;
    case 1038: return 455;
    case 1039: return 457;
    case 1040: return 436;
    case 1041: return 438;
    case 1042: return 458;
    case 1043: return 460;
    case 1044: return 440;
    case 1045: return 442;
    case 1046: return 461;
    case 1047: return 463;
    case 1048: return 444;
    case 1049: return 446;
    case 1050: return 464;
    case 1051: return 466;
    case 1052: return 426;
    case 1053: return 448;
    case 1054: return 451;
    case 1055: return 467;
    case 1056: return 404;
    case 1057: return 402;
    case 1058: return 412;
    case 1059: return 410;
    case 1060: return 409;
    case 1061: return 406;
    case 1062: return 391;
    case 1063: return 415;
    case 1064: return 392;
    case 1065: return 403;
    case 1066: return 390;
    case 1067: return 398;
    case 1068: return 407;
    case 1069: return 396;
    case 1070: return 416;
    case 1071: return 429;
    case 1072: return 394;
    case 1073: return 405;
    case 1074: return 408;
    case 1075: return 413;
    case 1076: return 400;
    case 1077: return 411;
    case 1078: return 431;

    default: return -1;
  }
}

static idivc_output_event doit(const idivc_input_event & ev,
                               const double * const fido_consts)
{
  idivc_output_event out;
  memset(&out, 0, sizeof(out));

  out.timeid = out.timeiv = 9999;

  for(int ch = 0; ch < 520; ch++){
    const int pmt = ch2pmt(ch);
    if(pmt < 0) continue;

    const double time = ev.tstart[ch] + fido_consts[pmt];

    if(pmt < 390){
      if(time < out.timeid){
        out.timeid = time; 
        out.firstidpmt = pmt;
      }
    } 
    else if(time < out.timeiv){
      out.timeiv = time;
      out.firstivpmt = pmt;
    }
  }    

  return out;
}

static void doit_loop(const unsigned int nevent,
                      const double * const fido_consts)
{
  printf("Working...\n");
  initprogressindicator(nevent, 4);

  // NOTE: Do not attempt to start anywhere but on event zero.
  // For better performance, we don't allow random seeks.
  for(unsigned int i = 0; i < nevent; i++)
    write_event(doit(get_event(i), fido_consts)), // never do this
    progressindicator(i, "IDIVC");
  printf("All done working.\n");
}

static double * getfidoconsts(const char * const timingfilename)
{
  TFile * hey = new TFile(timingfilename, "read");

  if(!hey || hey->IsZombie()){
    printf("Could not open timing file %s\n", timingfilename);
    exit(1);
  }
  
  const TGraphErrors * const calgraph = 
    dynamic_cast<TGraphErrors *>(hey->Get("finalt0table_caliter01"));

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

    if(pmt >= NPMT){ printf("bad PMT number %d\n", int(pmt)); exit(1); }

    consts[int(pmt)] = time;
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
  doit_loop(nevent, fido_consts);

  root_finish();
  
  return 0;
}
