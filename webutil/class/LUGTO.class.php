<?php

require_once('SkelGTO.class.php');

class CLUGTO extends CGTO {
	public $out = array();
	public $ds = array();
	public $rate = array();

	const NOFOUT   = 8;
	const NOFPARAM = 49;
	const NOFIN    = 40;
	
	public $seladv = array (
  array("param"=>"in0",   "val"=>0x0000000000000001),
  array("param"=>"in1",   "val"=>0x0000000000000002),
  array("param"=>"in2",   "val"=>0x0000000000000004),
  array("param"=>"in3",   "val"=>0x0000000000000008),
  array("param"=>"in4",   "val"=>0x0000000000000010),
  array("param"=>"in5",   "val"=>0x0000000000000020),
  array("param"=>"in6",   "val"=>0x0000000000000040),
  array("param"=>"in7",   "val"=>0x0000000000000080),
  array("param"=>"in8",   "val"=>0x0000000000000100),
  array("param"=>"in9",   "val"=>0x0000000000000200),
  array("param"=>"in10",  "val"=>0x0000000000000400),
  array("param"=>"in11",  "val"=>0x0000000000000800),
  array("param"=>"in12",  "val"=>0x0000000000001000),
  array("param"=>"in13",  "val"=>0x0000000000002000),
  array("param"=>"in14",  "val"=>0x0000000000004000),
  array("param"=>"in15",  "val"=>0x0000000000008000),
  array("param"=>"in16",  "val"=>0x0000000000010000),
  array("param"=>"in17",  "val"=>0x0000000000020000),
  array("param"=>"in18",  "val"=>0x0000000000040000),
  array("param"=>"in19",  "val"=>0x0000000000080000),
  array("param"=>"not0",  "val"=>0x0000000000100000),
  array("param"=>"not1",  "val"=>0x0000000000200000),
  array("param"=>"not2",  "val"=>0x0000000000400000),
  array("param"=>"not3",  "val"=>0x0000000000800000),
  array("param"=>"not4",  "val"=>0x0000000001000000),
  array("param"=>"not5",  "val"=>0x0000000002000000),
  array("param"=>"not6",  "val"=>0x0000000004000000),
  array("param"=>"not7",  "val"=>0x0000000008000000),
  array("param"=>"not8",  "val"=>0x0000000010000000),
  array("param"=>"not9",  "val"=>0x0000000020000000),
  array("param"=>"not10", "val"=>0x0000000040000000),
  array("param"=>"not11", "val"=>0x0000000080000000),
  array("param"=>"not12", "val"=>0x0000000100000000),
  array("param"=>"not13", "val"=>0x0000000200000000),
  array("param"=>"not14", "val"=>0x0000000400000000),
  array("param"=>"not15", "val"=>0x0000000800000000),
  array("param"=>"not16", "val"=>0x0000001000000000),
  array("param"=>"not17", "val"=>0x0000002000000000),
  array("param"=>"not18", "val"=>0x0000004000000000),
  array("param"=>"not19", "val"=>0x0000008000000000),
  array("param"=>"sig",   "val"=>0x0000010000000000),
  array("param"=>"nsig",  "val"=>0x0000020000000000),
  array("param"=>"1k",    "val"=>0x0000040000000000),
  array("param"=>"10k",   "val"=>0x0000080000000000),
  array("param"=>"level", "val"=>0x0000100000000000),
  array("param"=>"pulse", "val"=>0x0000200000000000),
  array("param"=>"or"  ,  "val"=>0x0001000000000000),
  array("param"=>"and",   "val"=>0x0002000000000000),
  array("param"=>"veto",  "val"=>0x8000000000000000),
	array ("param"=>"", "val"=>-1)
	);

	public $adr_out = array(
		 0,  8,  16, 24, 32, 40, 48, 56);

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
		$lldata = unpack("L*", $data);
		$cdata = unpack("C*", $data);
		for($i=0;$i<8;$i++){
			$this->out[$i] = ($lldata[$i*2+2] << 32) | ($lldata[$i*2+1]);
			//printf("Decode %d = %016x<br>\n", $i, $this->out[$i]);
		}
	}

	function encout($chs){
		$ret = 0;

		foreach($chs as $nm){
			for($i=0;$i<self::NOFPARAM;$i++){
				if(!strcmp($nm, $this->seladv[$i]['param'])){
					$ret |= $this->seladv[$i]['val'];
				}
			}
		}

		return $ret;
	}

	function strparam($chs){
		$ret = '';

		foreach($chs as $nm){
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
		parent::write64($this->adr_out[$n], $val);
	}

	function chkout($o, $in){
		return $this->out[$o] & $this->seladv[$in]['val'];
	}

	function chkdata($d, $in){
		return $d & $this->seladv[$in]['val'];
	}

	function dataparam($n){
		$ret = array();
		for($i=0;$i<self::NOFIN;$i++){
			if($this->chkout($n, $i)){
				array_push($ret, $this->seladv[$i]['param']);
			}else{
			}
		}
		return $ret;
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



	// end of CLUGTO
}


?>
