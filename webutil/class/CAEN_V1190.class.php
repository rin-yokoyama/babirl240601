<?php

require_once('module.class.php');
require_once('CAEN_V1290.class.php');

class CAEN_V1190 {
  public $module = array();
  public $edgec = array(0=>'L', 1=>'T');

   const kHeaderMask        = 0xf8000000;
   const kGlobalHeader      = 0x40000000;
   const kTDCHeader         = 0x08000000;
   const kTDCMeasurement    = 0x00000000;
   const kTDCTrailer        = 0x18000000;
   const kTDCError          = 0x20000000;
   const kGlobalTrailer     = 0x80000000;
   const kMaskGeometry      = 0x0000001f;
   const kMaskEventCounter  = 0x7ffffe0;
   const kMaskBunchID       = 0x00000fff;
   const kMaskEventID       = 0x00000fff;
   const kMaskChannel       = 0x03f80000;
   const kMaskMeasure       = 0x0007ffff;
   const kMaskEdgeType      = 0x04000000;
   const kShiftGeometry     = 0;
   const kShiftEventCounter = 5;
   const kShiftBunchID      = 0;
   const kShiftEventID      = 12;
   const kShiftChannel      = 19;
   const kShiftMeasure      = 0;
   const kShiftEdgeType     = 26;

  function decode($buff){
    $evtdata = unpack("I*", $buff);
    $cnt = count($evtdata);
    for($i=1;$i<=$cnt;$i++) {
      $ih = $evtdata[$i]&self::kHeaderMask;
      if($ih == self::kGlobalHeader) {
        $ghf = 1;
        $igeo = ($evtdata[$i]&self::kMaskGeometry)>>self::kShiftGeometry;
	$mod = new CModuleData($igeo);
	array_push($this->module, $mod);
      } else if ($ih == self::kTDCHeader) {
        if ($ghf != 1) break;
        $bncid = ($evtdata[$i]&self::kMaskBunchID)>>self::kShiftBunchID;
        $evtid = ($evtdata[$i]&self::kMaskEventCounter)>>self::kShiftEventCounter;
      } else if ($ih == self::kTDCMeasurement) {
        $ch = ($evtdata[$i]&self::kMaskChannel) >> self::kShiftChannel;
        $edge = ($evtdata[$i]&self::kMaskEdgeType) >> self::kShiftEdgeType;
        $val = ($evtdata[$i]&self::kMaskMeasure) >> self::kShiftMeasure;
	$mod->setChannelEdgeVal($ch, $edge, $val);
      } else if ($ih == self::kTDCTrailer) {
      } else if ($ih == self::kTDCError) {
      } else if ($ih == self::kGlobalTrailer) {
	$ghf = 0;
      }
   }
  }

  function show_decode(){
    echo "<pre>";
    foreach ($this->module as $mod){
      printf("geo=%2d ",$mod->getGeo());
      $lines = 0;
      for($i=0;$i<128;$i++){
	for($j=0;$j<2;$j++){
	  if($ch = $mod->getChEdge($i, $j)){
	    printf("%3d:%s:%6d ",$i, $this->edgec[$j], $ch->getVal());
	    $lines++;
	  }
	  if($lines >= 4){
	    $lines = 0;
	    echo "\n       ";
	  }
	}
      }
      echo "\n";
    }
    echo "</pre>";
  }
}

class CAEN_V1190A_VSTA extends CAEN_V1X90_CVME {
  const DESC = 'CAEN V1190A';
  const FUNC = '128ch 100ps Multihit TDC';

  function __construct(){
    //parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
  }
}

class CAEN_V1190B_VSTA extends CAEN_V1X90_CVME {
  const DESC = 'CAEN V1190B';
  const FUNC = '64ch 100ps Multihit TDC';

  function __construct(){
    //parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
  }
}


?>
