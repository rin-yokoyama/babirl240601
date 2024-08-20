void stop(void){
  // Disable interrupt of ADC
  v7XX_set_interrupt(ADCADDR, 0, 0);
  v7XX_clear(ADCADDR);
}
