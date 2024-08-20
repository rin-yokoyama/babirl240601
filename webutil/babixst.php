<html>
<head>
<title>babirl XML DAQ Status Monitor</title>
</head>
<body>
<h1>babirl XML DAQ Status Monitor</h1>

<a href="./index.php">Back to index page</a>

<p><?php
require "babilib.php";
require "babixlib.php";
   
$arserver = array();

if(isset($_POST)){
  $post = $_POST;
  $postflag = 1;
  if(!count($post)) $postflag = 0;
}else{
  $postflag = 0;
}

$ctrl = 0;
if(isset($_GET['ctrl'])){
  if(!strcmp($_GET['ctrl'], 'on')){
    $ctrl = 1;
  }
}
$fsrv = 0;
if(isset($_GET['srv'])){
  $server = $_GET['srv'];
  $fsrv = 1;
}

if(!$fsrv){
  if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
    echo "no hostlist!!<br>";
    echo "<a href='index.php'>Go to the index page</a>";
    die();
  }
}else{
  echo "srv by post=".$server."<br>\n";
}

//echo $hostlist->asXML();
if(!$fsrv){
  foreach($hostlist->host as $node){
    if($node->babild){
      array_push($arserver, $node->name);
      $on["$node->name"] = 0;
    }
  }
}

echo date("Y/m/d H:i:s");


$tmpfile = 'dat/babixst.tmp';

$opt = '';
$efn = '';
$onn = 0;
$rserver = '';
$rcom = 0;
$rnssta = 0;
$rstop = 0;

if($fsrv){
  $on[$server] = 1;
  $onn = 1;
  array_push($arserver, $server);
}

if($postflag){
  while(list($key, $val) = each($post)){
    foreach($arserver as $name){
      if(!strcmp($key, $name)){
	$on["$name"] = 1;
	$onn++;
      }
    }
    if(!strcmp($key, 'NSSTA')){
      $rcom = 1;
      $rnssta = 1;
      $rserver = $post['NSSTA'];
    }
    if(!strcmp($key, 'STOP')){
      $rcom = 1;
      $rstop = 1;
      $rserver = $post['STOP'];
    }
  }

  if($onn && !$fsrv){
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

if($rcom==1){
  echo '<br><br>';
  $domr = new DomDocument('1.0');
  $xcomr = $domr->appendChild($domr->createElement('babildxcom'));
  $xcomr->appendChild($domr->createElement('getruninfo'));
  $domr->formatOutput = true;
  $xmlr = $domr->saveXML();

  $rxmlr = babildxcom($rserver, $xmlr);
  $rdomr = simplexml_load_string($rxmlr);
  $rstat = getxrunstat($rdomr);

  if($rnssta){
    if(!strcmp($rstat, 'IDLE')){

      $domr = new DomDocument('1.0');
      $xcomr = $domr->appendChild($domr->createElement('babildxcom'));
      $xcomr->appendChild($domr->createElement('nssta'));
      $domr->formatOutput = true;
      $xmlr = $domr->saveXML();
      $rxmlr = babildxcom($rserver, $xmlr);

      echo '<font color=blue>'.$rserver.' NSSTA</font>';
    }else{
      echo 'RUN Status is not IDLE';
    }
  }else if($rstop){
    if(!strcmp($rstat, 'NSSTA')){
      $domr = new DomDocument('1.0');
      $xcomr = $domr->appendChild($domr->createElement('babildxcom'));
      $xcomr->appendChild($domr->createElement('stop', 'stop via web'));
      $domr->formatOutput = true;
      $xmlr = $domr->saveXML();
      $rxmlr = babildxcom($rserver, $xmlr);

      echo '<font color=blue>'.$rserver.' STOP</font>';
    }else{
      echo 'RUN Status is not NSSTA';
    }
  }
  echo "<br>\n";
}

$rstat = '';

if($ctrl && !$fsrv){
  echo "<form action='babixst.php?ctrl=on' method='post'>";
}else if($ctrl && $fsrv){
  echo "<form action='babixst.php?ctrl=on&srv=".$server."' method='post'>";
}else if(!$ctrl && $fsrv){
  echo "<form action='babixst.php?srv=".$server."' method='post'>";
}else{
  echo "<form action='babixst.php' method='post'>";
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


$dom = new DomDocument('1.0');
$xcom = $dom->appendChild($dom->createElement('babildxcom'));
$xcom->appendChild($dom->createElement('getdaqinfo'));
$dom->formatOutput = true;
$xml = $dom->saveXML();

$domr = new DomDocument('1.0');
$xcomr = $domr->appendChild($domr->createElement('babildxcom'));
$xcomr->appendChild($domr->createElement('getruninfo'));
$domr->formatOutput = true;
$xmlr = $domr->saveXML();

foreach($arserver as $name){
  if($on["$name"]){
    $server = $name;
    
    //echo "hoge $server";
    echo "<hr width=500 align=left>\n";
    echo "<h2>Server = $server</h2>\n";
    $rxml = babildxcom($server, $xml);
    $rdom = simplexml_load_string($rxml);

    printxdaqinfo($rdom);
    printxeflist($rdom);
    printxhdlist($rdom);
    //echo $rxml;

    $rxmlr = babildxcom($server, $xmlr);
    $rdomr = simplexml_load_string($rxmlr);
    printxruninfo($rdomr);

    if($ctrl == 1){
      $rstat = getxrunstat($rdomr);
      echo '(Controllable)<br>';
      if(!strcmp($rstat, 'NSSTA')){
	echo "<button type='submit' name='STOP' value='".$server."'>STOP</button>\n";
      }else if(!strcmp($rstat, 'IDLE')){
	echo "<button type='submit' name='NSSTA' value='".$server."'>NSSTA</button>\n";
      }else if(!strcmp($rstat, 'START')){
	echo '<i>Now RUN is running</i>';
      }
      echo '<br>';
    }


  }
}

echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button>\n";
echo "</form>";    


?></p>


</body>
</html>
