<?php

require_once('class/BBHtml.class.php');
require_once('class/Spyc.php');
require_once('class/SelGTO.class.php');
require_once('class/LUGTO.class.php');
require_once('class/LUGTO2.class.php');

$tstart = microtime(true);



function issetset($d, $v, $s1, $s2=null){
	if($s2){
		if(isset($v[$s1][$s2])){
			return $v[$s1][$s2];
		}
	}else{
		if(isset($v[$s1])){
			return $v[$s1];
		}
	}
	return $d;
}

function isseterr($v, $s, $com){
	if(isset($v[$s])){
		return $v[$s];
	}

	return null;
}


function chkoutdata($g, $o, $ip){
	$i = preg_replace('/out/', '', $o);

	if($g->chkout($i, $g->idxseladv($ip))){
		return 1;
	}

	return 0;
}

$html = new BBHtml();
$thisfile = basename($_SERVER['PHP_SELF']);


###########
$title = 'GTO Trigger';
$tx = 5;
$ty = 5;

##########

$yaml = Spyc::YAMLLoad('dat/brtrigger.yml');

$page = array();
$gto  = array();
$cel  = array();


# analyze yaml array
if(isset($yaml['page'])){
	$title = issetset($title, $yaml, 'page', 'title');
	$tx = issetset($tx, $yaml, 'page', 'x');
	$ty = issetset($ty, $yaml, 'page', 'y');
}
foreach($yaml as $ym){
	if(isset($ym['cel'])){
		$x = issetset(0, $ym, 'cel', 'x');
		$y = issetset(0, $ym, 'cel', 'y');
		$cel[$x][$y] = $ym['cel'];
	}
	if(isset($ym['gto'])){
		$gto[$ym['gto']['name']] = $ym['gto'];
	}
}

$html->std_html_header($title);
$html->heading(2, $title);
$html->hr();


### check post ####
$fsave = 0;
if(isset($_POST)){
	if(isset($_POST['save'])){
		$fsave = 1;
	}else if(isset($_POST['eepwrite'])){
		foreach($gto as $gg){
			$g = new $gg['class']($gg['host']);
			$g->open();
			$g->eepwrite();
			$data = $g->read();
			$g->close();
			printf("WWPWrite Done<font color=%s>%s</font><br>\n",
						 issetset('#ffffff', $gg, 'color'), $gg['name']);
		}
	}else if(isset($_POST['eepread'])){
		foreach($gto as $gg){
			$g = new $gg['class']($gg['host']);
			$g->open();
			$g->eepread();
			$data = $g->read();
			$g->close();
			printf("WWPWrite Done<font color=%s>%s</font><br>\n",
						 issetset('#ffffff', $gg, 'color'), $gg['name']);
		}
	}
}

####
$modar = array();
if($fsave){
	$modn = $html->modify_check($_POST);
	foreach($modn as $n){
		$sk = "/=".$n."/";
		foreach($_POST as $k => $v){
			if(preg_match($sk, $k)){
				$kk = str_replace('='.$n, '', $k);
				//echo 'hoge'.$kk."<br>\n";
				$kr = explode(':', $kk);
				if(isset($modar[$kr[0]][$kr[1]])){
					$modar[$kr[0]][$kr[1]] .= ":".$v;
				}else{
					$modar[$kr[0]][$kr[1]] = $v;
				}
			}
		}
	}

	// 
	//print_r($modar);

	/// data to gto
	foreach($modar as $gg => $dt){
		$g = new $gto[$gg]['class']($gto[$gg]['host']);
		$ret = $g->open();
		if(strlen($ret)){
			echo $ret;
			continue;
		}

		$foutmem = array();
		$foutch = array();
		// set output
		foreach($dt as $k => $d){
			if(preg_match('/out/', $d)){
				$foutch[$k] = $d;
				// fanout
				continue;
			}
			$chs = explode(":", $d);
			//echo "key=".$k." d=".$d."<br>\n";
			if(preg_match('/out/', $k) ||
				 preg_match('/dssel/', $k) || 
				 preg_match('/dsrate/', $k)) {
				/*  for readout + setting
						$tchs = $chs;
						foreach($tchs as $x){
						if(preg_match('/out/', $x)){
						$data = $g->read();
						$g->decode($data);
						$tn = intval(str_replace('out','',$x), 0);
						$dar = $g->dataparam($tn);
						
						$kk = array_search($x, $chs);
						unset($chs[$kk]);
						$chs = array_merge($chs, $dar);
						}
					}
				*/

				if(preg_match('/out/', $k)){
					$n = intval(str_replace('out','',$k), 0);
					$s = $g->strparam($chs);
					if(!strlen($s)){
						$s = 'none';
					}
					$g->selout($n, $chs);
				}else if(preg_match('/dssel/', $k)){
					$n = intval(str_replace('dssel','',$k), 0);
					$s = $g->strparam($chs);
					if(!strlen($s)){
						$s = 'none';
					}
					$g->dssel($n, $d);
				}else if(preg_match('/dsrate/', $k)){
					$n = intval(str_replace('dsrate','',$k), 0);
					$s = $d;
					$d = intval($d, 0);
					$g->dsrate($n, $d);
				}
					
					
				$foutmem[$k] = $chs;
				printf("Write <font color=%s>%s</font> %s %s<br>\n",
							 issetset('#ffffff', $gto[$gg], 'color'), $gg, $k, $s);
					//printf("DB %d %016x<br>\n", $n, $g->encout($chs));
			}
		}

		// for fanout
		foreach($foutch as $k => $d){
			if(isset($foutmem[$d])){
				$n = intval(str_replace('out','',$k), 0);
				$s = $g->strparam($foutmem[$d]);
				$g->selout($n, $foutmem[$d]);
				printf("Write <font color=%s>%s</font> %s %s (fanout of %s)<br>\n",
							 issetset('#ffffff', $gto[$gg], 'color'), $gg, $k, $s, $d);
			}
		}

		$g->close();
	}

}



### read data from gto ###
foreach($gto as $gg){
	$g = new $gg['class']($gg['host']);
	$ret = $g->open();
	if(strlen($ret)){
		echo $ret;
	}
	$data = $g->read();
	$g->decode($data);
	$g->close();
	$gto[$gg['name']]['obj'] = $g;
}




$html->hr();
$html->std_form_header($thisfile);
echo "<br>\n";
echo $html->button_generic('EEPWrite');
echo $html->button_generic('EEPRead');
$id = 0;
$html->table_start('#eeeeee', "border=0 cellspacing=3 id='modtable'");
echo "<tr>\n";
for($x=0;$x<$tx;$x++){
	echo "<td>\n";
	$html->table_start('', 'border=0 cellspacing=3');
	for($y=0;$y<$ty;$y++){
		echo "<tr>\n";
		if(isset($cel[$x][$y])){
			echo "<td bgcolor=''>\n";
			$v = $cel[$x][$y];
			//print_r($v);

			$c = issetset('', $v, 'color');
			$html->table_start($c);
			echo "<tr><td colspan=3 bgcolor='#dddddd' align='center'>";
			echo "<b>".$v['name']."</b><br>";
			$fc = issetset('#000000', $gto[$v['gto']], 'color');
			echo "<font color='".$fc."'>".$v['gto']."</font>\n";

			$g = isseterr($v, 'gto', 'cel='.$v['name']);
			if(!$g) continue;

			$lid=0;
			if(isset($gto[$v['gto']]['output'][$v['name']])){
				$op = $gto[$v['gto']]['output'][$v['name']];
			}else{
				echo "\n<tr><td><td>";
				$html->errormsg("Error: output ".$v['name']." is not in ".$v['gto']);
				continue;
			}
			$lg = issetset('or', $v, 'logic');
			$sig = issetset('sig', $v, 'signal');

			if(isset($v['radio'])){
				echo $html->hidden_id($v['gto'].":".$op.":".$lid."=", $lg, $id);
				$lid++;
				echo $html->hidden_id($v['gto'].":".$op.":".$sig."=", $sig, $id);
				$lid++;
				foreach($v['radio'] as $r){
					if(isset($gto[$v['gto']]['input'][$r])){
						$ip = $gto[$v['gto']]['input'][$r];
					}else{
						echo "\n<tr><td><td>";
						$html->errormsg("Error: input ".$r." is not in ".$v['gto']);
						continue;
					}
					$n = intval(str_replace('out','',$op), 0);
					$chk = chkoutdata($gto[$v['gto']]['obj'], $op, $ip);
					if(!strcmp($ip, 'none') && !$gto[$v['gto']]['obj']->out[$n]){
						$chk = 1;
					}
					echo "\n<tr>";
					echo $html->radio_tdid($v['gto'].":".$op.":".$lid."=", $chk, $id, $r, '', $ip);
					echo "<td><font size='-1' color='".$fc."'>(".$ip.")</font>\n";
				}
			}
			if(isset($v['check'])){
				echo "<tr>";
				if(!strcmp($lg, 'and')){
					echo "<td>&loz;<td><I>AND</I>";
				}else{
					echo "<td>&loz;<td><I>OR</I>";
				}
				echo $html->hidden_id($v['gto'].":".$op.":".$lid."=", $lg, $id);
				$lid++;
				echo $html->hidden_id($v['gto'].":".$op.":".$sig."=", $sig, $id);
				$lid++;
				foreach($v['check'] as $vc){
					if(isset($gto[$v['gto']]['input'][$vc])){
						$ip = $gto[$v['gto']]['input'][$vc];
					}else{
						echo "\n<tr><td><td>";
						$html->errormsg("Error: input ".$vc." is not in ".$v['gto']);
						continue;
					}
					$chk = chkoutdata($gto[$v['gto']]['obj'], $op, $ip);
					echo "\n<tr>";

					echo $html->checkbox_tdid($v['gto'].":".$op.":".$lid."=", $chk, $id, $ip);
					echo "<td>".$vc."\n";
					echo "<td><font size='-1' color='".$fc."'>(".$ip.")</font>\n";
						$lid++;
				}
			}
			if(isset($v['fanout'])){
				foreach($v['fanout'] as $f){
					$top = $gto[$v['gto']]['output'][$f];
					echo $html->hidden_id($v['gto'].":".$top.":".$lid."=", $op, $id);
				}
				$lid++;
			}
			if(isset($v['downscale'])){
				if(isset($gto[$v['gto']]['input'][$v['downscale']])){
					$ip = $gto[$v['gto']]['input'][$v['downscale']];
					$n = intval(str_replace('out','',$op), 0);
					$rate = $gto[$v['gto']]['obj']->rate[$n];
					$dip = "ds".$n;
					echo "<tr><td>DS\n";
					echo $html->hidden_id($v['gto'].":".$op.":".$lid."=", $dip, $id);
					echo $html->hidden_id($v['gto'].":dssel".$n.":".$lid."=", $ip, $id);
					echo $html->number_tdid($v['gto'].":dsrate".$n.":".$lid."=", $rate,
																	"0:16777215", $id, 8);
					echo "<td><font size='-1' color='".$fc."'>(".$ip.")</font>\n";
					$lid++;
				}else{
					echo "\n<tr><td><td>";
					$html->errormsg("Error: input ".$r." is not in ".$v['gto']);
					continue;
				}
			}
			if(isset($v['ludownscale'])){
				if(isset($gto[$v['gto']]['input'][$v['ludownscale']])){
					$ip = $gto[$v['gto']]['output'][$v['ludownscale']];
					$lo = str_replace('out','lo',$ip);
					#$n = intval(str_replace('out','',$op), 0);
					$n = 0;
					$rate = $gto[$v['gto']]['obj']->rate[$n];
					$dip = "ds".$n;
					echo "<tr><td>DS\n";
					echo $html->hidden_id($v['gto'].":".$op.":".$lid."=", $dip, $id);
					echo $html->hidden_id($v['gto'].":dssel".$n.":".$lid."=", $lo, $id);
					echo $html->number_tdid($v['gto'].":dsrate".$n.":".$lid."=", $rate,
																	"0:16777215", $id, 8);
					echo "<td><font size='-1' color='".$fc."'>(".$lo.")</font>\n";
					$lid++;
				}else{
					echo "\n<tr><td><td>";
					$html->errormsg("Error: input ".$r." is not in ".$v['gto']);
					continue;
				}
			}

			$id++;

			echo "<tr><td>";
			echo "<td align='left'>&rArr; <font color='".$fc."'>";
			echo $op;
			echo "</font>\n";

			if(isset($v['fanout'])){
				foreach($v['fanout'] as $f){
					$top = $gto[$v['gto']]['output'][$f];
					echo "<tr><td>";
					echo "<td align='left'>&rArr; <font color='".$fc."'>";
					echo $top;
					echo "</font>\n";
				}
				$lid++;
			}

			$html->table_end();

		}else{
			echo "<td>\n";
		}
	}
	$html->table_end();
}
$html->table_end();
//print_r($gto);
$html->input_modify($id);

$html->std_form_ender();
echo $html->button_generic('EEPWrite');
echo $html->button_generic('EEPRead');

$tstop = microtime(true) - $tstart;

echo "<br><br>Transaction Time=".sprintf("%10.3f", $tstop)." ms<br>\n";

$html->std_html_ender();

?>
