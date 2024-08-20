void evt(void){
  /* Event */

  init_event();

  init_segment(9);
  v513_segdata(COIN_ADDR);
  end_segment();

  init_segment(1); 
  v792_segdata(BLSCQ_ADDR); 
  v785_segdata(BLSCT_ADDR); 
  end_segment(); 

  init_segment(2); 
  v785_segdata(SSDT_ADDR); 
  end_segment(); 

  init_segment(3); 
  v792_dmasegdata(PPACQ_ADDR, PPACQL); 
  v775_dmasegdata(PPACT_ADDR, PPACTL); 
  //v792_segdata(PPACQ_ADDR); 
  //v775_segdata(PPACT_ADDR); 
  end_segment(); 
}
