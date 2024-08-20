<html>
<head>
<title>babirl web configurator</title>
</head>
<body>
<h1>babirl web configurator</h1>

<a href="./index.php">Back to index page</a>

<?php
require "babilib.php";
require "babixlib.php";

$thisfile = basename($_SERVER['SCRIPT_NAME']);

$arserver = array();

$thisfile = basename($_SERVER['SCRIPT_NAME']);
$file = "dat/hostlist.xml";

if(!($hostlist = simplexml_load_file($file))){
  die();
 }

$thishost = $_GET["host"];
#$thishost = 'sdaq02';

if(!$thishost){
  echo "no host name<br>\n";
  die();
 }

echo date("Y/m/d H:i:s");

echo "<form action='".$thisfile."?host=".$thishost."' method='post' autocomplete='off' >";

$post = $_POST;

$fsave = 0;
$efmod = array();
$efhost = array();
$efname = array();
$efefn = array();
$efof = array();
$efnum = array();
$efex = array();

$hdhdn = array();
$hdpath = array();
$hdof = array();

$esebhost = array();
$esrtdrv = array();
$esreload = array();
$srunname = array();
$srunnumber = array();

while(list($key, $val) = each($post)){
  if(!strcmp($key, "save")){
    $fsave = 1;
  }
  $n = ereg_replace("[a-zA-Z]", "", $key);
  if(!strncmp($key, "efn", 3)){
    $efefn[(int)$n] = $val;
    $efex[(int)$n] = 1;
  }
  if(!strncmp($key, "host", 4)){
    $efhost[(int)$n] = $val;
  }
  if(!strncmp($key, "name", 4)){
    $efname[(int)$n] = $val;
  }
  if(!strncmp($key, "of", 2)){
    $efof[(int)$n] = $val;
  }
  if(!strncmp($key, "mod", 3)){
    $efmod[(int)$n] = (int)$val;
    $efnum[(int)$n] = (int)$n;
  }
  if(!strncmp($key, "hdhdn", 5)){
    $hdhdn[(int)$n] = $val;
  }
  if(!strncmp($key, "hdpath", 6)){
    $hdpath[(int)$n] = $val;
  }
  if(!strncmp($key, "hdof", 4)){
    $hdof[(int)$n] = $val;
  }
  if(!strncmp($key, "reload", 6)){
    $esreload[(int)$n] = $val;
  }
  if(!strncmp($key, "rtdrv", 5)){
    $esrtdrv[(int)$n] = $val;
  }
  if(!strncmp($key, "ebhost", 6)){
    $esebhost[(int)$n] = $val;
  }
  if(!strncmp($key, "esefn", 5)){
    $esefn[(int)$n] = $val;
  }
  if(!strncmp($key, "runname", 7)){
    $srunname[(int)$n] = $val;
  }
  if(!strncmp($key, "runnumber", 9)){
    $srunnumber[(int)$n] = $val;
  }
 }

# Find selected server from host
foreach($hostlist->host as $t){
  if(!strcmp($t->name, $thishost)){
    $srv = $t;
  }
}

echo "<hr width=500 align=left>\n";

$server = $thishost;
# for status line
if($fsave){

# get eflist
  $dom = new DomDocument('1.0');
  $xcom = $dom->appendChild($dom->createElement('babildxcom'));
  $xcom->appendChild($dom->createElement('geteflist'));
  $dom->formatOutput = true;
  $xml = $dom->saveXML();
  $rxml = babildxcom($server, $xml);
  $rdom = simplexml_load_string($rxml);
  //printxml($rxml);
  foreach($rdom->daqinfo->eflist as $eflist){
    $tefn = (int)$eflist->efn;
    $flag = 0;
    foreach($efnum as $m){
      if(isset($efefn[$m])){
	if($efefn[$m] == $tefn){
	  $flag = 1;
	}
      }
    }
    if(!$flag){
      $x = count($efnum);
      $efex[$x] = "0";
      $efefn[$x] = $tefn;
      $efhost[$x] = "";
      $efname[$x] = "";
      $efof[$x] = "0";
      $efnum[$x] = $x;
      $efmod[$x] = "1";
    }
  }


# sethdlist
  $efediti=0;$hdediti=0;$esediti=0;$reediti=0;
  $efedit = array();
  $hdedit = array();
  $esedit = array();
  $reedit = array();
  $rnameedit = 0;
  $rnumberedit = 0;
  $runinfoedit = 0;
  foreach($efnum as $m){
    if($efmod[$m] > 0){
      if(isset($efefn[$m])){
	$efedit[$efediti] = $m;
	$efediti++;
      }else if(isset($hdhdn[$m])){
	$hdedit[$hdediti] = $m;
	$hdediti++;
      }else if(isset($esebhost[$m])){
	$esedit[$esediti] = $m;
	$esediti++;
      }else if(isset($esreload[$m])){
	$reedit[$reediti] = $m;
	$reediti++;
      }else if(isset($srunname[$m])){
	$rnameedit = $m;
	$runinfoedit = 1;
      }else if(isset($srunnumber[$m])){
	echo "rnumber <br>";
	$rnumberedit = $m;
	$runinfoedit = 1;
      }
    }
  }

  if($efediti || $hdediti || $runinfoedit){
    $dom = new DomDocument('1.0');
    $xcom = $dom->appendChild($dom->createElement('babildxcom'));
    
# seteflist
    if($efediti){
      $seteflist = $xcom->appendChild($dom->createElement('seteflist'));
      for($i=0;$i<$efediti;$i++){
	$m = $efedit[$i];
	if($efefn[$m] > 0 && $efefn[$m] < 256){
	  $eflist = $seteflist->appendChild($dom->createElement('eflist'));
	  $eflist->appendChild($dom->createElement('efn', $efefn[$m]));
	  $eflist->appendChild($dom->createElement('host', $efhost[$m]));
	  $eflist->appendChild($dom->createElement('name', $efname[$m]));
	  $eflist->appendChild($dom->createElement('of', $efof[$m]));
	  $eflist->appendChild($dom->createElement('ex', $efex[$m]));
	}
      }
    }
    if($hdediti){
      $sethdlist = $xcom->appendChild($dom->createElement('sethdlist'));
      for($i=0;$i<$hdediti;$i++){
	$m = $hdedit[$i];
	if($hdhdn[$m] >= 0 && $hdhdn[$m] < 4){
	  $hdlist = $sethdlist->appendChild($dom->createElement('hdlist'));
	  $hdlist->appendChild($dom->createElement('hdn', $hdhdn[$m]));
	  $hdlist->appendChild($dom->createElement('ex', 1));
	  $hdlist->appendChild($dom->createElement('of', $hdof[$m]));
	  $hdlist->appendChild($dom->createElement('path', $hdpath[$m]));
	}
      }
    }
    if($rnameedit >= 0 or $rnumberedit >= 0){
      $setruninfo = $xcom->appendChild($dom->createElement('setruninfo'));
      if($rnameedit){
	$setruninfo->appendChild($dom->createElement('runname',
						     $srunname[$rnameedit]));
      }
      if($rnumberedit){
	$setruninfo->appendChild($dom->createElement('runnumber',
						     $srunnumber[$rnumberedit]));
      }
    }
    
    $dom->formatOutput = true;
    $xml = $dom->saveXML();

    /*
      echo "hoge1";
      printxml($xml);
      echo "end1<br>\n";
    */

    $rxml = babildxcom($server, $xml);
    
    $rdom = simplexml_load_string($rxml);
    foreach($rdom->error as $err){
      echo "<font color=red>".$err."</font><br>\n";
    }
  }

# reloadesdrv
  if($esediti || $reediti){
    $dom = new DomDocument('1.0');
    $xcom = $dom->appendChild($dom->createElement('babinfoxcom'));
    if($esediti){
      for($i=0;$i<$esediti;$i++){
	$m = $esedit[$i];
	$setesconfig = $xcom->appendChild($dom->createElement('setesconfig'));
	$setesconfig->appendChild($dom->createElement('efn', $esefn[$m]));
	if(isset($esebhost[$m])){
	  $setesconfig->appendChild($dom->createElement('host', $esebhost[$m]));
	}
	if(isset($esrtdrv[$m])){
	  $setesconfig->appendChild($dom->createElement('rtdrv', $esrtdrv[$m]));
	}
      }
    }
    if($reediti){
      for($i=0;$i<$reediti;$i++){
	$m = $reedit[$i];
	$reloadesdrv = $xcom->appendChild($dom->createElement('reloadesdrv'));
	$reloadesdrv->appendChild($dom->createElement('efn', $esreload[$m]));
	echo "m=".$m." val=".$esreload[$m]."<br>";
      }
    }
    $dom->formatOutput = true;
    $xml = $dom->saveXML();
    $rxml = babinfoxcom($server, $xml);
    $rdom = simplexml_load_string($rxml);
    foreach($rdom->error as $err){
      echo "<font color=red>".$err."</font><br>\n";
    }
  }

  echo "<font color=#993333>Saved</font><br>\n";
 }

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button> \n ";
echo "<button type='submit' name='save'>Save</button> \n ";

echo "<hr width=500 align=left>\n";
echo "<h2>Server = $server</h2>\n";

$fid = 1;

# get daqinfo
$dom = new DomDocument('1.0');
$xcom = $dom->appendChild($dom->createElement('babildxcom'));
$xcom->appendChild($dom->createElement('getdaqinfo'));
$dom->formatOutput = true;
$xml = $dom->saveXML();
$rxml = babildxcom($server, $xml);
$rdom = simplexml_load_string($rxml);
list($fid , $efnar) = editxeflist($fid, $rdom);
$fid = editxhdlist($fid, $rdom, $srv);
$fid = editxruninfo($fid, $rdom, $srv);

$dom = new DomDocument('1.0');
$xcom = $dom->appendChild($dom->createElement('babinfoxcom'));
$efncom = $xcom->appendChild($dom->createElement('getesconfig'));
foreach($efnar as $efn){
  $efncom->appendChild($dom->createElement('efn', $efn));
}
$dom->formatOutput = true;
$xml = $dom->saveXML();

$rxml = babinfoxcom($server, $xml);
$rdom = simplexml_load_string($rxml);

echo "<br>\n";
$fid = editxesconfig($fid, $rdom, $hostlist);

for($i=0;$i<$fid;$i++){
  echo "<input type=hidden name=mod".$i." value=0>";
  //echo "<input name=mod".$i." value=0>";
 }

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button>\n";
echo "<button type='submit' name='save'>Save</button> \n ";
echo "</form>";

?>

<script src="./formcol.js" type="text/javascript"></script>

</body>
</html>
