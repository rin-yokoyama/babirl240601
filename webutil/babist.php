<html>
<head>
<title>babirl DAQ Status Monitor</title>
</head>
<body>
<h1>babirl DAQ Status Monitor</h1>

<a href="./index.php">Back to index page</a>

<p><?php
require "babilib.php";

$arserver = array();

if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
  die();
 }

foreach($hostlist->host as $node){
  if($node->babild){
    array_push($arserver, $node->name);
    $on["$node->name"] = 0;
  }
}

echo date("Y/m/d H:i:s");

echo "<form action='babist.php' method='post'>";

$post = $_POST;
$opt = '';
$efn = '';

while(list($key, $val) = each($post)){
  foreach($arserver as $name){
    if(!strcmp($key, $name)){
      $on["$name"] = 1;
    }
  }
 }

echo "<hr width=500 align=left>\n";
$cnt = 0;
foreach($arserver as $name){
  if($on["$name"]){
    echo "<input type='checkbox' value='ON' name='$name' checked>$name</input>\n";
  }else{
    echo "<input type='checkbox' value='ON' name='$name'>$name</input>\n";
  }
  $cnt ++;
  if($cnt == 10){
    print "<br>\n";
    $cnt = 0;
  }
}

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button> \n ";

foreach($arserver as $name){
  if($on["$name"]){
    $server = $name;
    
    echo "<hr width=500 align=left>\n";
    echo "<h2>Server = $server</h2>\n";

    $st = getdaqinfo($server);

    if($st){
      $ft = ftdaqinfo();
      $ft .= "/";

      $i=0;
      for($i=0;$i<256;$i++){
	$ft .= fteflist($i);
	$ft .= "/";
      }
      $i=0;
      for($i=0;$i<10;$i++){
	$ft .= fthdlist($i);
	$ft .= "/";
      }
      $daqinfo = unpack($ft, $st);



#   Get ssminof
      $st = getssminfo($server);
      $ft = ftssminfo();
      $ssminfo = unpack($ft, $st);
      $of = $ofstr[$ssminfo['of']];
      
#   SSMINFO
      echo "<b>SSM Information</b><br>\n";
      if($ssminfo['ex']){
	echo "<table><tr><td bgcolor='#33ffff'>SSM hostname<td>$ssminfo[host] ($of)</tr>\n";
	echo "<tr><td bgcolor='#33ffff'>Start command<td>$ssminfo[start]</tr>\n";
	echo "<tr><td bgcolor='#33ffff'>Stop command<td>$ssminfo[stop]</tr>\n";
	echo "</table>\n";
      }else{
	echo "SSM is not exist\n";
      }
      echo "<br><br>\n";
      
#   EFLIST
      echo "<b>Event fragment</b><br>\n";
      echo "<table><tr bgcolor='#33ffff'><td>ID<td>Hostname<td>Nichname<td>on/off</tr>\n";
      for($i=0;$i<256;$i++){
	$exi = "efex$i";
	$namei = "efname$i";
	$hosti = "efhost$i";
	$ofi   = "efof$i";
	$of    = $ofstr[$daqinfo[$ofi]];
	if($daqinfo[$exi]){
	  echo "<tr><td>$i<td>$daqinfo[$hosti]<td>$daqinfo[$namei]<td>$of</tr>\n";
	}
      }
      echo "</table><br>\n";
      
#   HDLIST
      echo "<b>HD list</b><br>\n";
      echo "<table><tr bgcolor='#33ffff'><td>ID<td>Path<td>on/off</tr>\n";
      for($i=0;$i<10;$i++){
	$exi   = "hdex$i";
	$pathi = "hdpath$i";
	$ofi   = "hdof$i";
	$of    = $ofstr[$daqinfo[$ofi]];
	$freeai= "hdfreea$i";
	$freebi= "hdfreeb$i";
	if($daqinfo[$exi]){
	  echo "<tr><td>$i<td>$daqinfo[$pathi]<td>$of</tr>\n";
	}
      }
      echo "</table><br>\n";
      
      
#   DAQINFO
      echo "<b>EB Information</b><br>\n";
      $of = $ofstr[$daqinfo['babildes']];
      echo "<table><tr><td bgcolor='#33ffff'>EF Number<td>$daqinfo[efn]</tr>\n";
      $kb = (int)($daqinfo['ebsize']*2/1024);
      echo "<tr><td bgcolor='#33ffff'>Event Build Size<td>$daqinfo[ebsize] ($kb kB)</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Babildes mode<td>$of</tr>\n";
      echo "</table><br>\n";



#   Get Runinof
      $st = getruninfo($server);
      $ft = ftruninfo();
      $runinfo = unpack($ft, $st);

      $startdate = date("d-M-y H:i:s", $runinfo['starttime']);
      $stopdate = date("d-M-y H:i:s", $runinfo['stoptime']);

#   RUNINFO
      echo "<b>RUN Information</b><br>\n";
      $run = $runstr[$runinfo['runstat']];
      echo "<table><tr><td bgcolor='#33ffff'>Run name<td>$daqinfo[runname]</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Run number<td>$daqinfo[runnumber]</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Run status<td>$run</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Start date<td>$startdate</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Stop date<td>$stopdate</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Header<td>$runinfo[header]</tr>\n";
      echo "<tr><td bgcolor='#33ffff'>Ender<td>$runinfo[ender]</tr>\n";
      echo "</table>\n";


    }
  }
}

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button>\n";
echo "</form>";    


?></p>


</body>
</html>
