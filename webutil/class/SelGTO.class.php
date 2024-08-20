<?php

require_once('SkelGTO.class.php');

class CSelGTO extends CGTO {
	public $out = array();
	public $ds = array();
	public $rate = array();

	const NOFOUT   = 8;
	const NOFPARAM = 32;
	const NOFIN    = 20;
	const EOFDS    = 28;
	const NOF1K    = 28;
	const NOF10K   = 29;
	const NOFLEVEL = 30;
	const NOFVETO  = 31;
	
	public $seladv = array (
		array ("param"=>"in0",   "val"=>0x00000001),
		array ("param"=>"in1",   "val"=>0x00000002),
		array ("param"=>"in2",   "val"=>0x00000004),
		array ("param"=>"in3",   "val"=>0x00000008),
		array ("param"=>"in4",   "val"=>0x00000010),
		array ("param"=>"in5",   "val"=>0x00000020),
		array ("param"=>"in6",   "val"=>0x00000040),
		array ("param"=>"in7",   "val"=>0x00000080),
		array ("param"=>"in8",   "val"=>0x00000100),
		array ("param"=>"in9",   "val"=>0x00000200),
		array ("param"=>"in10",  "val"=>0x00000400),
		array ("param"=>"in11",  "val"=>0x00000800),
		array ("param"=>"in12",  "val"=>0x00001000),
		array ("param"=>"in13",  "val"=>0x00002000),
		array ("param"=>"in14",  "val"=>0x00004000),
		array ("param"=>"in15",  "val"=>0x00008000),
		array ("param"=>"in16",  "val"=>0x00010000),
		array ("param"=>"in17",  "val"=>0x00020000),
		array ("param"=>"in18",  "val"=>0x00040000),
		array ("param"=>"in19",  "val"=>0x00080000),
		array ("param"=>"ds0",   "val"=>0x00100000),
		array ("param"=>"ds1",   "val"=>0x00200000),
		array ("param"=>"ds2",   "val"=>0x00400000),
		array ("param"=>"ds3",   "val"=>0x00800000),
		array ("param"=>"ds4",   "val"=>0x01000000),
		array ("param"=>"ds5",   "val"=>0x02000000),
		array ("param"=>"ds6",   "val"=>0x04000000),
		array ("param"=>"ds7",   "val"=>0x08000000),
		array ("param"=>"1k",    "val"=>0x10000000),
		array ("param"=>"10k",   "val"=>0x20000000),
		array ("param"=>"level", "val"=>0x40000000),
		array ("param"=>"veto",  "val"=>0x80000000),
		array ("param"=>"none",  "val"=>0x00000000),
		array ("param"=>"", "val"=>-1)
	);

	public $adr_out = array(
		 0,  4,  8, 12, 16, 20, 24, 28);

	public $adr_dsrate = array(
		32, 35, 38, 41, 44, 47, 50, 53);

	public $adr_dssel = array(
		56, 57, 58, 59, 60, 61, 62, 63);

	public $uadr_rst   = 0;
	public $uadr_pulse = 1;
	public $uadr_test  = 2;
	

	function idxseladv($ip){
		$i=0;
		foreach($this->seladv as $sel){
			if(!strcmp($sel['param'], $ip)){
				return $i;
			}
			$i++;
		}
		return 0;
	}

	function decode($data){
		$ldata = unpack("L*", $data);
		$cdata = unpack("C*", $data);
		for($i=0;$i<8;$i++){
			$this->out[$i] = $ldata[$i+1];
		}
		for($i=56;$i<64;$i++){
			$this->ds[$i-56] = $cdata[$i+1];
		}
		for($i=0;$i<8;$i++){
			$this->rate[$i]
				= ($cdata[1+34+$i*3] << 16) | ($cdata[1+33+$i*3] << 8) | $cdata[1+32+$i*3];
		}
	}

	function encout($chs){
		$ret = 0;

		foreach($chs as $nm){
			if(!strcmp('sig', $nm)) continue;
			if(!strcmp('nsig', $nm)) continue;
			if(!strcmp('or', $nm)) continue;
			if(!strcmp('and', $nm)) continue;

			for($i=0;$i<self::NOFPARAM;$i++){
				if(!strcmp($nm, $this->seladv[$i]['param'])){
					$ret |= $this->seladv[$i]['val'];
					break;
				}
			}
		}

		return $ret;
	}

	function strparam($chs){
		$ret = '';

		foreach($chs as $nm){
			if(!strcmp('sig', $nm)) continue;
			if(!strcmp('nsig', $nm)) continue;
			if(!strcmp('or', $nm)) continue;
			if(!strcmp('and', $nm)) continue;

			for($i=0;$i<self::NOFPARAM;$i++){
				if(!strcmp($nm, $this->seladv[$i]['param'])){
					$ret .= " ".$this->seladv[$i]['param'];
					break;
				}
			}
		}

		return $ret;
	}


	function selout($n, $chs){
		$val = $this->encout($chs);
		parent::write32($this->adr_out[$n], $val);
	}

	function chkout($o, $in){
		return $this->out[$o] & $this->seladv[$in]['val'];
	}

	function chkdata($d, $in){
		return $d & $this->seladv[$in]['val'];
	}

	function dataparam($n){
		$ret = array();
		for($i=0;$i<self::NOFPARAM;$i++){
			if($this->chkout($n, $i)){
				array_push($ret, $this->seladv[$i]['param']);
			}
		}
		return $ret;
	}


	function selectedds($i){
		if($this->ds[$i] == 20){
			return '50M';
		}else{
			return $this->seladv[$this->ds[$i]]['param'];
		}
	}


	function showsel($val){
		if($val == 0){
			return "none";
		}
		$i = 0;
		$ret = '';

		while($this->seladv[$i]['val'] != -1){
			if($val & $this->seladv[$i]['val']){
				$ret .= $this->seladv[$i]['param'];
			}
			$i++;
		}

		return $ret;
	}

	function convdivide($val){
		$ret = $val;
		$kv = $val/1000.;
		$mv = $kv/1000.;
		if($mv >= 10.){
			$ret = $mv."M";
		}else if($kv >= 10.){
			$ret = $kv."k";
		}

		return $ret;
	}

	function dssel($ch, $nm){
		$n = -1;
		for($i=0;$i<self::NOFIN;$i++){
			if(!strcmp($nm, $this->seladv[$i]['param'])){
				$n = $i;
			}
		}
		if(!strcmp($nm, "50M")){
			$n = 20;
		}
		if($n >= 0){
			parent::write($this->adr_dssel[$ch], $n);
		}
	}

	function dsrate($ch, $val){
		if($val < 0 || $val > 16777215){
			echo "Invalid DS rate ".$val;
			return 0;
		}

		parent::write24($this->adr_dsrate[$ch], $val);
		$this->rst();
	}

	function rst(){
		parent::uwrite($this->uadr_rst, 1);
	}


	function displayname($j, $namexml){
		if($j<self::NOFIN){
			$disp = $namexml->input[$j]->name;
		}else if($j<self::EOFDS){
			$n = $j - self::NOFIN;
			$div = $this->convdivide($this->rate[$n]);
			if($this->ds[$n]<20){
				$disp = $namexml->input[$this->ds[$n]]->name."/".$div;
			}else if($this->ds[$n]==20){
				$disp = "50M/".$div;
			}
		}else{
			switch($j){
			case self::NOF1K:
				$disp = '1k clock';
				break;
			case self::NOF10K:
				$disp = '10k clock';
				break;
			case self::NOFLEVEL:
				$disp = 'Level';
				break;
			case self::NOFVETO:
				$disp = 'Veto';
				break;
			default:
				$disp = '&nbsp;';
				break;
			}
			
		}
		return $disp;
	}

	// end of CSelGTO
}


?>
