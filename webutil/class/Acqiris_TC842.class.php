<?php

require_once('module.class.php');

class Acqiris_TC842 {
  public $module = array();

  public $nch = 0;

  function decode($buff){
    $list = unpack("d*", $buff);
    $cnt = count($list);
    $mod = new CModuleData(0);
    for($i=1;$i<=$cnt;$i++){
      $val = $list[$i];
      $mod->setChannelVal($i-1, $val);
      $this->nch ++;
    }
    array_push($this->module, $mod);    
  }

  function show_decode(){
    echo "<pre>";
    foreach ($this->module as $mod){
      $lines = 0;
      for($i=0;$i<$this->nch;$i++){
	if($ch = $mod->getCh($i)){
	  $v = $ch->getVal();
	  $s = 's';
	  if($v < 1e-6){
	    $v = $v * 1e+9;
	    $s = 'ns';
	  }else if($v < 1e-3){
	    $v = $v * 1e+6;
	    $s = 'us';
	  }else if($v < 1){
	    $v = $v * 1e+3;
	    $s = 'ms';
	  }else if($v > 100){
	    $v = 100;
	    $s = 'ovf';
	  }
	  printf("%6.2f %-3s ", $v, $s);
	  $lines++;
	}
	if($lines >= 2){
	  $lines = 0;
	  echo "\n";
	}
      }
      echo "\n";
    }
    echo "</pre>";
  }
}


?>
