<?php

require_once('module.class.php');

class Generic_Scaler32Bit {
  public $module = array();

  const MASK  = 0xffffffff;
  public $nch = 0;

  function decode($buff){
    $list = unpack("I*", $buff);
    $cnt = count($list);
    $mod = new CModuleData(0);
    for($i=1;$i<=$cnt;$i++){
      $val = $list[$i] & self::MASK;
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
	  printf("%10d ", $ch->getVal());
	  $lines++;
	}
	if($lines >= 4){
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
