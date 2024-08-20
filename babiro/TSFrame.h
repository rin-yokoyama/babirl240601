#include <TApplication.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGWindow.h>
#include <TH1F.h>
#include <TCanvas.h>

//using namespace std;

#define MAXFINFO 10


class FInfo {
private:
  string path;
  string name;
public:
  FInfo(){;}
  ~FInfo(){;}
  FInfo(string p){
    path = p;
    name = p;
  }
  
  void setName(string p){
    int idx1, idx2;

    path = p;

    idx1 = path.find_last_of('/');
    idx2 = path.find_last_of('.');
    name = path.substr(idx1+1, idx2 - idx1 - 1);
    //cout << "idx" << idx1 << " name=" << name <<  endl;
  }

  string getName(void){
    return name;
  }
  string getPath(void){
    return path;
  }
};


class TSFrame : public TGMainFrame {
private:
  TGTextButton *exit;
  TGTextButton *run;
  TGTextButton *draw;
  int min;
  int max;
  int evtmax;
  TCanvas* canvas;

  FInfo fInfo[MAXFINFO];
  TH1F *ht[MAXFINFO];
  int htn;
  FILE *fd[MAXFINFO];

public:
  TSFrame( const TGWindow *p, UInt_t w, UInt_t h );
  void setAction(void);
  void setMinMax(int min, int max, int evtmax);
  void setFInfo(int n, string s);
  void createTH1(void);
  void runAnalysis(void);
  void drawTH1(void);
  void quitFrame(void);
  virtual ~TSFrame();

  ClassDef(TSFrame, 0);
};
