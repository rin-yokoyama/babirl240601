<?php

require_once('module.class.php');

class CAEN_V7XX {
  public $module = array();

  const HDMASK   = 0x06000000;
  const HDVALID  = 0x06000000;
  const HDHEADER = 0x02000000;
  const HDEOB    = 0x04000000;
  const HDDATA   = 0x00000000;
  const GEOMASK  = 0xf8000000;
  const GEOSFT   = 27;
  const CNTMASK  = 0x00003f00;
  const CNTSFT   = 8;
  const CHMASK   = 0x001f0000;
  const CHSFT    = 16;
  const ADCMASK  = 0x00001fff;
  const ADCSFT   = 0;

  function decode($buff){
    $flag = 0;
    $list = unpack("I*", $buff);
    $cnt = count($list);
    for($i=1;$i<=$cnt;$i++){
      $hd = $list[$i] & self::HDMASK;
      if($hd == self::HDVALID){
	$flag = 0;
	continue;
      }else if($hd == self::HDHEADER && !$flag){
	$geo = ($list[$i] & self::GEOMASK) >> self::GEOSFT;
	$mod = new CModuleData($geo);
	array_push($this->module, $mod);
	$flag = 1;
      }else if($hd == self::HDDATA && $flag){
	$ch =  ($list[$i] & self::CHMASK) >> self::CHSFT;
	$val = ($list[$i] & self::ADCMASK) >> self::ADCSFT;
	$mod->setChannelVal($ch, $val);
      }else if($hd == self::HDEOB && $flag){
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


class CAEN_V7XX_CVME extends CVME {
  public $comment = 'for the interrupt generateion, it should be set in the readout driver';

  public $am = self::A32;
  public $fcolor = self::COL_FWAD;
  public $bcolor = self::COL_CAEN;

  public $bits_ctrl1 = array(2 => array('BLKEND',
					'read all data (BLT+BERR)',
					'read 1 event data (BLT+BERR)'),
			     5 => array('BERR_ENABLE',
					'BERR is disabled',
					'BERR is enabled'));
  // 4 => 'PROG RESET' not use
  //'6' => 'ALIGN64' not use

  public $bits_bitset2 = array(3 => array('OVERRANGE_EN',
					  'overflow suppression',
					  'no overflow suppression'),
			       4 => array('LOW_THR_EN',
					  'zoro suppression',
					  'no zero suppression'),
			       8 => array('STEP_TH',
					  'Threshold = TH x 16',
					  'Threshold = TH x 2')
			       );

  public $val_mcstctrl = array(0 => 'Disable',
			       1 => 'Last board',
			       2 => 'First board',
			       3 => 'Intermediate board');
					   

  public $reg
    = array(
	    'firmware'  => array('mode' => 'd16',
				 'addr' => '0x1000',
				 'titl' => 'Firmware',
				 'show' => 'show_char16_dot',
				 'offv' => '0x00'
				 ),
	    'geo'       => array('mode' => 'd16',
				 'addr' => '0x1002',
				 'mask' => '0x1f',
				 'edit' => 'edit_number',
				 'rang' => '0:31',
				 'titl' => 'Geometry Address',
				 'comt' => '= slot number if board with PAUX connector',
				 'offv' => '0x00'
				 ),
	    'mcst'      => array('mode' => 'd16',
				 'addr' => '0x1004',
				 'mask' => '0xff',
				 'edit' => 'edit_hex',
				 'rang' => '2',
				 'titl' => 'Multicast address',
				 'offv' => '0xaa'
				 ),
	    'bitset1'   => array('mode' => 'd16',
				 'addr' => '0x1006',
				 'titl' => 'Bit Set 1'
				 ),
	    'bitclear1' => array('mode' => 'd16',
				 'addr' => '0x1008'
				 ),
	    'intlevel'  => array('mode' => 'd16',
				 'addr' => '0x100A',
				 'mask' => '0x7',
				 'rang' => '0:7',
				 'titl' => 'Interrupt Level'
				 ),
	    'intvector' => array('mode' => 'd16',
				 'addr' => '0x100C',
				 'mask' => '0xff',
				 'rang' => '0xFF',
				 'titl' => 'Interrupt Vector'
				 ),
	    'status1'   => array('mode' => 'd16',
				 'addr' => '0x100E',
				 'titl' => 'Status1'
				 ),
	    'ctrl1'     => array('mode' => 'd16',
				 'addr' => '0x1010',
				 'bits' => 'bits_ctrl1',
				 'titl' => 'Control Register1',
				 'comt' => 'affects only for BLT',
				 'offv' => '0x00'
				 ),
	    'reset'     => array('mode' => 'd16',
				 'addr' => '0x1016',
				 'defw' => '1'
				 ),
	    'mcstctrl'  => array('mode' => 'd16',
				 'addr' => '0x101A',
				 'mask' => '3',
				 'val'  => 'val_mcstctrl',
				 'titl' => 'Multicast Control',
				 'comt' => 'only for MSCT/CBLT',
				 'offv' => '0x00'
				 ),
	    'evttrg'    => array('mode' => 'd16',
				 'addr' => '0x1020',
				 'titl' => 'Event trigger'
				 ),
	    'status2'   => array('mode' => 'd16',
				 'addr' => '0x1022'
				 ),
	    'bitset2'   => array('mode' => 'd16',
				 'addr' => '0x1032',
				 'bits' => 'bits_bitset2',
				 'titl' => 'Bit Set 2',
				 'offv' => '0x00'
				 ),
	    'bitclear2' => array('mode' => 'd16',
				 'addr' => '0x1034',
				 'defw' => '0x0118'
				 ),
	    'threshold' => array('mode' => 'd16',
				 'addr' => '0x1080',
				 'nch'  => '32',
				 'mask' => '0x1FF',
				 'titl' => 'Threshold',
				 'edit' => 'edit_number',
				 'rang' => '0:256',
				 'offv' => '0x00',
				 'comt' => '256 = disable',
				 'mval' => '256',
				 'mcom' => ' (disable)',
				 'norm' => array(16, 2),
				 'nobt' => 'bitset2',
				 'nobs' => 8,
				 'nout' => 'ch',
				 'botn' => array(
						 array('All Zero', 'js_setVal', '0'),
						 array('All Disable', 'js_setVal', '256')
						 )
				 )
		      );

  public $readparam = array('bitset2', 
			    'firmware', 'geo', 'mcst',
			    'ctrl1', 'mcstctrl',
			    'threshold');

  public $showparam = array('firmware', 'geo', 
			    'bitset2', 
			    'mcst', 'mcstctrl',
			    'ctrl1', 
			    'threshold');
  public $writeparam = array('geo', 'reset', 'bitclear2', 'bitset2',
			     'mcst', 'mcstctrl', 'ctrl1', 'threshold');
  
}

class CAEN_V792_VSTA extends CAEN_V7XX_CVME {
  const DESC = 'CAEN V792';
  const FUNC = '32ch QDC';

  function __construct(){
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
    $comment = 'should increase until pedestal peak is clearly visible with reasonable width (not too narrow)<br>'.$this->comment;

    $iped = array(
	    'iped'      => array('mode' => 'd16',
				 'addr' => '0x1060',
				 'mask' => '0xff',
				 'rang' => '0:255',
				 'offv' => '200',
				 'edit' => 'edit_number',
				 'titl' => 'IPED',
				 'comt' => 'Pedestal value'
				 )
		  );

    $this->reg = array_merge($this->reg, $iped);
    array_push($this->readparam, 'iped');
    array_splice($this->showparam, count($this->showparam)-1, 0, 'iped');
    array_push($this->writeparam, 'iped');

  }

}

class CAEN_V792N_VSTA extends CAEN_V792_VSTA {
  const DESC = 'CAEN V792N 16ch QDC';
  const FUNC = '16ch QDC';

  function __construct(){
    parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
    $this->reg['threshold']['nch'] = 16;
    $this->reg['threshold']['step'] = 4;
  }
}

class CAEN_V775_VSTA extends CAEN_V7XX_CVME {
  const DESC = 'CAEN V775 32ch TDC';
  const FUNC = '32ch TDC';

  function __construct(){
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);

    $fsr = array(
		  'fsr' => array('mode' => 'd16',
				 'addr' => '0x1060',
				 'mask' => '0xff',
				 'rang' => '0:255',
				 'edit' => 'edit_number',
				 'titl' => 'Full Scale Range',
				 'offv' => '112',
				 'comt' => '230=150ns, 112=300ns, 66=500ns, 30=1000ns'
				 )
		  );

    $this->reg = array_merge($this->reg, $fsr);
    array_push($this->readparam, 'fsr');
    array_splice($this->showparam, count($this->showparam)-1, 0, 'fsr');
    array_push($this->writeparam, 'fsr');

  }

}

class CAEN_V775N_VSTA extends CAEN_V775_VSTA {
  const DESC = 'CAEN V775N 16ch TDC';
  const FUNC = '16ch TDC';

  function __construct(){
    parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
    $this->reg['threshold']['nch'] = 16;
    $this->reg['threshold']['step'] = 4;
  }
}

class CAEN_V785_VSTA extends CAEN_V7XX_CVME {
  const DESC = 'CAEN V785 32ch ADC';
  const FUNC = '32ch ADC';

  function __construct(){
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
    $comment = 'input signal should have positive groud level<br>'.$this->comment;
  }

}

class CAEN_V785N_VSTA extends CAEN_V785_VSTA {
  const DESC = 'CAEN V785N 16ch ADC';
  const FUNC = '32ch ADC';

  function __construct(){
    parent::__construct();
    $this->setName(self::DESC);
    $this->setFunc(self::FUNC);
    $this->reg['threshold']['nch'] = 16;
    $this->reg['threshold']['step'] = 4;
  }
}

?>
