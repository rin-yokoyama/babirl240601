#include <bbjson.h>

int main(int argc, char *argv[]){
  //int i;

  bbjson_begin();

  bbjson_begin_obj("scrdata");
  /*
  bbjson_charobj("scrname","namae");
  bbjson_intobj("scrid",32);
  bbjson_begin_array("data");
  for(i=0;i<3;i++){
    bbjson_begin_obj(0);
    bbjson_intobj("ch", i);
    bbjson_charobj("name", "hogehoge");
    bbjson_end_obj();
  }
  bbjson_end_array();
  */
  bbjson_end_obj();
  bbjson_begin_obj("hoge");
  bbjson_end_obj();

  /*
  bbjson_charobj("hoge","desuyo");
  bbjson_intobj("eventnumber", 3333);
  bbjson_begin_obj("daqinfo");
  bbjson_charobj("runname","runname");
  bbjson_intobj("runnumber", 22);
  bbjson_end_obj();
  bbjson_begin_obj("runinfo");
  bbjson_charobj("runname","runname");
  bbjson_intobj("runnumber", 25);
  bbjson_end_obj();
  bbjson_begin_array("eflist");
  for(i=0;i<3;i++){
    x = i * 10;
    bbjson_begin_obj(0);
    bbjson_intobj("efn", x);
    bbjson_charobj("host", "hogehost");
    bbjson_charobj("of", "on");
    bbjson_end_obj();
  }
  bbjson_end_array();

  bbjson_begin_array("eflist2");
  for(i=0;i<3;i++){
    x = i * 10;
    bbjson_begin_obj(0);
    bbjson_intobj("efn", x);
    bbjson_charobj("host", "hogehost");
    bbjson_charobj("of", "on");
    bbjson_end_obj();
  }
  bbjson_end_array();

  bbjson_begin_obj("runinfo");
  bbjson_charobj("runname","runname");
  bbjson_intobj("runnumber", 25);
  bbjson_end_obj();
  */

  bbjson_end();
  
  return 0;
}
