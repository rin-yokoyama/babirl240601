void clear(void){
  v513_cldata(COIN_ADDR);
  v513_clstrobe(COIN_ADDR);

  rpv130_output(RPV130ADDR, OPBUFFCL);
  rpv130_output(RPV130ADDR, OPBUSYCL);
}
