void startup(void){
  /* Startup Function */

  // Define the interrupt level for the controller
  vme_define_intlevel(INTLEVEL);

  // Set the interrupt genartion of ADC
  v7XX_set_interrupt(ADCADDR, INTLEVEL, 1);

  // Clear data
  v7XX_clear(ADCADDR);

}
