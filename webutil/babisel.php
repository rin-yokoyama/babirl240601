<html>
<head>
<title>babirl Trigger Selector</title>
</head>
<body>
<h1>babirl Trigger Selector</h1>

<a href="./index.php">Back to index page</a>

<p><?php
require "babilib.php";


$rcmd = "/usr/nbbq/bin/cmdvme -lr ";
$wcmd = "/usr/nbbq/bin/cmdvme -lw ";

if(!($selector = simplexml_load_file("./selector.xml"))){
  echo "selector.xml is not found.\n";
  die();
 }

echo "<form action='babisel.php' method='post'>";


$post = $_POST;
$wf = 0; $wfc = ""; 
$rf = 0; $rfc = ""; $rstr = "";
$srv = ""; $ststr = ''; 
$wtbit = 0; $wvbit = 0; $wsbit = 0;
$wtstr = ''; $wvstr = ''; $wsstr = '';

while(list($key, $val) = each($post)){
  if(!strcmp($key, "READCOMMAND")){
    $rf = 1;
  }else if(!strcmp($key, "WRITECOMMAND")){
    $rf = 1;
    $wf = 1;
  }else{
    foreach($selector->trigger->name as $name){
      if(!strcmp($key, "TRIGGERLIST".$name)){
	$tbit = hexdec($val);
	$wtbit = $wtbit | $tbit;
      }
    }
    foreach($selector->veto->name as $name){
      if(!strcmp($key, "VETOLIST".$name)){
	$tbit = hexdec($val);
	$wvbit = $wvbit | $tbit;
      }
    }
    foreach($selector->ssm->name as $name){
      if(!strcmp($key, "SSMLIST".$name)){
	$tbit = hexdec($val);
	$wsbit = $wsbit | $tbit;
      }
    }
  }
 }

echo "<hr width=500 align=left>\n";

echo "<button type='submit' name='READCOMMAND'>READ</button>\n";
echo "<button type='submit' name='WRITECOMMAND'>WRITE</button>\n";

$srv = $selector->server->host;
echo "<h2>$srv</h2>";
if($wf){
  foreach($selector->server->address->trigger as $addr){
    $wfc = $wcmd.$addr;
    $wfc = $wfc." 0x".dechex($wtbit);
    $wtstr = dechex($wtbit);
    execarg($srv, $wfc);
#    echo "wtrg $srv / $wfc<br>\n";
    usleep(100000);
  }
  foreach($selector->server->address->veto as $addr){
    $wfc = $wcmd.$addr;
    $wfc = $wfc." 0x".dechex($wvbit);
    $wvstr = dechex($wvbit);
    execarg($srv, $wfc);
#    echo "wveto $srv / $wfc<br>\n";
    usleep(100000);
  }
  foreach($selector->server->address->ssm as $addr){
    $wfc = $wcmd.$addr;
    $wfc = $wfc." 0x".dechex($wsbit);
    $wsstr = dechex($wsbit);
    execarg($srv, $wfc);
#    echo "wveto $srv / $wfc<br>\n";
    usleep(100000);
  }
 }

$rtstr = '';
$rvstr = '';
$rsstr = '';

$rtval = ''; $rvval = '';
if($rf){
  $rfc = $rcmd.$selector->server->address->trigger[0];
  $rtstr = execargr($srv, $rfc);
  usleep(100000);
  $rfc = $rcmd.$selector->server->address->veto[0];
  $rvstr = execargr($srv, $rfc);
  usleep(100000);
  $rfc = $rcmd.$selector->server->address->ssm[0];
  $rsstr = execargr($srv, $rfc);
  usleep(100000);
  $rfc = $rcmd.$selector->server->address->status;
  $ststr = execargr($srv, $rfc);
 }

$rtval = hexdec($rtstr);
$rvval = hexdec($rvstr);
$rsval = hexdec($rsstr);
$stval = hexdec($ststr);

echo "<table><tr><td valign=top>\n";
# for trigger
echo "<table bgcolor=#eeffcc><tr><td><td><font color=#3300ff size=+1>Trigger</font>\n";
foreach($selector->trigger->name as $name){
  $bit = hexdec($name['bit']);
  $val = $name['bit'];
  echo "<tr>";
  if($rtval & $bit){
    $chkd = 'checked';
    $col = "#cc6633";
    $bb = "<b>";
    $ubb = "</b>";
  }else{
    $chkd = '';
    $col = "#9999ff";
    $bb = "";
    $ubb = "";
  }
  echo "<td><input type='checkbox' value='$val' name='TRIGGERLIST$name' $chkd></input>\n";
  echo "<td><font color=$col>$bb$name$ubb</font>";
  
  if($stval & $bit){
    $tbg = "#cc3333";
  }else{
    $tbg = "#ffffff";
  }
#  echo "<td bgcolor=$tbg>&nbsp;&nbsp;";
  echo "<td>&nbsp;&nbsp;";

}
echo "</table><td valign=top>\n";

# for ssm
echo "<table  bgcolor=#ffeecc><tr><td><td><font color=#6633cc size=+1>SSM</font>\n";
foreach($selector->ssm->name as $name){
  $bit = hexdec($name['bit']);
  $val = $name['bit'];
  echo "<tr>";
  if($rsval & $bit){
    $chkd = 'checked';
    $col = "#cc6633";
    $bb = "<b>";
    $ubb = "</b>";
  }else{
    $chkd = '';
    $col = "#9999ff";
    $bb = "";
    $ubb = "";
  }
  echo "<td><input type='checkbox' value='$val' name='SSMLIST$name' $chkd></input>\n";
  echo "<td><font color=$col>$bb$name$ubb</font>";

  if($stval & $bit){
    $tbg = "#ff3300";
  }else{
    $tbg = "#ffffff";
  }
  echo "<td bgcolor=$tbg>&nbsp;&nbsp;";
}

# for veto
echo "<tr><td><td><font color=#6633cc size=+1>VETO</font>\n";
foreach($selector->veto->name as $name){
  $bit = hexdec($name['bit']);
  $val = $name['bit'];
  echo "<tr>";
  if($rvval & $bit){
    $chkd = 'checked';
    $col = "#cc6633";
    $bb = "<b>";
    $ubb = "</b>";
  }else{
    $chkd = '';
    $col = "#9999ff";
    $bb = "";
    $ubb = "";
  }
  echo "<td><input type='checkbox' value='$val' name='VETOLIST$name' $chkd></input>\n";
  echo "<td><font color=$col>$bb$name$ubb</font>";

  if($stval & $bit){
    $tbg = "#ff3300";
  }else{
    $tbg = "#ffffff";
  }
  echo "<td bgcolor=$tbg>&nbsp;&nbsp;";
}
echo "</table>\n";
echo "</table>\n";

echo "</form>\n";
echo "<hr width=500 align=left>\n";
echo "<font color=#999999>\n";
echo "raw value<br>\n";
echo "read trigger = 0x$rtstr / ssm = 0x$rsstr / veto = 0x$rvstr<br>";
echo "write trigger = 0x$wtstr / ssm = 0x$wsstr / veto = 0x$wvstr<br>";
echo "status = 0x$ststr<br>\n";

foreach($selector->server->address->children() as $addr){
  $str = $addr->getName();
  echo "vme address $str $addr<br>\n";
}

echo "<br>";
echo "</font>";
?>

</body>
</html>
