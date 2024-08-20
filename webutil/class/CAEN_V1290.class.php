<?php

require_once('module.class.php');

class CAEN_V1290 {
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
   const kMaskChannel       = 0x03e00000;
   const kMaskMeasure       = 0x001fffff;
   const kMaskEdgeType      = 0x04000000;
   const kShiftGeometry     = 0;
   const kShiftEventCounter = 5;
   const kShiftBunchID      = 0;
   const kShiftEventID      = 12;
   const kShiftChannel      = 21;
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


class CAEN_V1X90_CVME extends CVME {
  public $comment = 'for the interrupt generateion, it should be set in the readout driver';
  
  public $am = self::A32;
  public $fcolor = self::COL_FWAD;
  public $bcolor = self::COL_CAEN;

  public $bits_ctrl = array(0 => array('BERR_EN',
				       'no BERR (can enable EVENT_FIFO)',
				       'use BERR (should disable EVENT_FIFO)'),
			    3 => array('EMPTY_EVENT',
				       'no data when no valid hit',
				       'always headers written'),
			    5 => array('COMPENSATION_ENABLE',
				       'INL compensation disable (need calibration)',
				       'INL compensation enable (default)'),
			    8 => array('EVENT_FIFO_ENABLE',
				       'event FIFO disable (can use BERR)',
				       'event FIFO enable (should not enable BERR)')
			    
			    );
  
  public $val_mcstctrl = array(0 => 'Disable',
			       1 => 'Last board',
			       2 => 'First board',
			       3 => 'Intermediate board');
  public $val_edge = array(1 => 'Trailing only',
			   2 => 'Leading only',
			   3 => 'Both edge');
  
  public $opqn_trigger = array(0 => array('Continuous Storage', '0x0100'),
			       1 => array('Trigger Mathing', '0x0000'));
  public $opqr_window = array(0 => 'winwidth',
			      1 => 'winoffset',
			      2 => 'extra',
			      3 => 'reject',
			      4 => 'subtract');
  
  public $reg
    = array(
	    'trigger'   => array('mode' => 'op',
				 'opqn' => 'opqn_trigger',
				 'opra' => '0x0200',
				 'titl' => 'Set Trigger Mode',
				 'oprn' => '1',
				 'comt' => 'Usually use Trigger Mathing Mode',
				 'offv' => '1',
				 ),
	    'window'    => array('mode' => 'op',
				 'opra' => '0x1600',
				 'oprn' => '5',
				 'opqr' => 'opqr_window'),
	    'winwidth'  => array('mode' => 'op',
				 'offv' => '80',
				 'opqw' => '0x1000',
				 'rang' => '1:4095',
				 'opqr' => 'opqr_window',
				 'edit' => 'edit_number',
				 'titl' => 'Window Width',
				 'norm' => 0.025,
				 'nout' => 'us'),
	    'winoffset' => array('mode' => 'op',
				 'offv' => '-40',
				 'opqw' => '0x1100',
				 'rang' => '-2048:40',
				 'opqr' => 'opqr_window',
				 'edit' => 'edit_number',
				 'sign' => '0xffff',
				 'titl' => 'Window Offset',
				 'norm' => 0.025,
				 'nout' => 'us',
				 'winc' => 'Current TDC Window is ',
				 'wine' => ' (<font color=red><b>invalid!</b></font> Width+Offset < 40 (1us))',
				 ),
	    'extra'     => array('mode' => 'op',
				  'opqw' => '0x1200',
				  'defw' => '0x08',
				  'titl' => 'Extra Search Margin'),
	    'reject'    => array('mode' => 'op',
				  'opqw' => '0x1300',
				  'defw' => '0x02',
				  'titl' => 'Reject Margin'),
	    'subtract'  => array('mode' => 'op',
				 'dfqn' => '0x1400',
				 'titl' => 'Enable Subtraction Trigger Time'),
	    'edge'      => array('mode' => 'op',
				 'opra' => '0x2300',
				 'offv' => '2',
				 'oprn' => '1',
				 'opqw' => '0x2200',
				 'val'  => 'val_edge',
				 'titl' => 'TDC Edge'),
	    'firmware'  => array('mode' => 'd16',
				 'addr' => '0x1026',
				 'titl' => 'Firmware',
				 'show' => 'show_char8_dot'
				 ),
	    'geo'       => array('mode' => 'd16',
				 'addr' => '0x100e',
				 'mask' => '0x1f',
				 'edit' => 'edit_number',
				 'rang' => '0:31',
				 'offv' => '0',
				 'titl' => 'Geometry Address'
				 ),
	    'mcst'      => array('mode' => 'd16',
				 'addr' => '0x1010',
				 'mask' => '0xff',
				 'edit' => 'edit_hex',
				 'rang' => '2',
				 'offv' => '0xaa',
				 'titl' => 'Multicast address'
				 ),
	    'mcstctrl'  => array('mode' => 'd16',
				 'addr' => '0x1012',
				 'mask' => '3',
				 'offv' => '0',
				 'val'  => 'val_mcstctrl',
				 'titl' => 'Multicast Control',
				 'comt' => 'only for MSCT/CBLT'
				 ),
	    'intlevel'  => array('mode' => 'd16',
				 'addr' => '0x100A',
				 'mask' => '0x7',
				 'rang' => '0-7',
				 'titl' => 'Interrupt Level'
				 ),
	    'intvector' => array('mode' => 'd16',
				 'addr' => '0x100C',
				 'mask' => '0xff',
				 'rang' => 'h:0xFF',
				 'titl' => 'Interrupt Vector'
				 ),
	    'almostfull'=> array('mode' => 'd16',
				 'addr' => '0x1022',
				 'edit' => 'edit_number',
				 'rang' => '1-32735',
				 'offv' => '5',
				 'titl' => 'Almost Full',
				 'comt' => '5 is a good value for event-by-event readout. Should enable EMPTY_EVENT'
				 ),
	    'ctrl'      => array('mode' => 'd16',
				 'addr' => '0x1000',
				 'bits' => 'bits_ctrl',
				 'offv' => '0x128',
				 'titl' => 'Control Register',
				 'comt' => 'If use IRQ, enable EVMPTY_EVENT. If don\'t calibrate, enable COMPENSATION_ENABLE'
				 ),
	    'reset'     => array('mode' => 'd16',
				 'addr' => '0x1014',
				 'defw' => '1'
				 ),
	    'clear'     => array('mode' => 'd16',
				 'addr' => '0x1016',
				 'defw' => '1'
				 )
	    );

  
  public $readparam = array('firmware', 'geo', 'mcst', 'mcstctrl', 'ctrl',
														'almostfull' ,'trigger', 'edge', 'window');
  
  public $showparam = array('firmware', 'geo', 'mcst', 'mcstctrl', 'ctrl',
														'almostfull', 'trigger', 'edge', 'winwidth', 'winoffset');
  
  public $writeparam = array('geo', 'reset', 'clear', 'sleep0',
														 'mcst', 'mcstctrl', 'ctrl', 'almostfull',
														 'sleep1', 'trigger', 
															 'sleep1', 'edge', 
														 'sleep1', 'winwidth', 
			     'sleep1', 'winoffset', 
			     'sleep1', 'subtract');
  
}

class CAEN_V1290_VSTA extends CAEN_V1X90_CVME {
  const DESC = 'CAEN V1290';
  const FUNC = '32ch 25ps Multihit TDC';

  function __construct(){
    //parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
  }
}

class CAEN_V1290N_VSTA extends CAEN_V1X90_CVME {
  const DESC = 'CAEN V1290N';
  const FUNC = '16ch 25ps Multihit TDC';

  function __construct(){
    //parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
  }
}


?>
