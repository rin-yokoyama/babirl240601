void evt(void){
  /* Event */

  // Disable interrupt
  v7XX_set_interrupt(ADCADDR, 0, 0);

  // Read interrupt vector, some modules require this to release the interrupt
  vme_read_intvector();

  // Initialize event
#ifdef TIMESTAMPEF
  init_eventts();
#else
  init_event();
#endif

  // Initialize segment
  // Devie=0, Focal=0, Detector=0, Module=V785
  init_segment(MKSEGID(0,0,0,V785));
  v7XX_segdata(ADCADDR); 
  // to store multiple module's data,
  // you have to set the geometry address before the data readout
  //v7XX_segdata(ADCADDR2);
  // End segment
  end_segment();

  // Next segment
  // Devie=0, Focal=0, Detector=1, Module=MADC32
  //init_segment(MKSEGID(0,0,1,MADC32));
  //madc32_segdata(ADCADDR3);
  //end_segment();

  // Clear module
  v7XX_clear(ADCADDR);
}
