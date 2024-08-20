<?php

$post = $_POST;
$opt = '';
$efn = '';

$head = '';
$html = '';
$rate = '';

$arreftime = array('None','5','10','30','60');

if(isset($_POST['rtime'])){
  $rtime = $_POST['rtime'];
}elseif(isset($_GET['rtime'])){
  $rtime = $_GET['rtime'];
}else{
  $rtime = '';
}

if($rtime == ''){
  $reftime = 'None';
 }else{
  $reftime = $rtime;
 }

$curung = 0;
$totalung = 0;

$head .=  "<head><head>\n";
if(strcmp($reftime, 'None')){
  $head .=  "<meta http-equiv=\"Refresh\" content=\"$reftime; url='babiscr.php?rtime=$reftime'\">\n";
 }
$head .=  "<title>babirl DAQ Scaler Monitor</title></head>\n";
$head .=  "<body><h1>babirl DAQ Scaler Monitor</h1>\n";
$head .=  "<a href=\"./index.php\">Back to index page</a>\n";
$head .=  "<p>\n";


require "babilib.php";
$arserver = array();

//list($gn, $gc) = split(" ", $line);
// clkn[id] = ch
// cnkr[id] = rate

if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
  die();
}

$clkn = array();
$clkr = array();

$head .=  date("Y/m/d H:i:s");
$head .=  "<form action='babiscr.php' method='post'>";

// to keep check box
if(isset($_POST)){
  $post = $_POST;
  $postflag = 1;
  if(!count($post)) $postflag = 0;
}else{
  $postflag = 0;
}

$arserver = array();
$srvon = array();
$tmpfile = 'dat/babiscr.tmp';
$stmpfile = 'dat/babiscrsrv.tmp';

foreach($hostlist->host as $node){
  if(isset($node->babild)){
		#    if(strcmp($node->name,$babild_name) == 0){
		$n = $node->name->__toString();
		if(isset($node->disabled)){
			$srvon[$n] = 0;
		}else{
			$srvon[$n] = 1;
			array_push($arserver, $n);
			#      }
    }
  }
}

$on = array();
$srvall = 0;
$scrs = array();
$scrlist = array();
foreach($arserver as $name){
  if(isset($srvon[$name])){
    $server = $name;
    $st = getscrlist($server);
    #      echo "hogehoge";
    if($st){
      #   Get scrlist
      $ft = "iscranan/";
      for($i=0;$i<MAXSCRANA;$i++){
        $ft .= ftscrlist($i);
        $ft .= "/";
      }
    }

		$scrlist[$name] = unpack($ft, $st);

		$all[$name] = 0;
		$ungn[$name] = 0;
		$ungc[$name] = 0;
		
		for ($i = 0; $i < $scrlist[$name]["scranan"]; $i++) {
			$indexID = "scrid$i";
			$indexName = "idname$i";
			$scrs[$name][$scrlist[$name][$indexID]] = $scrlist[$name][$indexName];
		}
		
		reset($post);
		while(list($key, $val) = each($post)){
			foreach($scrs[$name] as $id => $tname) {
				if(!strcmp($key, $id.":".$name)){
					$on[$name][$id] = 1;
				}
			}
			if (!strcmp($key, 'all:'.$name)) {
				$all[$name] = 1;
			}
			if(!strcmp($key, 'srvall')){
				$srvall = 1;
			}
		}
	}
}

if ($postflag) {
	$fd = fopen($stmpfile, 'w');
	foreach($arserver as $name){
		if($srvon[$name]){
			fprintf($fd, "%s\n", $name);
		}
	}
	fclose($fd);


	$fd = fopen($tmpfile, 'w');
	foreach($arserver as $name){
		if($srvon[$name]) {
			foreach ($scrs[$name] as $id => $tname) {
				if (isset($on[$name][$id])){
					if($on[$name][$id] || $all[$name]){
						fprintf($fd, "%s:%s\n", $id, $name);
					}
				}
			}
		}
	}
	fclose($fd);

}

if(file_exists($stmpfile)){
	if(($fd = fopen($stmpfile, 'r'))){
		while(!feof($fd)){
			$line = fgets($fd);
			$nline = rtrim($line, "\n");
			$srvon[$nline] = 1;
		}
	}
}

if (file_exists($tmpfile)) {
	if(($fd = fopen($tmpfile, 'r'))) {
		while(!feof($fd)) {
			$line = fgets($fd);
			$nline = rtrim($line, "\n");
			foreach($arserver as $name){
				foreach ($scrs[$name] as $id => $tname) {
					$kid = $id.":".$name;
					if (!strcmp($nline, $kid)) {
						$on[$name][$id] = 1;
					}
				}
			}
		}
	}
}

////// end of for check boxes
if($srvall){
	$head .= "<input type='checkbox' name='srvall";
	$head .= "' checked>all</input>\n";
}else{
	$head .= "<input type='checkbox' name='srvall";
	$head .= "'>all</input>\n";
}
foreach($arserver as $name){
	$chk = '';
	if($srvon[$name]){
		$chk = 'checked';
	}
	$head .= "<input type='checkbox' name='srv:".$name;
	$head .= "' ".$chk." >".$name."</input>\n";
}


foreach($arserver as $name){
	$cnt = 0;
	$head .= "<hr width=500 align=left>\n";
	$cnt = 0;
	if($all[$name]){
		$head .= "<input type='checkbox' name='all:".$name;
		$head .= "' checked>all</input>\n";
	}else{
		$head .= "<input type='checkbox' name='all:".$name;
		$head .= "'>all</input>\n";
	}
	foreach ($scrs[$name] as $id => $tname){
		$chk = '';
		if(isset($on[$name][$id])){
			if($on[$name][$id]){
				$chk = 'checked';
			}
		}
		if($all[$name]){
			$chk = 'checked';
			$on[$name][$id] = 1;
		}

		$head .= "<input type='checkbox' name='$id:".$name;
		$head .= "' ".$chk." >$id ($name)</input>\n";
		
		$cnt ++;
		if($cnt == 5){
			$head .= "<br>\n";
			$cnt = 0;
		}
	}
}


#$lastrunFile = fopen("lastRun.txt", "r");
#$lastrun = trim(fgets($lastrunFile));
#fclose($lastrunFile);

#$head .=  "<hr width=500 align=left>\n";
#$head .=  "<input type='button' onclick=\"location.href='scalerHistory/babiscr_".$lastrun.".html'\" value='Prev'/> \n ";
$head .= "<br>\n";
$head .=  "<button type='submit' name='refresh'>Refresh</button>\n ";
$head .=  "Refresh time <select name='rtime'>\n";
foreach($arreftime as $t){
  if($t == $reftime || !strcmp($t, $reftime)){
    $head .=  "<option selected value='$t'>$t\n";
  }else{
    $head .=  "<option value='$t'>$t\n";
  }
}
$head .=  "</select>\n";

echo $head;

foreach($arserver as $name){
	$html ='';
  if($srvon[$name]){
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
			
			#   Get Runinof
      $st = getruninfo($server);
      $ft = ftruninfo();
      $runinfo = unpack($ft, $st);
			
      $startdate = date("d-M-y H:i:s", $runinfo['starttime']);
      $stopdate = date("d-M-y H:i:s", $runinfo['stoptime']);
			
      $html .=  "<table valign=top>\n";
			
			#   DAQINFO
      $html .=  "<tr valign=top><td>";
      $html .=  "<b>EB Information</b><br>\n";
      $of = $ofstr[$daqinfo['babildes']];
      $html .=  "<table><tr><td bgcolor='#33ffff'>EF Number<td>$daqinfo[efn]</tr>\n";
      $kb = (int)($daqinfo['ebsize']*2/1024);
      $html .=  "<tr><td bgcolor='#33ffff'>Event Build Size<td>$daqinfo[ebsize] ($kb kB)</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Babildes mode<td>$of</tr>\n";
      $html .=  "</table><br>\n";

			
			#   RUNINFO
      $html .=  "<td>";
      $html .=  "<b>RUN Information</b><br>\n";
      $run = $runstr[$runinfo['runstat']];
      $html .=  "<table><tr><td bgcolor='#33ffff'>Run name<td>$daqinfo[runname]</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Run number<td>$daqinfo[runnumber]</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Run status<td>$run</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Start date<td>$startdate</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Stop date<td>$stopdate</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Header<td>$runinfo[header]</tr>\n";
      $html .=  "<tr><td bgcolor='#33ffff'>Ender<td>$runinfo[ender]</tr>\n";
      $html .=  "</table>\n";

			
			#   EFLIST
      $html .=  "</tr><tr valign=top><td>";
      $html .=  "<b>Event fragment</b><br>\n";
      $html .=  "<table><tr bgcolor='#33ffff'><td>ID<td>Hostname<td>Nichname<td>on/off</tr>\n";
      for($i=0;$i<256;$i++){
				$exi = "efex$i";
				$namei = "efname$i";
				$hosti = "efhost$i";
				$ofi   = "efof$i";
				$of    = $ofstr[$daqinfo[$ofi]];
				if($daqinfo[$exi]){
					$html .=  "<tr><td>$i<td>$daqinfo[$hosti]<td>$daqinfo[$namei]<td>$of</tr>\n";
				}
      }
      $html .=  "</table><br>\n";
			
			#   HDLIST
      $html .=  "<td>";
      $html .=  "<b>HD list</b><br>\n";
      $html .=  "<table><tr bgcolor='#33ffff'><td>ID<td>Path<td>on/off</tr>\n";
      for($i=0;$i<10;$i++){
				$exi   = "hdex$i";
				$pathi = "hdpath$i";
				$ofi   = "hdof$i";
				$of    = $ofstr[$daqinfo[$ofi]];
				$freeai= "hdfreea$i";
				$freebi= "hdfreeb$i";
				if($daqinfo[$exi]){
					$html .=  "<tr><td>$i<td>$daqinfo[$pathi]<td>$of</tr>\n";
				}
      }
      $html .=  "</table><br>\n";
		
      $html .=  "</tr></table>\n";

      $st = getscrlist($server);
			#      echo "hogehoge";
      if($st){
				#   Get scrlist
				$ft = "iscranan/";
				for($i=0;$i<MAXSCRANA;$i++){
					$ft .= ftscrlist($i);
					$ft .= "/";
				}
				$scrlist = unpack($ft, $st);
				
				$scrlivest = getscrlive($server);
				$ft = ftscrlive();
				$scrlive = unpack($ft, $scrlivest);
				$gn = $scrlive['gatedid'];
				$gc = $scrlive['gatedch'];
				$ungn = $scrlive['ungatedid'];
				$ungc = $scrlive['ungatedch'];
				#   Sort scr id
				$bbst = array();
				for($i=0;$i<$scrlist['scranan'];$i++){
					$scridi  = "scrid$i";
					$tn = $scrlist[$scridi];
					$ix = 0;
					for($j=0;$j<$scrlist['scranan'];$j++){
						$scridj = "scrid$j";
						if($i!=$j && $tn > $scrlist[$scridj]){
							$ix++;
						}
					}
					$bbst[$ix] = $i;
				}
				#   SCRLIST
				#	$html .=  "<b>Scaler List</b><br>\n";
				#	$html .=  "<table><tr bgcolor='#33ffff'><td>ID<td>SCRN<td>NAME</tr>\n";
				#	for($i=0;$i<$scrlist[scranan];$i++){
				#	  $scridi  = "scrid$bbst[$i]";
				#	  $scrni   = "scrn$bbst[$i]";
				#	  $idnamei = "idname$bbst[$i]";
				#	  $html .=  "<tr><td>$scrlist[$scridi]<td>$scrlist[$scrni]<td>$scrlist[$idnamei]</tr>\n";
				#	}
				#	$html .=  "</table>\n";
				#	$html .=  "<br><br>\n";
				
				#   SCRDATA
				$ov    = 4294967296;
				
				$html .=  "<table valign=top>\n";
				$vi=0;
				for($i=0;$i<$scrlist['scranan'];$i++){
					$scridi  = "scrid$bbst[$i]";
					$scrni   = "scrn$bbst[$i]";
					$idnamei = "idname$bbst[$i]";
					$id = $scrlist[$scridi];
					
					if(!isset($on[$name][$id])){
						continue;
					}else{
						if($on[$name][$id] == 0){
							continue;
						}
					}
					
					if($vi%2==0) $html .=  "<tr valign=top>\n";
					$html .=  "<td>\n";
					$html .=  "<b>Scaler Data ID=$id ($scrlist[$idnamei])</b><br>\n";
					$st = getscrdata($server, $id);
					#   SCRDATA Format
					$ft = "itid/iclassid/iscrid/iscrn/iratech/irate/a80idname/";
					for($j=0;$j<$scrlist[$scrni];$j++){
						$ft .= ftscrcont($j);
						$ft .= "/";
					}
					$scrdata = unpack($ft, $st);
					$html .=  "<table><tr bgcolor='#ffcccc'><td colspan=2>ID<td colspan=2>Current<td colspan=2>Total</tr>\n";
					
					$ratech = $scrdata['ratech'];
					$rate = $scrdata['rate'];
					$clkn[$id] = $ratech;
					$clkr[$id] = $rate;
					#	echo "$id ratech $ratech / rate $rate";
					$j = $clkn[$id];
					$curi  = "cur$j";
					$totai = "tota$j";
					$totbi = "totb$j";
					
					if($clkr[$id] > 0){
						$clkt = ($scrdata[$totai] + $scrdata[$totbi] * $ov) / $clkr[$id] * 1000.;
						$clkc = $scrdata[$curi] / $clkr[$id] * 1000.;
					}else{
						$clkt = 0;
						$clkc = 0;
					}
					
					for($j=0;$j<$scrlist[$scrni];$j++){
						$curi  = "cur$j";
						$totai = "tota$j";
						$totbi = "totb$j";
						$dmi   = "dm$j";
						$namei = "name$j";
						$total = $scrdata[$totai] + $scrdata[$totbi] * $ov;
						$cur   = $scrdata[$curi];
						$mf = 0;
						$km = 'k';
						if($clkt != 0){
							if($total / $clkt > 1000.){
								$mf = 1;
								$km = 'M';
							}
							$totalr = sprintf("%7.2f", $total / $clkt);
							if($mf == 1){
								$totalr = sprintf("%7.2f", $total / $clkt / 1000.);
							}
						}else{
							$totalr = 0;
						}
						if($clkc != 0){
							$curr =   sprintf("%7.2f", $cur / $clkc);
							if($mf == 1){
								$curr = sprintf("%7.2f", $cur / $clkc / 1000.);
							}
						}else{
							$curr = 0;
						}
						
						
						if($ungn == $id && $ungc == $j){
							$curung = $cur;
							$totalung = $total;
						}
						if($gn == $id && $gc == $j){
							$curg = $cur;
							$totalg = $total;
						}
						
						$html .=  "<tr><td align=right>$j<td>($scrdata[$namei])<td align=right>$cur<td align=right>($curr $km)<td align=right>$total<td align=right>($totalr $km)</tr>\n";
					}
					$html .=  "</table>\n";
					$html .=  "<br>\n";
					if(($vi+1)%2 == 0) $html .=  "</tr>\n";
					$vi++;
					
				}
				$html .=  "</table>\n";
      }
    }
  }
	

	if($curung != 0){
		$livecur = $curg*100./$curung;
	}else{
		$livecur = 0.;
	}
	if($totalung != 0){
		$livetot = $totalg*100./$totalung;
	}else{
		$livetot = 0.;
	}
	$rate = "<font color=blue><b>";
	$rate .= sprintf("Live time = %4.2f%% (total = %4.2f%%)<br>\n", $livecur, $livetot);
	$rate .= "</b></font><br>";
	$rate .= "Gated Id=$gn Ch=$gc / Ungated Id=$ungn Ch=$ungc<br><br>";
	
	$out = $rate.$html;
	echo $out;
}
echo "<hr width=500 align=left>\n";
echo "<button type='submit' name='refresh'>Refresh</button>\n";
echo "</form>";
	

?></p>


</body>
</html>
