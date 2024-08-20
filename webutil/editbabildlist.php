<html>
<head>
<title>babirl edit XML babild list</title>
</head>
<body>
<h1>babirl edit XML babild list</h1>

<a href="./index.php">Back to index page</a>

<p><?php
require "babilib.php";
require "babixlib.php";

function isel($id, $def){
  $ret = "<select name='hdi".$id."'>\n";
  for($i=0;$i<7;$i++){
    if($def == $i){
      $sel = 'selected';
    }else{
      $sel = '';
    }
    $ret .= "<option ".$sel." value='".$i."'>".$i."\n";
  }
  $ret .= "</select>\n";

  return $ret;
}

function itex($id, $def){
  $ret = "<input type='text' size=60 name='hdp".$id."' value='".$def."'>\n";

  return $ret;
}

function drvtex($name, $def, $id){
  $ret = "<input type='text' size=60 name='drv".$name."_".$id."' value='".$def."'>\n";

  return $ret;
}

$thisfile = basename($_SERVER['SCRIPT_NAME']);
$thishost = $_GET["host"];

if(!$thishost){
  echo "no host name<br>\n";
  die();
 }

echo date("Y/m/d H:i:s");

echo "<form action='".$thisfile."?host=".$thishost."' method='post'>";

$post = $_POST;
$save = 0;
$hdi = 0;
##print_r($post);
while(list($key, $val) = each($post)){
  if(!strcmp($key, 'save')){
    $save = 1;
  }
  if(!strncmp($key, 'hdi', 3)){
    $hdi++;
  }
  if(!strncmp($key, 'drv', 3)){
		//$li = explode("_", $key);
    $x = strrpos($key, '_');
    $li[0] = substr($key, 0, $x);		
    $li[1] = substr($key, $x+1);
    sscanf($li[0], "drv%s", $thost);
    $ti = $li[1];
    $rti[$thost] = $ti;
    $rtv[$thost][$ti] = $val;
#    echo "thost=".$thost."/".$rti[$thost]."  val=".$rtv[$thost][$ti]."<br>";
  }
#  echo "key=$key  val=$val <br>\n";
 }


# Find selected server from host
$file = "dat/hostlist.xml";
$hostdom = dom_loadfile_node($dom, $file, 'hostlist');
$thisdom = dom_find_hostname($dom, $thishost);

if(!dom_node_isset($thisdom, 'babild')){
  echo "<font color=red>".$thishost." is not babild server<br>\n";
  die();
 }else{
  $babilddom = dom_find_node($thisdom, 'babild');
 }

# Store XML to file
if($save){
# delete all hdlist 
  dom_remove_children_tag($babilddom, 'hd');
# set new hdlist
  for($i=0;$i<$hdi;$i++){
    $idhdi = "hdi".$i;
    $idhdp = "hdp".$i;
    $id = $post[$idhdi];
    $text = $post[$idhdp];
    if(strcmp($text, '')){
      $xhd = $babilddom->appendChild($dom->createElement('hd'));
      $xhd->appendChild($dom->createElement('id', $id));
      $xhd->appendChild($dom->createElement('path', $text));
    }
  }
  
  $hlist = $hostdom->getElementsByTagName('host');
  foreach($hlist as $hd){
    if(dom_node_isset($hd, 'disabled')) continue;
    $thost = dom_find_node_value($hd,'name');
    dom_remove_children_tag($hd, 'rtdrv');
    if(isset($rti[$thost])){
      for($i=0;$i<=$rti[$thost];$i++){
	$dp = "/home/daq/daqconfig/".$thost."/";
	$np = $rtv[$thost][$i];
	if(strcmp($dp, $np) && strlen($np)){
	  if(!($rtdom = dom_find_node($hd, 'rtdrv'))){
	    $rtdom = $hd->appendChild($dom->createElement('rtdrv'));
	  }
	  $rtdom->appendChild($dom->createElement('path', $np));
	}
      }
    }
  }
# save into file
  dom_savefile($dom, $file);
 }

if(!($hostlist = simplexml_load_file($file))){
  die();
 }
foreach($hostlist->host as $t){
  if(!strcmp($t->name, $thishost)){
    $srv = $t;
  }
}

if(!$srv->babild){
  echo "<font color=red>".$thishost." is not babild server<br>\n";
  die();
 }

$babild = $srv->babild;

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='save'>Save</button> \n ";
echo "<button type='submit' name='reset'>Reset</button> \n ";

echo "<h2>Server = $thishost</h2>\n";

echo "<font color=#883355 size=+1><B>HD list</B></font>\n";
echo "<table><tr bgcolor='#33cc66'><td>ID<td>Path</td>\n";
$i = 0;
$mxhdi = 0;
foreach($babild->hd as $hd){
  echo "<tr><td>".isel($i, $hd->id)."\n";
  echo "<td>".itex($i, $hd->path)."\n";
  if($hd->id > $mxhdi) $mxhdi = $hd->id;
  $i++;
}
if($mxhdi != 0) $mxhdi++;
echo "<tr><td>".isel($i, $mxhdi)."\n";
echo "<td>".itex($i, '')."\n";
echo "</table>";


foreach($hostlist->host as $t){
  if(isset($t->disabled)) continue;
  //if(!isset($t->babild)){
    echo "<h3>Babies = ".$t->name."</h3>\n";
    echo "<table><tr bgcolor='#ff33cc'><td>Driver Path</td>\n";
    $i = 0;
    if(isset($t->rtdrv)){
      foreach($t->rtdrv->path as $path){
	echo "<tr><td>".drvtex($t->name, $path, $i)."\n";
	$i++;
      }
    }
    $dp = "/home/daq/daqconfig/".$t->name."/";
    echo "<tr><td>".drvtex($t->name, $dp, $i)."\n";
    echo "</table>\n";
    //}
}

?></p>


</body>
</html>
