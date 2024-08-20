void dfullf(void){
  /* VETO Start for Double Buffer full */
  rpv130_level(RPV130ADDR,1);
}

void dfullcl(void){
  /* VETO Clear for Double Buffer full */
  //short val;
  //val = OPBUFFCL;
  //write_mod(0,OPRN,0,17,&val);
  rpv130_level(RPV130ADDR,0);
}
