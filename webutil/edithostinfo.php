<html>
<head>
<title>Edit Host Information</title>
<link rel='stylesheet' type='text/css' href='xutil.css'>
<script language='JavaScript'>
   function AsEBClick(){
   document.hform.halt.checked = false;
   document.hform.reboot.checked = false;
   document.hform.init_onoff.checked = false;
   document.hform.babinfo_onoff.checked = true;
   document.hform.babild_onoff.checked = true;
   document.hform.babiau_onoff.checked = true;
   document.hform.babies_onoff.checked = false;
   document.hform.babissm_onoff.checked = false;
   document.hform.babian_onoff.checked = false;
   document.hform.timereset_onoff.checked = false;
 }
   function AsFEClick(){
   document.hform.halt.checked = true;
   document.hform.reboot.checked = true;
   document.hform.init_onoff.checked = true;
   document.hform.babinfo_onoff.checked = false;
   document.hform.babild_onoff.checked = false;
   document.hform.babiau_onoff.checked = false;
   document.hform.babies_onoff.checked = true;
   document.hform.babissm_onoff.checked = false;
   document.hform.babian_onoff.checked = false;
   document.hform.timereset_onoff.checked = false;
 }
</script>
</head>
<body>

<?php

require 'babixlib.php';


function chkpost(&$dom, $key, $val){
  $chklist = array('name', 'efn', 'halt', 'reboot', 'description');

  $i=0;
  foreach($chklist as $okey){
    addcompost($dom, $chklist[$i], $okey, $key, $val);
    $i++;
  }
}


function chkopts(&$dom, $key, $val){
  $pslist = array('babinfo', 'babild', 'babiau', 'babies', 'babissm', 'babian', 'function', 'init');

  $i=0;
  $p = explode('_', $key);
  foreach($pslist as $ps){
    $item = 0;
    if(!strcmp($p[0], $ps)){
      $chkdom = $dom->getElementsByTagName($ps);
      if($chkdom->length){
	$item = $chkdom->item(0);
      }
      if(!$item){
	$item = $dom->appendChild($dom->ownerDocument->createElement($p[0]));
      }
      if(strlen($val)){
	$item->appendChild($dom->ownerDocument->createElement($p[1], $val));
#	echo "<b>$p[0] $ps $key $p[1] $val</b><br>\n";
      }
    }
  }
}

$thisfile = basename($_SERVER['SCRIPT_NAME']);
if(isset($_GET['host'])){
  $thishost = $_GET['host'];
 }else{
  $thishost = 0;
 }
if(!strcmp($thishost, 'new')){
  $new = 1;
 }else{
  $new = 0;
 }

$post = $_POST;

$save = 0; $del = 0;
$dom = new DomDocument('1.0');
$dom->formatOutput = true;
$domhost = $dom->appendChild($dom->createElement('host'));
while(list($key, $val) = each($post)){
  //echo "$key $val <br>\n";
  if(!strcmp($key, 'com')){
    if(!strcmp($val, 'save')){
      $save = 1;
    }
    if(!strcmp($val, 'delete')){
      $del = 1;
    }
  }
  chkpost($domhost, $key, $val);
  chkopts($domhost, $key, $val);
 }

echo "<h2>Make Host Information";
if($thishost){
  echo " (".$thishost.")";
  $edit = 1;
 }else{
  $edit = 0;
 }
echo "</h2>\n";

$xml = $dom->saveXML();
if($save){
  //printxml($xml);
  $chkdom = $domhost->getElementsByTagName('name');
  if(!$chkdom->length){
    echo "<font color='red'><b>Invalud host name</b></font><br>\n";
  }else{
    $tname = $chkdom->item(0)->nodeValue;
  }

  if(!strlen($tname)){
    echo "<font color='red'><b>Invalid host name</b></font><br>\n";
  }else{
    $wfile = "./xml/".$tname.".xml";
    //echo "wfile = $wfile<br>\n";
    $fp = fopen($wfile, "w");
    fwrite($fp, $xml);
    fclose($fp);
    chmod($wfile, 0666);
    echo "<font color='red'>Saved</font><br>\n";
  }
 }


echo "<a href='./index.php'>Back to index page</a>\n";
echo "<p>\n";
if($del){
  $wfile = "./xml/".$thishost.".xml";
  unlink($wfile);
  echo "<font color='red'>Delete $thishost</font>\n";
  $edit = 0;
  $thishost = '';
 }

/*
  $xdir = opendir('./xml');
  $flist = array();
  while($fname = readdir($xdir)){
  if(strcmp($fname, '.') && strcmp($fname, '..')){
  $tf = str_replace('.xml', '', $fname);
    array_push($flist, $tf);
    }
    }
    closedir($xdir);
*/
$flist = array();
$list = glob('./xml/*.xml');

foreach($list as $fn){
  $tf = str_replace('xml', '', $fn);
  $tf = str_replace('.', '', $tf);
  $tf = str_replace('//', '', $tf);
  array_push($flist, $tf);
}

sort($flist);
array_push($flist, 'new');

if($edit){
if(!$new){
  $fname = './xml/'.$thishost.'.xml';
  $hostinfo = simplexml_load_file($fname);
 }else{
  $hostinfo = 'host';
 }

 if(!isset($hostinfo->name))           $hostinfo->name = '';
 if(!isset($hostinfo->efn))            $hostinfo->efn  = '';
 if(!isset($hostinfo->description ))   $hostinfo->description = '';
 if(!isset($hostinfo->halt))           $hostinfo->halt = 'off';
 if(!isset($hostinfo->reboot))         $hostinfo->reboot = 'off';
 if(!isset($hostinfo->init->onoff))    $hostinfo->init->onoff = 'off';
 if(!isset($hostinfo->init->path))
   $hostinfo->init->path = '/home/daq/daqconfig/'.$thishost.'/init/daqinitrc';
 if(!isset($hostinfo->babinfo->onoff)) $hostinfo->babinfo->onoff = 'off';
 if(!isset($hostinfo->babild->onoff))  $hostinfo->babild->onoff = 'off';
 if(!isset($hostinfo->babild->option)) $hostinfo->babild->option = '-l';
 if(!isset($hostinfo->babiau->option)) $hostinfo->babiau->option = '-lt';
 if(!isset($hostinfo->babies->onoff))  $hostinfo->babies->onoff = 'off';
 if(!isset($hostinfo->babies->option)) $hostinfo->babies->option = '-l';
 if(!isset($hostinfo->babies->efn))    $hostinfo->babies->efn = '';
 if(!isset($hostinfo->babissm->onoff)) $hostinfo->babissm->onoff = 'off';
 if(!isset($hostinfo->babian->onoff))  $hostinfo->babian->onoff = 'off';
 if(!isset($hostinfo->function->timereset)) $hostinfo->function->timereset = '';
 if(!isset($hostinfo->function->timeresetpath))
   $hostinfo->function->timeresetpath = '/home/daq/daqconfig/'.$thishost.'/bin/resettimestamp';
 }

echo "<form name='hform' action='".$thisfile."?host=".$thishost."' method='post'>\n";
echo "<table>\n";
echo "<tr><td>\n";
echo "<table bgcolor='#ccccff'>\n";
foreach($flist as $l){
  echo "<tr>";
  if(!strcmp($thishost, $l)){
    $bgc = '#ccffcc';
  }else{
    $bgc = '#ffffff';
  }
  
  echo "<td bgcolor='$bgc'>";
  echo "<a class='editlink' href='".$thisfile."?host=".$l."'>".$l."<br>\n";
}
echo "</table>\n";
echo "<td>\n";
if($edit){
  echo "<table width=470 border=1>\n";
//
  echo "<tr bgcolor='#ccff33'><td width=130>Host";
  echo "<td width=130> <td width=130> <td width=80>\n";
  echo "<tr><td>Hostname<td colspan=2>".btxt('name', '40', 0, $hostinfo->name);
  echo "<tr><td>EFN<td>".btxt('efn', '3', 3, $hostinfo->efn)."<td>";
  echo "<tr><td>Description<td colspan=2>".btxt('description', '40', 0, $hostinfo->description);
  echo "<tr><td>".bchkbox('', 'halt', $hostinfo->halt);
  echo "<td>".bchkbox('', 'reboot', $hostinfo->reboot);
  echo "<td>".bchkbox_name('init', 'onoff', $hostinfo->init->onoff);
  //
  echo "<tr><td>Init path<td colspan=2>".btxt('init_path', '40', 0,
					      $hostinfo->init->path);
  echo "<tr bgcolor='#ffcc33'><td>Process<td>On/Off<td>Option<td>Sub-EFN\n";
  echo "<tr><td>babinfo<td>".bchkbox('babinfo', 'onoff', $hostinfo->babinfo->onoff)."<td>\n";
  echo "<tr><td>babild<td>".bchkbox('babild', 'onoff', $hostinfo->babild->onoff)."<td>".babildopt($hostinfo->babild->option)."\n";
  echo "<tr><td>babiau<td>".bchkbox('babiau', 'onoff', $hostinfo->babiau->onoff)."<td>".babiauopt($hostinfo->babiau->option)."\n";
  echo "<tr><td>babies<td>".bchkbox('babies', 'onoff', $hostinfo->babies->onoff)."<td>".babiesopt($hostinfo->babies->option)."\n";
  echo "<td>".btxt('babies_efn', '4', 0, $hostinfo->babies->efn)."\n";
  echo "<tr><td>babissm<td>".bchkbox('babissm', 'onoff', $hostinfo->babissm->onoff)."<td>\n";
  echo "<tr><td>babian<td>".bchkbox('babian', 'onoff', $hostinfo->babian->onoff)."<td>\n";
  //
  echo "<tr bgcolor='#ccccff'><td>Function<td>On/Off<td>Option\n";
  echo "<tr><td>Time Reset<td>".bchkbox('function', 'timereset', $hostinfo->function->timereset)."<td>\n";
  echo "<tr><td><td colspan=2>Time Reset Path<br>".btxt('function_timeresetpath', '40', 0, $hostinfo->function->timeresetpath);
  echo "<tr><td>";
  echo "<tr><td colspan=3><button type='submit' name='com' value='save'>Save</button>";
  echo "<button name='com' value='refresh' type='refresh'>Refresh</button>&nbsp; ";
  echo "<input type='button' name='aseb' value='As EB' onclick='AsEBClick()'>";
  echo "<input type='button' name='asfe' value='As FE' onclick='AsFEClick()'>";
  echo "<button name='com' value='delete' type='submit'>Delete</button>&nbsp; ";
  echo "</table>\n";
  echo "</table>\n";
  echo "</form>\n";
  echo "</p>\n";
 }else{
  echo "Please choose host name\n";
 }
?>
</body>
</html>
