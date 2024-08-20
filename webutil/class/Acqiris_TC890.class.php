<?php

require_once('module.class.php');

class Acqiris_TC890 {
  public $module = array();

  const MASK   = 0x8fffffff;
  const CHMASK = 0x70000000;
  const CHSFT  = 28;
  public $nch = 0;

  function decode($buff){
    $list = unpack("I*", $buff);
    $cnt = count($list);
    $mod = new CModuleData(0);
    for($i=1;$i<=$cnt;$i++){
      $val = $list[$i] & self::MASK;
      $ch = ($list[$i] & self::CHMASK) >> self::CHSFT;
      if($ch >= 1 && $ch <= 6){
	$mod->setChannelValMulti($ch, $val);
	$this->nch ++;
      }
    }
    array_push($this->module, $mod);    
  }

  function show_decode(){
    echo "<pre>";
    foreach ($this->module as $mod){
      $lines = 0;
      for($i=0;$i<$this->nch;$i++){
	if($cch = $mod->getChObj($i)){
	  $ch = $cch->getChannel();
	  $v = $cch->getVal();
	  $v = $v * 50. / 1000.; // in ns
	  $s = 's';
	  if($v < 1e+3){
	    $v = $v;
	    $s = 'ns';
	  }else if($v < 1e+6){
	    $v = $v / 1000.;
	    $s = 'us';
	  }else if($v < 1e+9){
	    $v = $v / 1000. / 1000.;
	    $s = 'ms';
	  }else{
	    $v = 100;
	    $s = 'ovf';
	  }
	  printf("%2d %6.2f %-3s ", $ch, $v, $s);
	  $lines++;
	}
	if($lines >= 3){
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
