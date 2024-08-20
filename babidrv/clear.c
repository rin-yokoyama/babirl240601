void clear(void){
  // Set the interrupt genartion of ADC
  v7XX_set_interrupt(ADCADDR, INTLEVEL, 1);

  // Muticlient case, you must send the end-of-busy to trigger circuits
  // this clearing function must be in here not in evt.c
  // rpv130_pulse(RPVADDR, 1);
}
