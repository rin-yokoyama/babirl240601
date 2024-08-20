//#include <TApplication.h>
//#include <TH1F.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
//#include <TGFrame.h>
//#include <TGButton.h>
#include <TGClient.h>



//#define MAXFINFO 10

using namespace std;

#include "TSFrame.h"

static int overlap = 1;


TSFrame::TSFrame( const TGWindow *p, UInt_t w, UInt_t h )
  : TGMainFrame( p, w, h )
{
  min = -1000;
  max = 1000;
  htn = 0;
  canvas= new TCanvas("canvas", "Canvas", 200, 10, 600, 600);

}

TSFrame::~TSFrame()
{
  Cleanup();
}

void TSFrame::setAction(void){
  run = new TGTextButton( this, "&Run");
  run->Connect("Clicked()", "TSFrame", this, "runAnalysis()");
  AddFrame( run, new TGLayoutHints( kLHintsExpandX, 1, 1, 1, 1 ) );

  draw = new TGTextButton( this, "&Draw");
  draw->Connect("Clicked()", "TSFrame", this, "drawTH1()");
  AddFrame( draw, new TGLayoutHints( kLHintsExpandX, 1, 1, 1, 1 ) );

  exit = new TGTextButton( this, "&Exit");
  exit->Connect("Clicked()", "TSFrame", this, "quitFrame()");
  AddFrame( exit, new TGLayoutHints( kLHintsExpandX, 1, 1, 1, 1 ) );
  
  MapSubwindows();
  Resize( GetDefaultSize() );
  MapWindow();
}

void TSFrame::setMinMax(int a, int b, int c){
  min = a;
  max = b;
  evtmax = c;
}

void TSFrame::setFInfo(int n, string s){
  fInfo[n].setName(s);
  htn++;

  cout << "setFInfo[" << htn << "]  = " << s << endl;
}

void TSFrame::createTH1(void){
  int i;
  string args;

  for(i=0;i<htn;i++){
    args = fInfo[i].getName();
    ht[i] = new TH1F(args.c_str(), args.c_str(), max - min, min, max);
    cout << "ht[" << i << "] created" << endl;
    if((fd[i] = fopen(fInfo[i].getPath().c_str(), "r")) == NULL){
      cout << "Can't open" << fInfo[i].getPath() << endl;
    }
  }
  canvas->Divide(1, htn);

}

void TSFrame::runAnalysis(void){
  long long int tb, tt;
  long long int val[MAXFINFO][2];
  int i, f, n[MAXFINFO], l;

  cout << "runTH1 (" << evtmax << ")" << endl;

  memset(n, 0, sizeof(n));

  l = 0;
  
  while(!feof(fd[0]) && l < evtmax){
    fread(val[0], 8, 2, fd[0]);
    tb = val[0][0] & 0x0000ffffffffffff;
    //cout << "val = " << tb << endl;
    l++;

    ht[0]->Fill(0);
    for(i=1;i<htn;i++){
      f = 1;
      
      while(f){
	if(!n[i]){
	  if(fread(val[i], 8, 2, fd[i]) != 2){
	    f = 0;
	    break;
	  }
	}
	tt = val[i][0] & 0x0000ffffffffffff;

	
	if(tb + min > tt){
	  f = 1;
	  n[i] = 0;
	}else{
	  if(tb + max < tt){
	    f = 0;
	    n[i] = 1;  // Keep tt[htn]
	  }else{
	    f = 0;
	    if(overlap == 1){
	      n[i] = 1;
	    }else{
	      n[i] = 0;
	    }
	    ht[i]->Fill(tt - tb);
	    //cout << "tb=" << tb << "  tt=" << tt << "  tb-tt=" << tt-tb << endl;
	  }
	}
      }
    }

  }

  drawTH1();
}

void TSFrame::drawTH1(void){
  int i;

  cout << "drawTH1" << endl;

  for(i=0;i<htn;i++){
    canvas->cd(i+1);
    ht[i]->Draw();
  }

  canvas->Modified();
  canvas->Update();
}

void TSFrame::quitFrame(void){
  cout << "Exit TSHisto" << endl;
  gApplication->Terminate(0);
}

int main(int argc, char *argv[]){
  int i, min, max, htn, evtmax, off, wid;
  stringstream ss;
  string args;
  int aargc;
  char aargv[MAXFINFO][256];

  aargc = argc;

  if(argc < 4){
    cout << "TSHisto OFF WID EVTN FILE [FILE] ..." << endl;
    exit(0);
  }

  htn = 0;
  for(i=0;i<argc;i++){
    strcpy(aargv[i], argv[i]);
    printf("%d %d %s\n", i, argc, aargv[i]);
  }

  TApplication theApp("app", &argc, argv);
  TSFrame win( gClient->GetRoot(), 400, 300 );

  for(i=1;i<aargc;i++){
    ss.clear();
    ss.str("");
    args = aargv[i];
    switch(i){
    case 1:
      ss << args;
      ss >> off;
      break;
    case 2:
      ss << args;
      ss >> wid;
      break;
    case 3:
      ss << args;
      ss >> evtmax;
      break;
    default:
      win.setFInfo(htn, args);
      htn++;
      break;
    }
  }

  min = off;
  max = off + wid;
  win.setMinMax(min, max, evtmax);
  win.setAction();
  win.createTH1();
  win.runAnalysis();
  win.drawTH1();

  theApp.Run();
  
  return 0;
}


/*
    int bin = max - min;

    histogram=new TH1F("histogram", "Histogram", bin, min, max);
    histogram->Fill(2);
    histogram->Fill(4);
    histogram->Fill(4);
    histogram->Draw();
*/

