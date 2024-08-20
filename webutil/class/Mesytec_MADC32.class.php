<?php

require_once('module.class.php');

class Mesytec_MADC32 {
  public $module = array();

  const MSK      = 0xc0000000;
  const MSKSFT   = 30;
  const HEADER   = 0x01;
	const SUBH     = 0x0fe00000;
	const DTSUBH   = 0x04000000;
  const DATA     = 0x00;
  const ENDER    = 0x11;
  const GEOMSK   = 0x00ff0000;
  const GEOSFT   = 16;
  const CHMSK    = 0x001f0000;
  const CHSFT    = 16;
  const ADCMSK   = 0x0000ffff;
  const ADCSFT   = 0;

  function decode($buff){
    $flag = 0;
    $list = unpack("I*", $buff);
    $cnt = count($list);
    for($i=1;$i<=$cnt;$i++){
      $msk = ($list[$i] & self::MSK) >> self::MSKSFT;
      if($msk == self::ENDER){
				$flag = 0;
				continue;
      }else if($msk == self::HEADER && !$flag){
				$geo = ($list[$i] & self::GEOMSK) >> self::GEOSFT;
				$mod = new CModuleData($geo);
				array_push($this->module, $mod);
				$flag = 1;
				continue;
      }else if($msk == self::DATA && $flag){
				if(($list[$i] & self::SUBH) == self::DTSUBH){
					$ch =  ($list[$i] & self::CHMSK) >> self::CHSFT;
					$val = ($list[$i] & self::ADCMSK) >> self::ADCSFT;
					
					$mod->setChannelVal($ch, $val);
				}
				continue;
      }else{
				$flag = 0;
				continue;
      }
    }
  }

  function show_decode(){
    echo "<pre>";
    foreach ($this->module as $mod){
      printf("geo=%2d ",$mod->getGeo());
      $lines = 0;
      for($i=0;$i<32;$i++){
	if($ch = $mod->getCh($i)){
	  printf("%2d:%4d ",$i, $ch->getVal());
	  $lines++;
	}
	if($lines >= 8){
	  $lines = 0;
	  echo "\n       ";
	}
      }
      echo "\n";
    }
    echo "</pre>";
  }
}

class Mesytec_MTDC32 {
  public $module = array();

  const MSK      = 0xc0000000;
  const MSKSFT   = 30;
  const HEADER   = 0x01;
  const DATA     = 0x00;
  const ENDER    = 0x11;
  const GEOMSK   = 0x00ff0000;
  const GEOSFT   = 16;
  const CHMSK    = 0x003f0000;
  const CHSFT    = 16;
  const TDCMSK   = 0x0000ffff;
  const TDCSFT   = 0;
  const TDCFIX   = 0x04800000;
  const TDCFDATA = 0x04000000;
  const TDCFTS   = 0x00800000;

  function decode($buff){
    $flag = 0;
    $list = unpack("I*", $buff);
    $cnt = count($list);
    for($i=1;$i<=$cnt;$i++){
      $msk = ($list[$i] & self::MSK) >> self::MSKSFT;
      if($msk == self::ENDER){
				$flag = 0;
				continue;
      }else if($msk == self::HEADER && !$flag){
				$geo = ($list[$i] & self::GEOMSK) >> self::GEOSFT;
				$mod = new CModuleData($geo);
				array_push($this->module, $mod);
				$flag = 1;
				continue;
      }else if($msk == self::DATA && $flag){
				if(($list[$i] & self::TDCFIX) == self::TDCFDATA){
					$ch =  ($list[$i] & self::CHMSK) >> self::CHSFT;
					$val = ($list[$i] & self::TDCMSK) >> self::TDCSFT;
					$mod->setChannelVal($ch, $val);
					continue;
				}
      }else{
				$flag = 0;
				continue;
      }
    }
  }

  function show_decode(){
    echo "<pre>";
    foreach ($this->module as $mod){
      printf("geo=%2d ",$mod->getGeo());
      $lines = 0;
      for($i=0;$i<34;$i++){
				if($ch = $mod->getCh($i)){
					printf("%2d:%4d ",$i, $ch->getVal());
					$lines++;
				}
				if($lines >= 8){
					$lines = 0;
					echo "\n       ";
				}
      }
      echo "\n";
    }
    echo "</pre>";
  }
}

class Mesytec_MQDC32 extends Mesytec_MADC32{
}

class Mesytec_MXDC32_CVME extends CVME {
  public $comment = 'for the interrupt generateion, it should be set in the readout driver';

  public $am = self::A32;
  public $fcolor = self::COL_FWAD;
  public $bcolor = self::COL_MESYTEC;

  public $val_resolution = array(0 => '2k (800ns coversion time)',
			         1 => '4k (1.6us conversion time)',
			         2 => '4k hires (3.2us conversion time)',
			         3 => '8k (6.4 us conversion time)',
			         4 => '8k hires (12.5us conversion time)');

  public $enabits_mcstctrl = array(
		0 => array('MCST',
							 'Enable', '6', '1', '7', '1',
							 'Disable', '6', '0', '6', '1'),
		1 => array('CBLT',
							 'Enable', '0', '1', '1', '1',
							 'Disable', '0', '0', '0', '1'),
		2 => array('FIRST',
							 'Enable', '4', '1', '5', '1',
							 'Disable', '4', '0', '4', '1'),
		3 => array('LAST',
							 'Enable', '2', '1', '3', '1',
							 'Disable', '2', '0', '2', '1'),
	);


  public $reg
    = array(
	    'firmware'  => array('mode' => 'd16',
				 'addr' => '0x600E',
				 'titl' => 'Firmware',
				 'show' => 'show_char16_dot',
				 'offv' => '0x00'
				 ),
	    'moduleid'  => array('mode' => 'd16',
				 'addr' => '0x6004',
				 'mask' => '0xff',
				 'edit' => 'edit_number',
				 'rang' => '0:255',
				 'titl' => 'Module ID',
				 'offv' => '0x00'
				 ),
	    'mcstctrl'  => array('mode' => 'd16',
				 'addr' => '0x6020',
				 'mask' => '0xff',
				 'enabits'  => 'enabits_mcstctrl',
				 'titl' => 'Multicast Control',
				 'comt' => 'only for MSCT/CBLT',
				 'offv' => '0x00'
				 ),
	    'cblt'      => array('mode' => 'd16',
				 'addr' => '0x6022',
				 'mask' => '0xff',
				 'edit' => 'edit_hex',
				 'rang' => '2',
				 'titl' => 'CBLT address',
				 'offv' => '0xaa'
				 ),
	    'reset'     => array('mode' => 'd16',
				 'addr' => '0x6008',
				 'defw' => '1'
				 ),
	    'resolution'  => array('mode' => 'd16',
				 'addr' => '0x6042',
				 'mask' => '0x0f',
				 'val'  => 'val_resolution',
				 'titl' => 'ADC Resolution',
				 'offv' => '4'
				 ),
	    'threshold' => array('mode' => 'd16',
				 'addr' => '0x4000',
				 'nch'  => '32',
				 'mask' => '0x1FFFF',
				 'titl' => 'Threshold',
				 'edit' => 'edit_number',
				 'rang' => '0:8191',
				 'offv' => '0x00',
				 'botn' => array(
						 array('All Zero', 'js_setVal', '0'),
						 array('All Disable', 'js_setVal', '8191')
						 )
				 )
		      );

  public $readparam = array('firmware', 'moduleid',
			    'resolution',
			    'threshold');

  public $showparam = array('firmware', 'moduleid', 
			    'resolution', 
			    'threshold');

  public $writeparam = array('reset', 'sleep1', 'moduleid',
														 'resolution', 'sleep1',
														 'threshold');
  
}

class Mesytec_MADC32_VSTA extends Mesytec_MXDC32_CVME {
  const DESC = 'Mesytec MADC32';
  const FUNC = '32ch ADC';

  public $val_inputrange = array(0 => '4V',
                                 1 => '10V',
                                 2 => '8V');

  public $val_nim0 = array(0 => 'as Busy',
                           1 => 'as Gate0');

	public $readparam = array('firmware', 'moduleid',
                              'resolution', 'inputrange',
                              'cblt','mcstctrl','nim0',
                              'threshold');
	
	public $showparam = array('firmware', 'moduleid', 
                              'resolution', 'inputrange',
                              'cblt','mcstctrl','nim0',
                              'threshold');

	public $writeparam = array('reset', 'sleep1', 'moduleid',
                               'resolution', 'sleep1',
                               'inputrange', 'sleep1',
                               'cblt','mcstctrl','nim0',
                               'threshold');
  function __construct(){
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);


    $this->reg['inputrange']
        = array
	    ('mode' => 'd16',
         'addr' => '0x6060',
         'mask' => '0x03',
         'val'  => 'val_inputrange',
         'titl' => 'Input Range',
         'offv' => '0');

    $this->reg['nim0']
        = array
	    ('mode' => 'd16',
         'addr' => '0x606E',
         'mask' => '0x03',
         'val'  => 'val_nim0',
         'titl' => 'NIM0',
         'offv' => '0');
  }

}

class Mesytec_MQDC32_VSTA extends Mesytec_MXDC32_CVME {
  const DESC = 'Mesytec MQDC32';
  const FUNC = '32ch QDC';

  public $readparam = array('firmware', 'moduleid',
                            'threshold');

  public $showparam = array('firmware', 'moduleid', 
                            'threshold');

  public $writeparam = array('reset', 'sleep1', 'moduleid',
                             'sleep1',
                             'threshold');


  function __construct(){
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
  }
}


class Mesytec_MTDC32_VSTA extends Mesytec_MXDC32_CVME {
  const DESC = 'Mesytec MTDC32';
  const FUNC = '32ch Multihit TDC';

  public $readparam = array('firmware', 'moduleid',
														'cblt','mcstctrl',
														'resolution', 'marking',
														'firsthit',
														'winoffset', 'winwidth');

  public $showparam = array('firmware', 'moduleid', 
														'cblt','mcstctrl',
														'resolution', 'marking',
														'firsthit',
														'winoffset', 'winwidth');
  public $writeparam = array('reset', 'sleep1', 'moduleid',
														'cblt','mcstctrl',
														 'resolution', 'sleep1',
														 'marking', 'sleep1',
														 'firsthit', 'sleep1',
														 'winwidth', 'sleep1', 
														 'winoffset', 'sleep1');

	public $val_marking
		= array(0 => 'Event counter',
						1 => 'Time Stamp',
						3 => 'Extended Time Stamp');

	public $val_firsthit
		= array(0 => 'All Hits (multi hit)',
						3 => 'Only First Hit (single hit)');

  public $val_resolution
		= array(2 => '3.90625 ps',
						3 => '7.8125 ps',
						4 => '15.625 ps',
						5 => '31.250 ps',
						6 => '62.500 ps',
						7 => '125.00 ps',
						8 => '250.00 ps',
						9 => '500.00 ps');
	
	function __construct(){
		// override
		$this->reg['resolution']
			= array('mode' => 'd16',
							'addr' => '0x6042',
							'mask' => '0x0f',
							'val'  => 'val_resolution',
							'titl' => 'TDC Resolution',
							'offv' => '5');
		$this->reg['winoffset']
			= array('mode' => 'd16',
							'addr' => '0x6050',
							'subr' => '0x6052',
							'offv' => '0',
							'rang' => '0:32767',
							 'edit' => 'edit_number',
							 'titl' => 'Window Offset',
							 'comt' => '16k=no offset, <16k -offset, >16k +offset',
							 'norm' => 0.001,
							 'nout' => 'us');
		$this->reg['winwidth']
			= array('mode' => 'd16',
							'addr' => '0x6054',
							'subr' => '0x6056',
							'offv' => '0',
							'rang' => '0:16383',
							'edit' => 'edit_number',
							'titl' => 'Window Width',
							'norm' => 0.001,
							'nout' => 'us',
							'winmc' => 'Current TDC Window is ');
		$this->reg['marking']
			= array('mode' => 'd16',
							'addr' => '0x6038',
							'mask' => '0x03',
							'val'  => 'val_marking',
							'titl' => 'Marking Type',
							'offv' => '0');
		$this->reg['firsthit']
			= array('mode' => 'd16',
							'addr' => '0x605c',
							'mask' => '0x03',
							'val'  => 'val_firsthit',
							'titl' => 'First Hit',
							'offv' => '3');
		
		unset($this->reg['threshold']);
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);

	}
}

?>
