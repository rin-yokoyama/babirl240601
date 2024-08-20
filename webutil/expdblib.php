<?php
require "babilib.php";
require "babiformlib.php";

$expdbcom = '/usr/babirl/dbaccess/expdbcom';

function cmpnstr($chk, $val){
  if(!strncmp($chk, $val, strlen($chk))){
    return 1;
  }else{
    return 0;
  }
}

function parse_csv($line) {
  preg_match_all('/("[^"]*(?:""[^"]*)*"|[^,]*),?/', $line, $a);
  foreach($a[1] as $key => $value) {
    if(preg_match('/^"(.*)"$/', $value, $value2)) {
      $a[1][$key] = preg_replace('/""/', '"', $value2[1]);
    }
  }
  return $a[1];
}

class Explist {
  public $param = array('ExpID'      => '',
			'Name'       => '',
			'StartDate'  => '',
			'StopDate'   => '',
			'Comment'    => '',
			'GatedID'    => '',
			'GatedCh'    => '',
			'UngatedID'  => '',
			'UngatedCh'  => '');

  public function init($val){
    $ar = explode(',', $val);
    $n = count($this->param);
    $keys = array_keys($this->param);
    for($i=0;$i<$n;$i++){
      $this->param[$keys[$i]] = $ar[$i];
    }
  }

  public function tablehead(){
    $html = "";
    $keys = array_keys($this->param);
    foreach($keys as $k){
      $html .= "<td>".$k;
    }
    return $html;
  }

  public function tableeditall($id){
    $keys = array_keys($this->param);
    $html = "<td>".$this->param['ExpID'];
    $html .= hidden_id('ExpID', $this->param['ExpID'], $id);
    for($i=1;$i<=4;$i++){
      $html .= text_tdid($keys[$i], $this->param[$keys[$i]], $id, 0);
    }
    for($i=5;$i<=8;$i++){
      $html .= text_tdid($keys[$i], $this->param[$keys[$i]], $id, 1);
    }

    return $html;
  }

  public function tableeditscaler($id){
    $keys = array_keys($this->param);
    $html = "<td>".$this->param['ExpID'];
    $html .= hidden_id('ExpID', $this->param['ExpID'], $id);
    for($i=1;$i<=4;$i++){
      $html .= "<td>".$this->param[$keys[$i]];
    }
    for($i=5;$i<=8;$i++){
      $html .= text_tdid($keys[$i], $this->param[$keys[$i]], $id, 1);
    }
    
    return $html;
  }
};

class Scalerlist {
  public $param = array('ScalerID'     => '',
			'Name'         => '',
			'Type'         => '',
			'ScalerType'   => '',
			'Rate'         => '',
			'RateCh'       => '');

  public function init($val){
    $ar = explode(',', $val);
    $n = count($this->param);
    $keys = array_keys($this->param);
    for($i=0;$i<$n;$i++){
      $this->param[$keys[$i]] = $ar[$i];
    }
  }

  public function tablehead(){
    $html = "";
    $keys = array_keys($this->param);
    $n = count($keys);
    foreach($keys as $k){
      if(strcmp($k, 'Type')){
	$html .= "<td>".$k;
      }
    }
    return $html;
  }

  public function tableeditscaler($id){
    $keys = array_keys($this->param);
    $html = "<td>".$this->param['ScalerID'];
    $html .= hidden_id('ScalerID', $this->param['ScalerID'], $id);
    $html .= text_tdid($keys[1], $this->param[$keys[1]], $id, 0);
    $html .= "<td>".$this->param[$keys[3]];

    for($i=4;$i<=5;$i++){
      $html .= text_tdid($keys[$i], $this->param[$keys[$i]], $id, 1);
    }
    
    return $html;
  }
};

class Channellist {
  public $param = array('Channel'     => '',
			'Name'        => '');

  public function init($val){
    $ar = explode(',', $val);
    $n = count($this->param);
    $keys = array_keys($this->param);
    for($i=0;$i<$n;$i++){
      $this->param[$keys[$i]] = $ar[$i];
    }
  }

  public function tablehead(){
    $html = "";
    $keys = array_keys($this->param);
    $n = count($keys);
    foreach($keys as $k){
      $html .= "<td>".$k;
    }
    return $html;
  }

  public function tableeditchannel($id){
    $keys = array_keys($this->param);
    $html = "<td>".$this->param['Channel'];
    $html .= hidden_id('Channel', $this->param['Channel'], $id);
    $html .= text_tdid('Name', $this->param['Name'], $id, 0);
    
    return $html;
  }

  public function tabledisablechannel($id){
    $keys = array_keys($this->param);
    $html = "<td>".$this->param['Channel'];
    $html .= hidden_id('Channel', $this->param['Channel'], $id);
    $html .= text_disable_tdid('Name', $this->param['Name'], $id, 0);
    
    return $html;
  }
};


class Runlist {
  public $param = array('RunID'      => '',
			'Name'       => '',
			'Number'     => '',
			'StartDate'  => '',
			'StopDate'   => '',
			'Header'     => '',
			'Ender'      => '',
			'Status'     => '');

  public function init($val){
    $ar = parse_csv($val);
    //$ar = explode(',', $val);
    $n = count($this->param);
    $arn = count($ar);

    //if($n != $arn) return;

    $keys = array_keys($this->param);
    for($i=0;$i<$n;$i++){
      $this->param[$keys[$i]] = $ar[$i];
    }
  }

  public function tablehead(){
    $html = "";
    $keys = array_keys($this->param);
    $n = count($keys);
    foreach($keys as $k){
      if(strcmp($k, 'RunID')){
	$html .= "<td>".$k;
      }
    }
    return $html;
  }

  public function tableshowrun(){
    $keys = array_keys($this->param);
    $html = "";
    for($i=1;$i<=7;$i++){
      $html .= "<td>".$this->param[$keys[$i]];
    }
    
    return $html;
  }
};

function doexpdbcom($server, $opt){
  global $expdbcom;
  $arg = $expdbcom." ".$opt;
  return execargr($server, $arg);
}

function getrunlist($server, $expid, $desc){
  $list = array();
  $i = 0;
  $opt = "--List --Run";
  $opt .= " --expid ".$expid;
  if($desc){
    $opt .= " --desc";
  }

  $ret = doexpdbcom($server, $opt);
  $ar = explode("\n", $ret);
  foreach ($ar as $line){
    if(!strlen($line)) break;
    $list[$i] = new Runlist;
    $list[$i]->init($line);
    $i++;
  }

  return $list;
}

function getexplist($server, $expid){
  $list = array();
  $i = 0;
  $opt = "--List --Exp";
  if($expid > 0){
    $opt .= " --expid ".$expid;
  }
  $ret = doexpdbcom($server, $opt);
  $ar = explode("\n", $ret);
  foreach ($ar as $line){
    if(!strlen($line)) break;
    $list[$i] = new Explist;
    $list[$i]->init($line);
    $i++;
  }

  return $list;
}

function getscalerlist($server, $expid){
  $list = array();
  $i = 0;
  $opt = "--List --Scaler";
  $opt .= " --expid ".$expid;

  $ret = doexpdbcom($server, $opt);
  $ar = explode("\n", $ret);
  foreach ($ar as $line){
    if(!strlen($line)) break;
    $list[$i] = new Scalerlist;
    $list[$i]->init($line);
    $i++;
  }

  return $list;
}

function getchannellist($server, $expid, $scrid){
  $list = array();
  $i = 0;
  $opt = "--List --Channel";
  $opt .= " --expid ".$expid;
  $opt .= " --scalerid ".$scrid;

  $ret = doexpdbcom($server, $opt);
  $ar = explode("\n", $ret);
  foreach ($ar as $line){
    if(!strlen($line)) break;
    $list[$i] = new Channellist;
    $list[$i]->init($line);
    $i++;
  }

  return $list;
}

function addopt($lopt, $val){
  $ret = '';
  if(strlen($val)){
    $ret = " --".$lopt." '".$val."'";
  }
  return $ret;
}

function updateexplist($server, $exp, $psd){
  if(!strlen($exp->param['ExpID'])){
    $opt = " --Insert --Exp";
  }else{
    $opt = " --Update --Exp";
    $opt .= addopt('expid', $exp->param['ExpID']);
  }
  $opt .= addopt('expname', $exp->param['Name']);
  $exp->param['StartDate'] = safedate($exp->param['StartDate']);
  $opt .= addopt('startdate', $exp->param['StartDate']);
  $exp->param['StopDate'] = safedate($exp->param['StopDate']);
  $opt .= addopt('stopdate', $exp->param['StopDate']);
  $opt .= addopt('comment', $exp->param['Comment']);
  $opt .= addopt('gatedid', $exp->param['GatedID']);
  $opt .= addopt('gatedch', $exp->param['GatedCh']);
  $opt .= addopt('ungatedid', $exp->param['UngatedID']);
  $opt .= addopt('ungatedch', $exp->param['UngatedCh']);
  $opt .= addopt('password', $psd);

  $ret = doexpdbcom($server, $opt);

  return $ret;
}

function updatescrlist($server, $expid, $scr, $psd){

  $opt = " --Update --Scaler";
  $opt .= addopt('expid', $expid);
  $opt .= addopt('scalerid', $scr->param['ScalerID']);
  $opt .= addopt('scalername', $scr->param['Name']);
  $opt .= addopt('rate', $scr->param['Rate']);
  $opt .= addopt('ratech', $scr->param['RateCh']);
  $opt .= addopt('password', $psd);

  $ret = doexpdbcom($server, $opt);

  return $ret;
}

function deletescrinfo($server, $expid, $scrid, $psd){

  $opt = " --Update --Scaler";
  $opt .= addopt('expid', $expid);
  $opt .= addopt('scalerid', $scrid);
  $opt .= addopt('comment', 'dropthisscaler');
  $opt .= addopt('password', $psd);

  $ret = doexpdbcom($server, $opt);

  return $ret;
}


function updatechannellist($server, $expid, $scrid, $ch, $psd){

  $opt = " --Update --Channel";
  $opt .= addopt('expid', $expid);
  $opt .= addopt('scalerid', $scrid);
  $opt .= addopt('channelnumber', $ch->param['Channel']);
  $opt .= addopt('channelname', $ch->param['Name']);
  $opt .= addopt('password', $psd);

  $ret = doexpdbcom($server, $opt);

  return $ret;
}

function getexp_post($post, $x){
  $ret = array();
  $ret = new Explist;
  $keys = array_keys($ret->param);

  foreach($keys as $k){
    $chk = $k.$x;
    if(isset($post[$chk])){
      $ret->param[$k] = $post[$chk];
    }
  }

  return $ret;
}

function getscr_post($post, $x){
  $ret = array();
  $ret = new Scalerlist;
  $keys = array_keys($ret->param);

  foreach($keys as $k){
    $chk = $k.$x;
    if(isset($post[$chk])){
      $ret->param[$k] = $post[$chk];
    }
  }

  return $ret;
}

function getchannel_post($post, $x){
  $ret = array();
  $ret = new Channellist;
  $keys = array_keys($ret->param);

  foreach($keys as $k){
    $chk = $k.$x;
    if(isset($post[$chk])){
      $ret->param[$k] = $post[$chk];
    }
  }

  return $ret;
}

function safedate($a){
  if(!strlen($a)){
    return "";
  }

  $t = explode(' ',$a,2);

  if(isset($t[0])){
    $x = explode('-', $t[0], 3);
    if(isset($x[0])){
      $y = $x[0];
    }else{
      $y = 2013;
    }
    if(isset($x[1])){
      $m = $x[1];
    }else{
      $m = 1;
    }
    if(isset($x[2])){
      $d = $x[2];
    }else{
      $d = 1;
    }
  }else{
    $y = 2013;
    $m = 1;
    $d = 1;
  }
  
  if(isset($t[1])){
    $v = explode(':', $t[1], 3);
    if(isset($v[0])){
      $H = $v[0];
    }else{
      $H = 0;
    }
    if(isset($v[1])){
      $M = $v[1];
    }else{
      $M = 0;
    }
    if(isset($v[2])){
      $S = $v[2];
    }else{
      $S = 0;
    }
  }else{
    $H = 0;
    $M = 0;
    $S = 0;
  }
  
  if($y > 2100 || $y < 2000){
    $y = 2013;
  }
  if($m > 12 || $m < 1){
    $m = 1;
  }
  if($d > 31 || $d < 1){
    $d = 1;
  }
  if($H > 23 || $H < 0){
    $H = 0;
  }
  if($M > 59 || $M < 0){
    $M = 0;
  }
  if($S > 59 || $S < 0){
    $S = 0;
  }
  
  $ret = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $y, $m, $d, $H, $M, $S);

  return $ret;
}

?>
