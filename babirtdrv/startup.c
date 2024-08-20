void startup(void){

  /* Startup Function */

  vme_define_intlevel(INTLEVEL);

  v513_cldata(COIN_ADDR);
  v513_clstrobe(COIN_ADDR);

  rpv130_level(RPV130ADDR, 0);

  rpv130_output(RPV130ADDR, OPBUFFCL);
  rpv130_output(RPV130ADDR, OPBUSYCL);

}
