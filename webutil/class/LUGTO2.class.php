<?php

require_once('SkelGTO.class.php');

class CLUGTO2 extends CGTO {
	public $out = array();
	public $ds = array();
	public $rate = array();

	const NOFOUT   = 8;
	const NOFPARAM = 60;
	const NOFIN    = 40;
	
	public $seladv = array (
  array("param"=>"in0",   "val"=>0x000000000001),
  array("param"=>"in1",   "val"=>0x000000000002),
  array("param"=>"in2",   "val"=>0x000000000004),
  array("param"=>"in3",   "val"=>0x000000000008),
  array("param"=>"in4",   "val"=>0x000000000010),
  array("param"=>"in5",   "val"=>0x000000000020),
  array("param"=>"in6",   "val"=>0x000000000040),
  array("param"=>"in7",   "val"=>0x000000000080),
  array("param"=>"in8",   "val"=>0x000000000100),
  array("param"=>"in9",   "val"=>0x000000000200),
  array("param"=>"in10",  "val"=>0x000000000400),
  array("param"=>"in11",  "val"=>0x000000000800),
  array("param"=>"in12",  "val"=>0x000000001000),
  array("param"=>"in13",  "val"=>0x000000002000),
  array("param"=>"in14",  "val"=>0x000000004000),
  array("param"=>"in15",  "val"=>0x000000008000),
  array("param"=>"in16",  "val"=>0x000000010000),
  array("param"=>"in17",  "val"=>0x000000020000),
  array("param"=>"in18",  "val"=>0x000000040000),
  array("param"=>"in19",  "val"=>0x000000080000),
  array("param"=>"not0",  "val"=>0x000000100000),
  array("param"=>"not1",  "val"=>0x000000200000),
  array("param"=>"not2",  "val"=>0x000000400000),
  array("param"=>"not3",  "val"=>0x000000800000),
  array("param"=>"not4",  "val"=>0x000001000000),
  array("param"=>"not5",  "val"=>0x000002000000),
  array("param"=>"not6",  "val"=>0x000004000000),
  array("param"=>"not7",  "val"=>0x000008000000),
  array("param"=>"not8",  "val"=>0x000010000000),
  array("param"=>"not9",  "val"=>0x000020000000),
  array("param"=>"not10", "val"=>0x000040000000),
  array("param"=>"not11", "val"=>0x000080000000),
  array("param"=>"not12", "val"=>0x000100000000),
  array("param"=>"not13", "val"=>0x000200000000),
  array("param"=>"not14", "val"=>0x000400000000),
  array("param"=>"not15", "val"=>0x000800000000),
  array("param"=>"not16", "val"=>0x001000000000),
  array("param"=>"not17", "val"=>0x002000000000),
  array("param"=>"not18", "val"=>0x004000000000),
  array("param"=>"not19", "val"=>0x008000000000),
  array("param"=>"sig",   "val"=>0x010000000000),
  array("param"=>"nsig",  "val"=>0x020000000000),
  array("param"=>"1k",    "val"=>0x030000000000),
  array("param"=>"10k",   "val"=>0x040000000000),
  array("param"=>"level", "val"=>0x000000000000),
  array("param"=>"pulse", "val"=>0x050000000000),
  array("param"=>"ds0" ,  "val"=>0x060000000000),
  array("param"=>"ds1" ,  "val"=>0x070000000000),
  array("param"=>"or"  ,  "val"=>0x100000000000),
  array("param"=>"and",   "val"=>0x200000000000),
  array("param"=>"none",  "val"=>0x400000000000),
  array("param"=>"veto",  "val"=>0x800000000000),
  array("param"=>"lo0",  "val"=>0),
  array("param"=>"lo1",  "val"=>1),
  array("param"=>"lo2",  "val"=>2),
  array("param"=>"lo3",  "val"=>3),
  array("param"=>"lo4",  "val"=>4),
  array("param"=>"lo5",  "val"=>5),
  array("param"=>"lo6",  "val"=>6),
  array("param"=>"lo7",  "val"=>7),
	array ("param"=>"", "val"=>-1)
	);

  public $luadv = array (
  array("param"=>"out0",   "val"=>0),
  array("param"=>"out1",   "val"=>1),
  array("param"=>"out2",   "val"=>2),
  array("param"=>"out3",   "val"=>3),
  array("param"=>"out4",   "val"=>4),
  array("param"=>"out5",   "val"=>5),
  array("param"=>"out6",   "val"=>6),
  array("param"=>"out7",   "val"=>7),
  array("param"=>"", "val"=>-1));


	public $adr_out = array(
		 0,  6,  12, 18, 24, 30, 36, 42);

	public $adr_dssel = array(54, 55);
	public $adr_dsrate = array(48, 51);

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
		$sdata = unpack("S*", $data);
		$cdata = unpack("C*", $data);
		for($i=0;$i<8;$i++){
			$tdata = ($sdata[$i*3+3] << 32) | ($sdata[$i*3+2] << 16) | ($sdata[$i*3+1]);
			$tdata = $tdata & 0x0000ffffffffffff;
			$this->out[$i] = $tdata;
			//printf("Decode %d = %016x<br>\n", $i, $this->out[$i]);
		}
		for($i=54;$i<56;$i++){
			$this->ds[$i-54] = $cdata[$i+1];
		}
		for($i=0;$i<2;$i++){
			$this->rate[$i]
				= ($cdata[1+50+$i*3] << 16) | ($cdata[1+49+$i*3] << 8) | $cdata[1+48+$i*3];
		}

	}

	function encout($chs){
		$ret = 0;

		foreach($chs as $nm){
			for($i=0;$i<self::NOFPARAM;$i++){
				if(!strcmp($nm, $this->seladv[$i]['param'])){
#					printf("%s : 0x%08x\n", $this->seladv[$i]['param'],
#								 $this->seladv[$i]['val']);
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

	function strluparam($chs){
		$ret = '';

		foreach($chs as $nm){
			for($i=0;$i<self::NOFOUT;$i++){
				if(!strcmp($nm, $this->luadv[$i]['param'])){
					$ret .= " ".$this->luadv[$i]['param'];
					break;
				}
			}
		}


		return $ret;
	}


	function selout($n, $chs){
		$val = $this->encout($chs);

		parent::write48($this->adr_out[$n], $val);
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

	function dssel($ch, $nm){
		$n = -1;
		for($i=0;$i<self::NOFPARAM;$i++){
			if(!strcmp($nm, $this->seladv[$i]['param'])){
				$n = $this->seladv[$i]['val'];
			}
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



	// end of CLUGTO2
}


?>
