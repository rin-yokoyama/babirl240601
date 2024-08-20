<?php

class Cbabinfo {
  const INF_GET_SCRLIST  =  301;
  const INF_SET_SCRNAME  =  302;
  const INF_GET_SCRDATA  =  303;
  const INF_GET_SCRLIVE  =  304;
  const INF_GET_RAWDATA  =  10;
  const INF_GET_BLOCKNUM =  11;

  private $babinfoport = 17516;
  private $timeout = 5;
  private $server = '';
  private $fp = null;

  function __construct($server){
    $this->server = $server;
  }

  function __destruct(){
    unset($this->server);
  }

  function opensock(){
    $this->fp = fsockopen($this->server, $this->babinfoport, $errno, $errstr,
		    $this->timeout);
    if(!$this->fp){
      $this->fp = null;
      return 0;
    }
    socket_set_timeout($this->fp, $this->timeout);
    return 1;
  }

  function closesock(){
    if($this->fp){
      fclose($this->fp);
    }
  }

  function inf_get($com){
    if(!$this->opensock()){
      return -1;
    }

    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($this->fp, $len);

    $com = pack("i", $com); // Command
    fputs($this->fp, $com);
    
    $len = stream_get_contents($this->fp, 4);
    $list = stream_get_contents($this->fp);
    $stat = socket_get_status($this->fp);

    $this->closesock();

    if($stat["timed_out"]) {
      return -1;
    }

    return $list;
  }

  // return integer
  function get_blknum(){
    $list = $this->inf_get(self::INF_GET_BLOCKNUM);
    $n = array_merge(unpack("i", $list));
    
    return $n[0];
  }

  // return string buffer
  function get_rawdata(){
    $list = $this->inf_get(self::INF_GET_RAWDATA);

    return $list;
  }
}

?>
