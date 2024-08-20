<html>
<head>
<title>babirl DAQ Process Manager</title>
</head>
<body>
<h1>babirl DAQ Process Manager</h1>

<a href="./index.php">Back to index page</a>

<p><?php
require "babilib.php";


$arserver = array();

if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
  die();
 }

foreach($hostlist->host as $node){
  if(!isset($node->disabled)){
    array_push($arserver, $node->name);
  }
 }

foreach($arserver as $name){
  $on["$name"] = 0;
}

$fcom = 0;
$ttimeout = '5';

$arname = array('babinfo', 'babild', 'babies', 'babissm', 'babian', 'babiau');
$artimeout = array('5', '10', '30', '60', '120');
$arbabiesop = array('-l', '-r', '-s', '-c', '-d');
$arbabildop = array('-l');
$arbabiauop = array('-lt', '-l');
$babiesoptlist = array();

echo date("Y/m/d H:i:s");

echo "<form action='babipm.php' method='post'>";

$opt = '';
$efn = '';
$all = 0;


// to keep check box
if(isset($_POST)){
  $post = $_POST;
  $postflag = 1;
  if(!count($post)) $postflag = 0;
}else{
  $postflag = 0;
}

$tmpfile = 'dat/babipm.tmp';
$onn = 0;


while(list($key, $val) = each($post)){
#  echo "key $key, val $val <br>\n";
  if(preg_match("/=/", $key)){
    list($ks, $cc) = split("=", $key);
  }else{
    $ks = '';
    $cc = '';
  }

  if(!strcmp($val, 'KILL')){
    $fcom = 1;
  }else if(!strcmp($val, 'RUN')){
    $fcom = 1;
  }else if(!strcmp($val, 'HALT')){
    $fcom = 1;
  }else if(!strcmp($val, 'REBOOT')){
    $fcom = 1;
  }else if(!strcmp($val, 'INIT')){
    $fcom = 1;
  }else if(!strcmp($val, 'TIMERESET')){
    $fcom = 1;
  }else if(!strcmp($cc, 'BABILDOPT')){
    $babildopt[$ks] = $val;
  }else if(!strcmp($cc, 'BABIAUOPT')){
    $babiauopt[$ks] = $val;
  }else if(!strcmp($cc, 'BABIESOPT')){
    $babiesopt[$ks] = $val;
  }else if(!strcmp($cc, 'BABILDEFN')){
    $babildefn[$ks] = $val;
  }else if(!strcmp($cc, 'BABIAUEFN')){
    $babiauefn[$ks] = $val;
  }else if(!strcmp($cc, 'BABIESEFN')){
    $babiesefn[$ks] = $val;
  }elseif(!strcmp($key, 'TIMEOUT')){
    $ttimeout = $val;
    settimeout($ttimeout);
  }
  
  foreach($arserver as $name){
    if(!strcmp($key, $name)){
      $on["$name"] = 1;
      $onn++;
    }
  }
  if(!strcmp($key, 'all')){
    $all = 1;
  }
 }

if($postflag){
  if($onn){
    $fd = fopen($tmpfile, 'w');
    foreach($arserver as $name){
      if($on["$name"]){
	fprintf($fd, "%s\n", $name);
      }
    }
    fclose($fd);
  }else{
    if(file_exists($tmpfile)){
      unlink($tmpfile);
    }
  }
}else{
  if(file_exists($tmpfile)){
    if(($fd = fopen($tmpfile, 'r'))){
      while(!feof($fd)){
	$line = fgets($fd);
	$nline = rtrim($line, "\n");
	foreach($arserver as $name){
	  if(!strcmp($nline, $name)){
	    $on["$name"] = 1;
	  }
	}
      }
    }
  }else{
    // noop
  }
}
////// end of for check boxes

echo "<hr width=800 align=left>\n";

$cnt = 0;
if($all){
  echo "<input type='checkbox' value='ON' name='all' checked>all</input>\n";
 }else{
  echo "<input type='checkbox' value='ON' name='all'>all</input>\n";
 }
foreach($arserver as $name){
  if($on["$name"] || $all){
    echo "<input type='checkbox' value='ON' name='$name' checked>$name</input>\n";
    $on["$name"] = 1;
  }else{
    echo "<input type='checkbox' value='ON' name='$name'>$name</input>\n";
  }
  $cnt ++;
  if($cnt == 10){
    print "<br>\n";
    $cnt = 0;
  }
}

echo "<hr width=800 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button> \n ";
echo "Timeout <select name='TIMEOUT'>\n";
 foreach($artimeout as $t){
   if($t == $ttimeout){
     echo "<option selected value='$t'>$t\n";
   }else{
     echo "<option value='$t'>$t\n";
   }
 }
echo "</select> s\n";

if($fcom){
echo "<hr width=800 align=left>\n";
  echo "<h2>Command</h2>\n";
  echo "<table bgcolor='#ffffcc'>\n";
  echo "<tr bgcolor='#ccff33'><td>Server</td><td>Process</td><td>Command</td><td>Option</td><td>EFN<td></tr>\n";
 }
$post = $_POST;
while(list($key, $val) = each($post)){
  if(!strcmp($val, 'KILL')){
    list($server, $name) = split("=", $key);
    echo "<tr><td>$server</td><td>$name</td><td>KILL</td>\n";
    echo "<td></td><td></td>\n";
    killpid($server, $name);
  }
  if(!strcmp($val, 'HALT')){
    list($server, $name) = split("=", $key);
    echo "<tr><td>$server</td><td>$name</td><td>HALT</td>\n";
    echo "<td></td><td></td>\n";
    $arg = "/sbin/halt";
    execarg($server, $arg);
  }
  if(!strcmp($val, 'REBOOT')){
    list($server, $name) = split("=", $key);
    echo "<tr><td>$server</td><td>$name</td><td>REBOOT</td>\n";
    echo "<td></td><td></td>\n";
    $arg = "/sbin/reboot";
    execarg($server, $arg);
  }
  if(!strcmp($val, 'INIT')){
    list($server, $name) = split("=", $key);
    echo "<tr><td>$server</td><td>$name</td><td>INIT</td>\n";
    echo "<td></td><td></td>\n";
    $path = "xml/" . $server . ".xml";
    $host = simplexml_load_file($path);
    if(!isset($host->init->path)){
      $arg = "/home/daq/daqconfig/$server/init/daqinitrc";
    }else{
      $arg = $host->init->path;
    }
    echo "<tr><td>Path = <td colspan=4>".$arg."</tr>\n";
    execarg($server, $arg);
  }
  if(!strcmp($val, 'TIMERESET')){
    list($server, $name) = split("=", $key);
    echo "<tr><td>$server</td><td>$name</td><td>TIMERESET</td>\n";
    echo "<td></td><td></td>\n";
    $path = "xml/" . $server . ".xml";
    $host = simplexml_load_file($path);
    if(isset($host->function->timeresetpath)){
      $arg = $host->function->timeresetpath;
    }else{
      $arg = "/home/daq/bin/resettimestamp";
    }
    echo $arg."<br>\n";
    execarg($server, $arg);
  }
  if(!strcmp($val, 'RUN')){
    list($server, $name) = split("=", $key);
    if(!strcmp($name, 'babild')){
      $opt = $babildopt[$server];
      $efn = $babildefn[$server];
      $arg = " $opt $efn";
    }else if(!strcmp($name, 'babiau')){
      $opt = $babiauopt[$server];
      $efn = $babiauefn[$server];
      $arg = " $opt $efn";
    }else if(!strcmp($name, 'babies')){
      $opt = $babiesopt[$server];
      $efn = $babiesefn[$server];
      $arg = " $opt $efn";
    }else{
      $arg = '';
    }
    echo "<tr><td>$server</td><td>$name</td><td>RUN</td>\n";
    echo "<td>$opt</td><td>$efn</td>\n";
    execbabiproc($server, $name, $arg);
  }
 }
echo "</table><br>";

usleep(100000);

echo "<hr width=800 align=left>\n";

$lid = 0;
echo "<table aligin=left>\n";
foreach($arserver as $name){
  if($on["$name"]){
    $server = $name;

    if($lid == 0) echo "<tr valign=top>\n";
    echo "<td>\n";
    echo "<h3>Server = $server</h3>\n";
    
#    $hdlist = gethdlist($server);
    $connect = execarg($server, '/bin/ls');
    $ip = ip2long(gethostbyname($server)) & 0xff;

    $path = "xml/" . $server . ".xml";
    $host = simplexml_load_file($path);
    
    echo "$host->description <br>";
    echo "$host->comment <br>";

#    $connect = 0;
#    if($n > 0){
#      $connect = 1;
#      echo "<table bgcolor='#ffffcc'>\n";
#      echo "<tr bgcolor='#33ffcc'><td>Device</td> <td>Path</td> <td>Total (GB)</td> <td>Used (GB)</td> <td>Free (GB)</td>\n";
#      sort($hdlist);
#      for($i=0;$i<$n;$i++){
#	list($dev, $path, $tot, $used, $free) = $hdlist[$i];
#	printf("<tr><td>%20s</td> <td>%20s</td> <td align=right>%4.1f</td> <td align=right>%4.1f</td> <td align=right>%4.1f</td></tr>\n",
#	       $dev, $path, $tot, $used, $free);
#	
#      }
#      echo "</table><br>\n";
#    }
    
    if($connect){
      echo "<table  bgcolor='#ffffcc'>\n";
      echo "<tr bgcolor='#ffccff'><td>Process</td> <td>PID</td><td>Command</td><td>Option</td><td>EFN</td></tr>\n";
      $n = count($arname);
      for($i=0;$i<$n;$i++){
	$name = $arname[$i];
	$pid = getpid($server, $name);
	$pidlist{$name} = $pid;
	if($pid > 0){
	  $ispid{$name} = 1;
	}else{
	  $ispid{$name} = 0;
	}
      }
      for($i=0;$i<$n;$i++){
	$name = $arname[$i];
	$pid = $pidlist{$name};
	if($pid > 0){
	  echo "<tr><td>$name</td><td>$pid</td><td align='center'>";

	  if((strcmp($name,'babinfo') == 0) && $ispid{'babild'}){
	    echo "<button disabled type='submit' name='$server=$name' value='KILL'>KILL</button></td><td></td>\n";
	  }else{
	    echo "<button type='submit' name='$server=$name' value='KILL'>KILL</button></td><td></td>\n";
	  }
	}else{
	  echo "<tr><td>$name</td><td>-----</td><td align='center'>";
	  if(strcmp($name,'babinfo') == 0){
	    //if(!$ispid{'babies'}){
	      $ds = disdefon($host->babinfo->onoff);
	      echo fmrunbt($ds, $server, $name);
	      //}
	    echo "</td>\n";
	  }else if(strcmp($name,'babild') == 0){
	    if($ispid{'babinfo'}){
	      $ds = disdefon($host->babild->onoff);
	      echo fmrunbt($ds, $server, $name);
	    }
	    echo "</td>\n";
	    echo "<td><select name='$server=BABILDOPT'>\n";
	    foreach($arbabildop as $t){
	      $sl = chksel($t, $host->babild->option);
	      echo "<option $sl value='$t'>$t\n";
	    }
	    echo "</select></td>\n";

	    $ipstr = prisel($host->babild->efn, $host->efn, $ip);
	    $str = fmtxt($server, 'BABILDEFN', $ipstr);
	    echo "<td>$str</td>\n";
	  }else if(strcmp($name, 'babies') == 0){
	    //if(!$ispid{'babinfo'}){
	      $ds = disdefon($host->babies->onoff);
	      echo fmrunbt($ds, $server, $name);
	      //}
	    echo "</td>\n";
	    echo "<td><select name='$server=BABIESOPT'>\n";
	    foreach($arbabiesop as $t){
	      $sl = chksel($t, $host->babies->option);
	      echo "<option $sl value='$t'>$t\n";
	    }

	    echo "</td>\n";

	    $ipstr = prisel($host->babies->efn, $host->efn, $ip);
	    $str = fmtxt($server, 'BABIESEFN', $ipstr);
	    echo "<td>$str</td>\n";

	  }else if(strcmp($name,'babiau') == 0){
	    if(!$ispid{'babinfo'} && !$ispid{'babild'}){
	      $ds = disdefon($host->babiau->onoff);
	      echo fmrunbt($ds, $server, $name);
	    }
	    echo "</td>\n";
	    echo "<td><select name='$server=BABIAUOPT'>\n";
	    foreach($arbabiauop as $t){
	      $sl = chksel($t, $host->babiau->option);
	      echo "<option $sl value='$t'>$t\n";
	    }
	    echo "</select></td>\n";
	    $ipstr = prisel($host->babiau->efn, $host->efn, $ip);
	    $str = fmtxt($server, 'BABIAUEFN', $ipstr);
	    echo "<td>$str</td>\n";

	  }else{
	    echo "<button type='submit' name='$server=$name' value='RUN'>RUN</button>";
	  }
	}
      }
      echo "</table>";

      $ds = disdefoff($host->reboot);
      echo fmcombt($ds, $server, $name, "REBOOT");
      $ds = disdefoff($host->halt);
      echo fmcombt($ds, $server, $name, "HALT");
      $ds = disdefoff($host->init->onoff);
      echo fmcombt($ds, $server, $name, "INIT");

      if($host->function->timereset){
	echo fmcombtcol("", $server, $name, "TIMERESET", "red");
      }
    }
    $lid ++;
    if($lid == 3){
      echo "</tr>\n";
      $lid = 0;
    }
  }
}
echo "</table>\n";

echo "<hr width=800 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button>\n";
echo "</form>";    


?></p>


</body>
</html>
