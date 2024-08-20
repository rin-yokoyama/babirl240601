<?php

class CGTO {
	private $host = '';
	private $port = 10001;
	private $timeout = 2;
	private $fp = NULL;

	const com_read     = 0x40;
	const com_reset    = 0x41;
	const com_eepread  = 0x42;
	const com_eepwrite = 0x60;
	const com_flush    = 0xc0;
	const com_write    = 0x80;  // com_write | addr
	const com_mask     = 0xc0;
	const com_user     = 0x00;
	const com_unmask   = 0x3f;

	function __construct($host){
		$this->host = $host;
	}

  function __destruct(){
    unset($this->host);
  }

	function open(){
		if($this->fp) $this->close();
		$errno;
		$errstr;
		$ret = '';
		$this->fp = fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout);

		if(!$this->fp){
			$ret = "error:Can't connect to ".$this->host;
			return $ret;
		}
		socket_set_timeout($this->fp, $this->timeout);

		return $ret;
	}
	
	function close(){
		if($this->fp) fclose($this->fp);
		$this->fp = null;
		usleep(50000);
	}

	function com($com){
		//echo sprintf("com 0=%x(%d) 1=%x<br>\n",$com[0],$com[0], $com[1]);

		$ret = '';
		$data = pack("CC", $com[0], $com[1]);
		fputs($this->fp, $data, 2);
		$stat = socket_get_status($this->fp);
		if($stat["timed_out"]){
			$ret = "error:Time out";
		}
		
		return $ret;
	}

	function read(){
		$ret = '';
		if(!$this->fp){
			if(strlen($ret = $this->open())){
				return $ret;
			}
		}
		
		$com = array(self::com_read, 0);
		$this->com($com);
    $ret = stream_get_contents($this->fp, 64);
		return $ret;
	}

	function uread($addr){
		$ret = '';
		if(!$this->fp){
			if(strlen($ret = $this->open())){
				return $ret;
			}
		}
		
		$com = array(self::com_user | ($addr & self::com_unmask), 0);
		$this->com($com);
    $ret = stream_get_contents($this->fp, 64);
		return $ret;
	}

	function eepread(){
		$com = array(self::com_eepread, 0);
		$this->com($com);
		usleep(100000);
	}		

	function eepwrite(){
		$com = array(self::com_eepwrite, 0);
		$this->com($com);
		usleep(100000);
	}		

	function reset(){
		$com = array(self::com_reset, 0);
		$this->com($com);
	}		

	function vwrite($com, $addr, $val){
		$com = array(($com & self::com_mask)|($addr & self::com_unmask), $val);
		return $this->com($com);
	}

	function vwrite16($com, $addr, $val){
		for($i=0;$i<2;$i++){
			$cval = ($val>>($i*8)) & 0xff;
			$ret = $this->vwrite($com, $addr+$i, $cval);
			if(strlen($ret)) return $ret;
		}
		return '';
	}

	function vwrite24($com, $addr, $val){
		for($i=0;$i<3;$i++){
			$cval = ($val>>($i*8)) & 0xff;
			$ret = $this->vwrite($com, $addr+$i, $cval);
			if(strlen($ret)) return $ret;
		}
		return '';
	}

	function vwrite32($com, $addr, $val){
		for($i=0;$i<4;$i++){
			$cval = ($val>>($i*8)) & 0xff;
			$ret = $this->vwrite($com, $addr+$i, $cval);
			if(strlen($ret)) return $ret;
		}
		return '';
	}

	function vwrite48($com, $addr, $val){
		for($i=0;$i<6;$i++){
			$cval = ($val>>($i*8)) & 0xff;
			$ret = $this->vwrite($com, $addr+$i, $cval);
			if(strlen($ret)) return $ret;
		}
		return '';
	}

	function vwrite64($com, $addr, $val){
		for($i=0;$i<8;$i++){
			$cval = ($val>>($i*8)) & 0xff;
			$ret = $this->vwrite($com, $addr+$i, $cval);
			if(strlen($ret)) return $ret;
		}
		return '';
	}

	function write($addr, $val){
		return $this->vwrite(self::com_write, $addr, $val);
	}

	function write16($addr, $val){

		return $this->vwrite16(self::com_write, $addr, $val);
	}

	function write24($addr, $val){
		
		return $this->vwrite24(self::com_write, $addr, $val);
	}
	
	function write32($addr, $val){
		return $this->vwrite32(self::com_write, $addr, $val);
	}
	
	function write48($addr, $val){
		return $this->vwrite64(self::com_write, $addr, $val);
	}

	function write64($addr, $val){
		return $this->vwrite64(self::com_write, $addr, $val);
	}

	function uwrite($addr, $val){
		
		return $this->vwrite(self::com_user, $addr, $val);
	}
	
	function uwrite16($addr, $val){
		
		return $this->vwrite16(self::com_user, $addr, $val);
	}
	
	function uwrite24($addr, $val){
		
		return $this->vwrite24(self::com_user, $addr, $val);
	}
	
	function uwrite32($addr, $val){
		return $this->vwrite32(self::com_user, $addr, $val);
	}

	function uwrite48($addr, $val){
		return $this->vwrite48(self::com_user, $addr, $val);
	}

	function uwrite64($addr, $val){
		return $this->vwrite64(self::com_user, $addr, $val);
	}


// end of class
}


?>
