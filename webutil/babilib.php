<?php

define('MON_GETHDST',        11);
define('MON_CHKPID',         12);
define('MON_KILLPID',        13);
define('MON_EXEC',           14);
define('MON_PEXEC',          15);
define('MON_BABIEXEC',       16);
define('MON_COPYFILE',       17);
define('MON_CHKBABIES',      18);
define('MON_GETHDST_STR',   101);
define('MON_CHKPID_STR',    102);
define('BABILD_PORT',     17511);
define('EB_GET_DAQINFO',      1);
define('EB_GET_RUNINFO',      3);
define('EB_GET_EFNUM',        5);
define('EB_GET_EFLIST',       6);
define('EB_GET_MTLIST',      13);
define('EB_GET_HDLIST',      17);
define('EB_GET_SSMINFO',     51);
define('STAT_RUN_IDLE',       0);
define('STAT_RUN_START',      1);
define('STAT_RUN_NSSTA',      2);
define('STAT_RUN_WAITSTOP',   3);

define('INF_GET_SCRLIST',   301);
define('INF_SET_SCRNAME',   302);
define('INF_GET_SCRDATA',   303);
define('INF_GET_SCRLIVE',   304);
define('INF_GET_RAWDATA',    10);
define('INF_GET_BLOCKNUM',   11);

#define('SIZEOF_DAQINFO',  44320);
define('SIZEOF_DAQINFO',     96);
define('SIZEOF_RUNINFO',    176);
define('SIZEOF_HDLIST',     112);
define('SIZEOF_EFLIST',     168);
define('MON_MAX_BUFF',   102400);

define('MAXSCRANA',          32);

$ofstr  = array('off', 'on', 'scr');
$runstr = array('IDLE', 'START', 'NSSTA');
$erstr = array('Error', 'OK');

$babimoport  = 17518;
$babildport  = 17511;
$babinfoport = 17516;
$timeout    = 5;

function settimeout($t){
  global $timeout;

  $timeout = $t;
}

function gethdlist($server){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br>\n";
    $hdlist = NULL;
  }else{
    socket_set_timeout($fp, $timeout);
    $len = pack("i", 4);   // Length = 4
    fputs($fp, $len);
    
    $com = pack("i", MON_GETHDST_STR); // Command
    fputs($fp, $com);
    
    $i=0;
    while($data = fscanf($fp, "%s %s %s %s %s\n")){
      list($dev, $path, $tot, $used, $free) = $data;
      
      // Convert to KB to GB
      $tot  /= 1024*1024;
      $used /= 1024*1024;
      $free /= 1024*1024;
      $hdlist[$i] = array($dev, $path, $tot, $used, $free);
      $i++;
    }

    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $hdlist;
}

function getpid($server, $name){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($name);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);
    
    $com = pack("i", MON_CHKPID_STR); // Command
    fputs($fp, $com);
    fputs($fp, $name);
    
    $pid = fgets($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $pid;
}

function killpid($server, $name){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($name);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);
    
    $com = pack("i", MON_KILLPID); // Command
    fputs($fp, $com);
    fputs($fp, $name);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);

    sleep(1);
  }
  return 0;
}

function execarg($server, $arg){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return 0;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($arg);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_EXEC); // Command
    fputs($fp, $com);
    fputs($fp, $arg);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return 1;
}

function execargr($server, $arg){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  $rstr = '';
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return -1;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($arg);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_PEXEC); // Command
    fputs($fp, $com);
    fputs($fp, $arg);

    $len = stream_get_contents($fp, 4);
    $rstr = stream_get_contents($fp);

    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $rstr;
}

function execproc($server, $name, $arg){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $line = "/usr/babirl/$name/$name $arg";
    $slen = 4 + strlen($line);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_EXEC); // Command
    fputs($fp, $com);
    fputs($fp, $line);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return 1;
}

function execbabiproc($server, $name, $arg){
  global $babimoport, $timeout;

  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $line = "$name $arg";
    $slen = 4 + strlen($line);
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_BABIEXEC); // Command
    fputs($fp, $com);
    fputs($fp, $line);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return 1;
}

function babildxcom($server, $xml){
  global $babildport, $timeout;
  
  $fp = fsockopen($server, $babildport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = strlen($xml);
    $len = pack("i", $slen); 
    fputs($fp, $len);
    fputs($fp, $xml);
    
    $len = stream_get_contents($fp, 4);
    $rxml = stream_get_contents($fp);
    if(!feof($fp)){
      $rxml .= stream_get_contents($fp);
    }

    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $rxml;
}

function babinfoxcom($server, $xml){
  global $babinfoport, $timeout;

  $fp = fsockopen($server, $babinfoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = strlen($xml);
    $len = pack("i", $slen); 
    fputs($fp, $len);
    fputs($fp, $xml);
    
    $try = 0;
    $len = stream_get_contents($fp, 4);
    while(!$len){
      $len = stream_get_contents($fp, 4);
      $try ++;
      if($try > 10) break;
      if(!$len) sleep(1);
    }

    $rxml = stream_get_contents($fp);
    $try = 0;
    while(!feof($fp)){
      $rxml .= stream_get_contents($fp);
      $try ++;
      if($try > 40) break;
      if(!feof($fp)) sleep(1);
    }
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $rxml;
}

function getdaqinfo($server){
  global $babildport, $timeout;
  
  $fp = fsockopen($server, $babildport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", EB_GET_DAQINFO); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $daqinfo = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $daqinfo;
}

function getruninfo($server){
  global $babildport, $timeout;
  
  $fp = fsockopen($server, $babildport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", EB_GET_RUNINFO); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $runinfo = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $runinfo;
}

function getssminfo($server){
  global $babildport, $timeout;
  
  $fp = fsockopen($server, $babildport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", EB_GET_SSMINFO); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $ssminfo = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $ssminfo;
}

function getscrlist($server){
  global $babinfoport, $timeout;
  
  $fp = fsockopen($server, $babinfoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", INF_GET_SCRLIST); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $scrlist = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $scrlist;
}

function getscrlive($server){
  global $babinfoport, $timeout;
  
  $fp = fsockopen($server, $babinfoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", INF_GET_SCRLIVE); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $scrlive = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $scrlive;
}

function getscrdata($server, $sid){
  global $babinfoport, $timeout;
  
  $fp = fsockopen($server, $babinfoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 8;
    $len = pack("i", $slen);   // Length = 8
    fputs($fp, $len);

    $com = pack("i", INF_GET_SCRDATA); // Command
    fputs($fp, $com);

    $id = pack("i", $sid); // SCR ID
    fputs($fp, $id);
    
    $len = stream_get_contents($fp, 4);
    $scrdata = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $scrdata;
}

function moncopytext($server, $name, $arg){
  global $babimoport, $timeout;

  $ret = 0;
  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($arg) + strlen($name) + 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_COPYFILE); // Command
    fputs($fp, $com);
    fputs($fp, $name);
    $zr = pack("i", 0);  // Insert '0' between filename and contents
    fputs($fp, $zr);
    fputs($fp, $arg);

    $ret = stream_get_contents($fp, 4);

    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $ret;
}

function moncopyfile($server, $lpath, $rpath){
  global $babimoport, $timeout;

	$fsz = filesize($lpath);
	if($fsz <= 0){
		// no file
		echo 'Cannot find file '.$lpath."<br>\n";
		return -1;
	}
	if($fsz >= 99*1024){
		// too large size
		echo 'Too large file '.$lpath." (".$fsz." bytes)<br>\n";
		return -2;
	}

	$arg = file_get_contents($lpath);
	$name = $rpath;


  $ret = 0;
  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4 + strlen($arg) + strlen($name) + 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_COPYFILE); // Command
    fputs($fp, $com);
    fputs($fp, $name);
    $zr = pack("i", 0);  // Insert '0' between filename and contents
    fputs($fp, $zr);
    fputs($fp, $arg);

    $ret = stream_get_contents($fp, 4);

    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }

  return $ret;
}

function monchkbabies($server){
  global $babimoport, $timeout;

  $ret = 0;
  $ar = array();
  $ar[1] = -1;
  $fp = fsockopen($server, $babimoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", MON_CHKBABIES); // Command
    fputs($fp, $com);


    $len = stream_get_contents($fp, 4);
    $ret = stream_get_contents($fp);
    $ar = unpack("l", $ret);
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $ar[1];
}

/*
  function getblknum($server){
  global $babinfoport, $timeout;
  
  $fp = fsockopen($server, $babinfoport, $errno, $errstr, $timeout);
  if(!$fp) {
    echo "Can't connect to $server<br><br>\n";
    return;
  }else{
    socket_set_timeout($fp, $timeout);
    $slen = 4;
    $len = pack("i", $slen);   // Length = 4
    fputs($fp, $len);

    $com = pack("i", INF_GET_BLOCKNUM); // Command
    fputs($fp, $com);
    
    $len = stream_get_contents($fp, 4);
    $blknum = stream_get_contents($fp);
    
    $stat = socket_get_status($fp);
    if($stat["timed_out"]) { echo "timeout<br>"; }
    fclose($fp);
  }
  return $blknum;
}
*/



function fteflist($i){
  $ft = "iefex$i/iefof$i/a80efname$i/a80efhost$i";

  return $ft;
}

function fthdlist($i){
  $ft = "ihdex$i/ihdof$i/a80hdpath$i/ihdfreea$i/ihdfreeb$i/ihdfulla$i/ihdfullb$i/ihdmaxa$i/ihdmaxb$i";

  return $ft;
}

function ftdaqinfo(){
  $ft = "a80runname/irunnumber/iebsize/iefn/ibabildes";

  return $ft;
}

function ftruninfo(){
  $ft = "irunnumber/irunstat/istarttime/istoptime/a80header/a80ender";

  return $ft;
}

function ftssminfo(){
  $ft = "iex/iof/a80host/a80start/a80stop";

  return $ft;
}

function ftscrlive(){
  $ft = "igatedid/igatedch/iungatedid/iungatedch";

  return $ft;
}

function ftscrlist($i){
  $ft = "iclassid$i/iscrid$i/iratech$i/irate$i/iupdate$i/iscrn$i/a80idname$i";

  return $ft;
}

function ftscrcont($i){
  $ft = "Icur$i/Idm$i/Itota$i/Itotb$i/a80name$i";

  return $ft;
}

function disdefon($of){
  if(!strcmp($of, "off")){
    return "disabled";
  }else{
    return "";
  }
}

function disdefoff($of){
  if(!strcmp($of, "on")){
    return "";
  }else{
    return "disabled";
  }
}

function chksel($opt, $sel){
  if(!strcmp($opt, $sel)){
    return "selected";
  }else{
    return "";
  }
}

function prisel($a, $b, $c){
  if($a){
    return $a;
  }else if($b){
    return $b;
  }else{
    return $c;
  }
}

function fmrunbt($ds, $server, $name){
  return "<button $ds type='submit' name='$server=$name' value='RUN'>RUN</button>";
}

function fmcombt($ds, $server, $name, $com){
  return "<button $ds type='submit' name='$server=$com' value='$com'>$com</button>";
}

function fmcombtcol($ds, $server, $name, $com, $col){
  return "<button $ds type='submit' name='$server=$com' value='$com'><font color=$col>$com</font></button>";
}


function fmtxt($server, $desc, $val){
  return "<input type='text' name='$server=$desc' value='$val' size='2'>";
}


function execcmdvme($server, $arg){
  $r = execargr($server, $arg);

  if($r == -1){
    return -1;
  }

  $ar = explode("\n", $r);
  $ar = array_map('trim', $ar);
  $ar = array_filter($ar, 'strlen');
  $ar = array_values($ar);
  $ar = preg_replace("/\s/","",$ar);

  $ret = array();

  foreach($ar as $a){
    if(!preg_match('/\//', $a)){
      $x = '0x'.$a;
      $d = intval($x, 0);
      $xar = array($x, $d);
      array_push($ret, $xar);
    }else{
      array_push($ret, explode("/", $a));
    }
  }
  return $ret;
}

function chkbabiesblk($server){
  $arg = '/bin/ls /tmp/babiesblk';
  $rarg = execargr($server, $arg);

  if(preg_match('/babiesblk/', $rarg)){
    return 1;
  }else{
    return 0;
  }
}

function waitbabiesblk($server){
  sleep(1);
  for($lp=0;$lp<20;$lp++){
    if(chkbabiesblk($server)){
      sleep(1);
    }else{
      $lp = 1000;
    }
  }

  if($lp < 1000){
    return 0;
  }

  return 1;
}

?>


