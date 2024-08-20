<?php

class CModule {
  public $name = 'Generic';
  public $func = 'Function';

  function setName($name){
    if(strlen($name)){
      $this->name = $name;
    }
  }

  function setFunc($func){
    if(strlen($func)){
      $this->func = $func;
    }
  }

  function echoJScript(){
    echo "<script type='text/javascript' language='javascript'>\n";
    echo "function js_setVal(cl, val){\n";
    echo "  var el = document.getElementsByClassName(cl);\n";
    echo "  for(var i=0; i<el.length; i++){\n";
    echo "    el[i].value=val;\n";
    echo "  }\n";
    echo "}\n";
    echo "</script>\n";
  }

  function show_char8_dot($v){
    $v = intval($v, 0);
    $v1 = ($v & 0x00f0) >> 4;
    $v2 = ($v & 0x000f);
    return sprintf("%d.%d", $v1, $v2);
  }

  function show_char16_dot($v){
    $v = intval($v, 0);
    $v1 = ($v & 0xff00) >> 8;
    $v2 = ($v & 0x00ff);
    return sprintf("%d.%d", $v1, $v2);
  }

  function show_bits_desc($v, $bt){
    $ret = '';
    $x = 0;
    foreach($bt as $k => $b){
      if($x) $ret .= "<br>";
      $f = (intval($v, 0) >> $k) & 0x1;
      $ret .= $b[1+$f]."\n";
      $x++;
    }
    return $ret;
  }

  function show_bits_checkbox($v, $bt){
    $ret = '';
    $x = 0;
    foreach($bt as $k => $b){
      if($x) $ret .= "<br>";
      $f = (intval($v, 0) >> $k) & 0x1;
      $ret .= "<input type='checkbox' name='";
      $ret .= $b[0]."' value=".$k;
      if($f == 1){
	$ret .= " checked='checked'";
      }
      $ret .= ">".$b[0]."\n";
      $x++;
    }
    return $ret;
  }

  function show_val_desc($vt){
    $ret = '';
    $x = 0;
    foreach($vt as $b){
      if($x) $ret .= "<br>";
      $ret .= $b."\n";
      $x++;
    }
    return $ret;
  }

	function show_enabits_radiobox($v, $vt, $n){
		$ret = "<table>\n";

		foreach($vt as $k => $b){
			$bt = ($v >> intval($b[2], 0)) & 0x01;
			if($bt == intval($b[3], 0)){
				$chk1 = " checked='checked' ";
				$chk2 = " ";
			}else{
				$chk1 = " ";
				$chk2 = " checked='checked' ";
			}
			$ret .= "<tr><td>";
			$ret .= $b[0];
			$ret .= "<td><input type='radio' name='".$n.":".$b[0]."'";
			$ret .= " value=".$b[4].":".$b[5].$chk1.">".$b[1];
			$ret .= "<td><input type='radio' name='".$n.":".$b[0]."'";
			$ret .= " value=".$b[9].":".$b[10].$chk2.">".$b[6]."\n";
		}
		$ret .= "</table>\n";

		return $ret;
	}


  function show_val_radiobox($v, $vt, $n){
    $ret = '';
    $x = 0;
    foreach($vt as $k => $b){
      if($x) $ret .= "<br>";
      if(intval($v, 0) == intval($k, 0)){
				$f = 1;
      }else{
				$f = 0;
      }
      $ret .= "<input type='radio' name='";
      $ret .= $n."' value=".$k;
      if($f == 1){
				$ret .= " checked='checked'";
      }
      $ret .= ">".$b."\n";
      $x++;
    }
    return $ret;
  }

  function show_opqn_radiobox($v, $vt, $n){
    $ret = '';
    $x = 0;
    foreach($vt as $k => $b){
      if($x) $ret .= "<br>";
      if(intval($v, 0) == intval($k, 0)){
	$f = 1;
      }else{
	$f = 0;
      }
      $ret .= "<input type='radio' name='";
      $ret .= $n."' value=".$b[1];
      if($f == 1){
	$ret .= " checked='checked'";
      }
      $ret .= ">".$b[0]."\n";
      $x++;
    }
    return $ret;
  }


  function edit_number($n, $v, $r, $id=null){
    $ret = array();
    $m = explode(":", $r);
    $ret[0] = "<input type='number' name='".$n."' ";
    $ret[0] .= "value=".$v." min=".$m[0]." max=".$m[1];
    $ret[0] .= " ".$id." >\n";
    $ret[1] = $v;

    return $ret;
  }

  function edit_hex($n, $v, $r){
    $ret = array();
    $ret[0] = "0x<input type='text' name='".$n."' ";
    $ret[0] .= "value=".sprintf("%02x", $v);
    $ret[0] .= " maxlength=".$r." size=4>\n";
    $ret[1] = sprintf("0x%02x", $v);
    return $ret;
  }

  function get_bitsval($v, $bv){

    $f = (intval($v, 0) >> $bv) & 0x1;

    return $f;
  }


}


class CCAMAC extends CModule {
  public $c=-1,$n=-1;

  function setCN($c, $n){
    $this->c = $c;
    $this->n = $n;
  }
}

class CVME extends CModule {
  const A32 = '0x09';
  const A24 = '0x39';
  const A16 = '0x29';
  const D16R = '-wr';
  const D32R = '-lr';
  const D16W = '-ww';
  const D32W = '-lw';
  const AMSR = '-am';
  const OPR  = '-qr';
  const OPW  = '-qw';
  const OPN  = '-qn';
  const COM  = '/usr/nbbq/bin/cmdvme';

  const COL_FWAD = '#ffffcc';
  const COL_COMT = '#eeeebb';
  const COL_BACK = '#cccc99';
  const COL_CAEN    = '#ff0020';
  const COL_MESYTEC = '#eeee00';

  public $base=-1;
  public $readparam = array();
  public $readdata = array();
  public $showparam = array();
  public $writeparam = array();
  public $reg = array();

  public $fcolor = self::COL_FAWD;
  public $ccolor = self::COL_COMT;
  public $bcolor = self::COL_BACK;

  function setBaseAddress($base){
    $this->base = $base;
  }

  function getBaseAddress(){
    return $this->base;
  }

  function chkReadMode($com){
    if(!strcmp($com['mode'],'d16')){
      return self::D16R;
    }else if(!strcmp($com['mode'],'d32')){
      return self::D32R;
    }else if(!strcmp($com['mode'],'op')){
      return self::OPR;
    }

    return self::D16R;
  }

  function chkWriteMode($com){
    if(!strcmp($com['mode'],'d16')){
      return self::D16W;
    }else if(!strcmp($com['mode'],'d32')){
      return self::D32W;
    }else if(!strcmp($com['mode'],'op')){
      if(isset($com['opqw'])){
	return self::OPW;
      }else{
	return self::OPN;
      }
    }

    return self::D16W;
  }

  function chAddr($com, $ch){
    $adr = intval($com['addr'], 0);
    if(!strcmp($com['mode'],'d16')){
      if(isset($com['step'])){
	$adr = intval($com['addr'],0) + intval($com['step'],0) * $ch;
      }else{
	$adr = intval($com['addr'],0) + 0x02 * $ch;
      }
    }
    if(!strcmp($com['mode'],'d32')){
      if(isset($com['step'])){
	$adr = intval($com['addr'],0) + intval($com['step'],0) * $ch;
      }else{
	$adr = intval($com['addr'],0) + 0x04 * $ch;
      }
    }
    return $adr;
  }

  function vmeAM($am){
    $ret = sprintf("%s %s %s", self::COM, self::AMSR, $am);
    return $ret;
  }

  function vmeRead($com, $ch=null){
    $ret = '';
    $mod = $this->chkReadMode($com);

    if(isset($com['addr'])){
      $adr = intval($com['addr'], 0);
      if(isset($ch)){
	$adr = $this->chAddr($com, $ch);
      }
      $radr = intval($adr, 0) + intval($this->base, 0);

      $ret = sprintf("%s %s 0x%08x", self::COM, $mod, $radr);
    }else if(isset($com['opra'])){
      $ret = sprintf("%s %s 0x%08x %s %s", self::COM, $mod, intval($this->base, 0),
		     $com['opra'], $com['oprn']);
    }

    return $ret;
  }

  function vmeWrite($com, $val, $ch=null){
    $ret = '';
    $mod = $this->chkWriteMode($com);

    if(isset($com['addr'])){
      $adr = intval($com['addr'], 0);
      if(isset($ch)){
				$adr = $this->chAddr($com, $ch);
      }
      
      $radr = $adr + intval($this->base, 0);
			
      $val = intval($val, 0);
      $ret = sprintf("%s %s 0x%08x 0x%04x", self::COM, $mod, $radr, $val);
    }else if(isset($com['opqn'])){
      $ret = sprintf("%s %s 0x%08x %s", self::COM, $mod, 
		     intval($this->base, 0), $val);
    }else if(isset($com['dfqn'])){
      $ret = sprintf("%s %s 0x%08x %s", self::COM, $mod, 
		     intval($this->base, 0), $val);
    }else if(isset($com['opqw'])){
      $ret = sprintf("%s %s 0x%08x %s %d", self::COM, $mod, 
		     intval($this->base, 0), $com['opqw'], $val);
    }
				  
    return $ret;
  }


  function getReadCommand(){
    $ret = array();
    foreach($this->readparam as $rd){
      $com = $this->reg{$rd};
      if(isset($com['nch'])){
	for($i=0;$i<$com['nch'];$i++){
	  $vme = $this->vmeRead($com, $i);
	  array_push($ret, $vme);
	}
      }else{
	$vme = $this->vmeRead($com);
	array_push($ret, $vme);
      }
    }
    return $ret;
  }

  function setReadData($data){
    unset($this->readdata);
    $i = 0;
    foreach ($this->readparam as $rd){
      if(isset($this->reg[$rd]['nch'])){
	for($j=0;$j<$this->reg[$rd]['nch'];$j++){
	  $this->readdata[$rd][$j] = $data[$i][0];
	  $i++;
	}
      }else if(isset($this->reg[$rd]['opqr'])){
	for($j=0;$j<$this->reg[$rd]['oprn'];$j++){
	  $wp = $this->{$this->reg[$rd]['opqr']};
	  $this->readdata[$wp[$j]] = $data[$i][0];
	  $i++;
	}
      }else{
	$this->readdata[$rd] = $data[$i][0];
	$i++;
      }
    }
  }

  function dbprtord($dbpr){
    $ret = array();
    foreach ($dbpr as $p){
      $ret[$p['Function']] = $p['Value'];
    }
    return $ret;
  }

  function setOfflineData($dbrd=null){
    unset($this->readdata);
    $i = 0;
    foreach ($this->readparam as $rd){
      if(isset($this->reg[$rd]['nch'])){
				for($j=0;$j<$this->reg[$rd]['nch'];$j++){
					$p = $rd."-".$j;
					if(isset($dbrd[$p])){
						$w = $dbrd[$p];
					}else{
						if(isset($this->reg[$rd]['offv'])){
							$w = intval($this->reg[$rd]['offv'], 0);
						}else{
							$w = 0;
						}
					}
					$this->readdata[$rd][$j] = $w;
					$i++;
				}
      }else if(isset($this->reg[$rd]['opqr'])){
				for($j=0;$j<$this->reg[$rd]['oprn'];$j++){
					$wp = $this->{$this->reg[$rd]['opqr']};
					if(isset($dbrd[$wp[$j]])){
						$w = $dbrd[$wp[$j]];
					}else{
						if(isset($this->reg[$wp[$j]]['offv'])){
							$w = intval($this->reg[$wp[$j]]['offv'], 0);
						}else{
							$w = 0;
						}
					}
					$this->readdata[$wp[$j]] = $w;
					$i++;
				}
      }else{
				if(isset($dbrd[$rd])){
					if(isset($this->reg[$rd]['enabits'])){
						$xx = intval($dbrd[$rd]);
						$tx = 0;
						foreach($this->{$this->reg[$rd]['enabits']} as $cm){
							$cx = intval($cm[5], 0) << intval($cm[4], 0);
							if($xx & $cx){
								$tx |= intval($cm[3], 0) << intval($cm[2], 0);
							}else{
								$tx |= intval($cm[8], 0) << intval($cm[7], 0);
							}
						}
						$w = $tx;
					}else{
						$w = $dbrd[$rd];
					}
				}else{
					if(isset($this->reg[$rd]['offv'])){
						$w = intval($this->reg[$rd]['offv'], 0);
					}else{
						$w = 0;
					}
				}
				$this->readdata[$rd] = $w;
				$i++;
      }
    }
  }

  function getWriteValue($com, $wd, $ps, $ch=-1){
    if(isset($com['defw'])){
      $w = $com['defw'];
      $w = intval($w, 0);
    }else{
      if(isset($com['bits'])){
				$w = 0;
				foreach($this->{$com['bits']} as $k => $v){
					if(isset($ps[$v[0]])){
						$w = $w | (0x01 << intval($k, 0));
					}
				}
			}else if(isset($com['enabits'])){
				$w = 0;
				foreach($this->{$com['enabits']} as $k => $v){
					$tk = $wd.":".$v[0];
					if(isset($ps[$tk])){
						list($bt, $vt) = explode(":", $ps[$tk]);
						$w = $w | (intval($vt) << intval($bt));
					}
				}
      }else{
				if($ch==-1){
					$w = $ps[$wd];
				}else{
					$wd = $wd."-".$ch;
					$w = $ps[$wd];
				}
				
				if(isset($com['edit'])){
					if(!strcmp($com['edit'], 'edit_hex')){
						if(!preg_match('/0x/', $w)){
							$w = '0x'.$w;
						}
					}
				}else{
					$w = intval($w, 0);
				}
      }
    }
		
    return $w;
  }

  function getWriteValueDB($com, $wd, $ps, $ch=-1){
    if(isset($com['defw'])){
      $w = $com['defw'];
      $w = intval($w, 0);
    }else{
      if(isset($com['bits'])){
	$w = $ps[$wd];
      }else{
	if($ch==-1){
	  $w = $ps[$wd];
	}else{
	  $wd = $wd."-".$ch;
	  $w = $ps[$wd];
	}
	
	if(isset($com['edit'])){
	  if(!strcmp($com['edit'], 'edit_hex')){
	    if(!preg_match('/0x/', $w)){
	      $w = '0x'.$w;
	    }
	  }
	}else{
	  $w = intval($w, 0);
	}
      }
    }

    return $w;
  }


  function getWriteCommand($ps){
    $ret = array();
    foreach($this->writeparam as $wd){
      if(!strcmp('sleep0', $wd)){
				array_push($ret, '/bin/sleep 0');
      }else if(!strcmp('sleep1', $wd)){
				array_push($ret, '/bin/sleep 1');
      }else{
				$com = $this->reg{$wd};
				if(isset($com['nch'])){
					for($i=0;$i<$com['nch'];$i++){
						$w = $this->getWriteValue($com, $wd, $ps, $i);
						$vme = $this->vmeWrite($com, $w, $i);
						array_push($ret, $vme);
					}
				}else if(isset($com['opqn'])){
					$vme = $this->vmeWrite($com, $ps[$wd]);
					array_push($ret, $vme);
				}else if(isset($com['dfqn'])){
					$vme = $this->vmeWrite($com, $com['dfqn']);
					array_push($ret, $vme);
				}else{
					$w = $this->getWriteValue($com, $wd, $ps);
					$vme = $this->vmeWrite($com, $w);
					array_push($ret, $vme);
				}
      }
    }
    return $ret;
  }

  function getWriteCommandDB($ps){
    $ret = array();
    foreach($this->writeparam as $wd){
      if(!strcmp('sleep0', $wd)){
				array_push($ret, '/bin/sleep 0');
      }else if(!strcmp('sleep1', $wd)){
				array_push($ret, '/bin/sleep 1');
      }else{
				$com = $this->reg{$wd};
				if(isset($com['nch'])){
					for($i=0;$i<$com['nch'];$i++){
						$w = $this->getWriteValueDB($com, $wd, $ps, $i);
						$vme = $this->vmeWrite($com, $w, $i);
						array_push($ret, $vme);
					}
				}else if(isset($com['opqn'])){
					$w = $this->{$com['opqn']}[$ps[$wd]][1];
					$vme = $this->vmeWrite($com, $w);
					array_push($ret, $vme);
				}else if(isset($com['dfqn'])){
					$vme = $this->vmeWrite($com, $com['dfqn']);
					array_push($ret, $vme);
				}else{
					$w = $this->getWriteValueDB($com, $wd, $ps);
					$vme = $this->vmeWrite($com, $w);
					array_push($ret, $vme);
				}
      }
    }
    return $ret;
  }


  function getDBCommand($ps){
    $ret = array();
    foreach($this->writeparam as $wd){
      if(!strcmp('sleep0', $wd)){
				// noop
      }else if(!strcmp('sleep1', $wd)){
				// noop
      }else{
				$com = $this->reg{$wd};
				if(isset($com['nch'])){
					for($i=0;$i<$com['nch'];$i++){
						$w = $this->getWriteValue($com, $wd, $ps, $i);
						$v = array();
						$v['Function'] = $wd."-".$i;;
						$v['Sub'] = '';
						$v['Value'] = $w;
						$v['Reserve'] = '';
						array_push($ret, $v);
					}
				}else if(isset($com['opqn'])){
					$v = array();
					$x = 0;
					foreach ($this->{$com['opqn']} as $k => $tn){
						if(!strcmp($tn[1], $ps[$wd])){
							$x = $k;
						}
					}
					
					$v['Function'] = $wd;
					$v['Sub'] = '';
					$v['Value'] = $x;
					$v['Reserve'] = '';
					array_push($ret, $v);
				}else{
					$w = $this->getWriteValue($com, $wd, $ps);
					$v = array();
					$v['Function'] = $wd;
					$v['Sub'] = '';
					$v['Value'] = $w;
					$v['Reserve'] = '';
					array_push($ret, $v);
				}
      }
    }
    return $ret;
  }



  function showData(){
    echo "<table bgcolor='".$this->bcolor;
    echo "'>\n<tr><td>Parameter<td>Set<td>Current\n";
    foreach($this->showparam as $s){
      if(isset($this->readdata[$s])){
				$nch = 0;
				if(isset($this->reg[$s]['nch'])){
					$ch = 0;
					$nch = $this->reg[$s]['nch'];
				}else{
					$v = $this->readdata[$s];
					$nch = 1;
				}
				
				for($ch=0;$ch<$nch;$ch++){
					if($nch == 1){
						$v = $this->readdata[$s];
					}else{
						$v = $this->readdata[$s][$ch];
					}
					
					
					if(isset($this->reg[$s]['mask'])){
						$v = intval($v,0) & intval($this->reg[$s]['mask'], 0);
					}else{
						$v = intval($v,0);
					}
					echo "<tr bgcolor='".$this->fcolor;
					echo "'><td>".$this->reg[$s]['titl'];
					if($nch > 1){
						echo " (ch".$ch.")\n";
					}
					if(isset($this->reg[$s]['show'])){
						echo "<td>";
						echo $this->{$this->reg[$s]['show']}($v);
						echo "<td>";
						echo $this->{$this->reg[$s]['show']}($v);
					}else if(isset($this->reg[$s]['bits'])){
						echo "<td>";
						echo $this->show_bits_checkbox($v, $this->{$this->reg[$s]['bits']});
						echo "<td>";
						echo $this->show_bits_desc($v, $this->{$this->reg[$s]['bits']});
					}else if(isset($this->reg[$s]['enabits'])){
						echo "<td>";
						echo $this->show_enabits_radiobox($v, $this->{$this->reg[$s]['enabits']}, $s);
						echo "<td>";
					}else if(isset($this->reg[$s]['val'])){
						echo "<td>";
						echo $this->show_val_radiobox($v, $this->{$this->reg[$s]['val']},
																					$s);
						echo "<td>";
						//echo $this->show_val_desc($this->{$this->reg[$s]['val']});
					}else if(isset($this->reg[$s]['edit'])){
						if($nch == 1){
							$ns = $s;
							$id = '';
						}else{
							$ns = $s."-".$ch;
							$id = "class='".$s."'";
						}
						if(isset($this->reg[$s]['sign'])){
							if(($v >= 32768) && !strcmp($this->reg[$s]['sign'], "0xffff")){
								$v = -65536 + $v;
							}
						}
						$r = $this->{$this->reg[$s]['edit']}($ns, $v,
																								 $this->reg[$s]['rang'],
																								 $id);
						echo "<td>".$r[0];
						echo "<td>".$r[1];
						if(isset($this->reg[$s]['mval'])){
							if($v == $this->reg[$s]['mval']){
								echo $this->reg[$s]['mcom']."\n";
							}else{
								if(isset($this->reg[$s]['norm'])){
									if(isset($this->reg[$s]['nobt'])){
										$xb = $this->get_bitsval($this->readdata[$this->reg[$s]['nobt']],
																						 $this->reg[$s]['nobs']);
										$x = $this->reg[$s]['norm'][$xb];
									}else{
										$x = $this->reg[$s]['norm'];
									}
									$vv = $v * $x;
									echo " (".$vv;
									if(isset($this->reg[$s]['nout'])){
										echo $this->reg[$s]['nout'];
									}
									echo ")";
								}
							}
						}else if(isset($this->reg[$s]['norm'])){
							$vv = $v * $this->reg[$s]['norm'];
							echo " (".$vv;
							if(isset($this->reg[$s]['nout'])){
								echo $this->reg[$s]['nout'];
							}
							echo ")";
						}
					}else if(isset($this->reg[$s]['opqn'])){
						echo "<td>";
						echo $this->show_opqn_radiobox($v, $this->{$this->reg[$s]['opqn']}, $s);
						echo "<td>";
					}else{
						echo "<td>";
						echo $v;
						echo "<td>";
						echo $v;
					}
				}
				echo "<br>\n";
				
				if(isset($this->reg[$s]['comt'])){
					echo "<tr bgcolor='".$this->ccolor;
					echo "'><td colspan=3>&nbsp;<i>".$this->reg[$s]['titl'];
					echo " ".$this->reg[$s]['comt']."</i>\n";
				}
				if(isset($this->reg[$s]['winc'])){
					echo "<tr bgcolor='".$this->ccolor;
					echo "'><td colspan=3>&nbsp;<i>";
					echo $this->reg[$s]['winc']."</i>\n";
					$wt = intval($this->readdata['winwidth'], 0);
					$of = intval($this->readdata['winoffset'], 0);
					if(($of >= 32768) && !strcmp($this->reg[$s]['sign'], "0xffff")){
						$of = -65536 + $of;
					}
					$wt = $wt * $this->reg['winwidth']['norm'];
					$of = $of * $this->reg['winoffset']['norm'];
					$ed = $of + $wt;
					$ut = $this->reg['winoffset']['nout'];
					echo " ".$of.$ut." <i>to</i> ".$ed.$ut;
					if($ed > 1){
						echo $this->reg[$s]['wine'];
					}
				}
				if(isset($this->reg[$s]['winmc'])){
					echo "<tr bgcolor='".$this->ccolor;
					echo "'><td colspan=3>&nbsp;<i>";
					echo $this->reg[$s]['winmc']."</i>\n";
					$wt = intval($this->readdata['winwidth'], 0);
					$of = intval($this->readdata['winoffset'], 0);
					$mof = $of - 16384;
					$wt = $wt * $this->reg['winwidth']['norm'];
					$of = $mof * $this->reg['winoffset']['norm'];
					$ed = $of + $wt;
					$ut = $this->reg['winoffset']['nout'];
					echo " ".$of.$ut." <i>to</i> ".$ed.$ut;
				}
				
				if(isset($this->reg[$s]['botn'])){
					echo "<tr bgcolor='".$this->ccolor;
					echo "'><td colspan=3>&nbsp;<i>".$this->reg[$s]['titl']."</i>\n";
					foreach($this->reg[$s]['botn'] as $btn){
						echo "<input type='button' value='".$btn[0]."' ";
						echo " onClick=\"".$btn[1]."('".$s."', ".$btn[2].")\">\n";
					}
					}
      }
    }
    echo "</table>";
  }
}


class CChannelData {
  public $ch = -1;
  public $val = -1;
  public $edge = -1;

  function __construct($ch){
    $this->ch = $ch;
  }

  function setVal($val){
    $this->val = $val;
  }

  function getChannel(){
    return $this->ch;
  }

  function getVal(){
    return $this->val;
  }

  function setEdge($edge){
    $this->edge = $edge;
  }

  function getEdge(){
    return $this->edge;
  }
}

class CModuleData {
  public $geo = -1;
  public $ch = array();

  function __construct($geo){
    $this->geo = $geo;
  }

  function addChannel($ch){
    $cch = new CChannelData($ch);
    array_push($this->ch, $cch);
  }

  function getGeo(){
    return $this->geo;
  }

  function getCh($ch){
    foreach ($this->ch as $c){
      if($c->getChannel() == $ch){
	return $c;
      }
    }
    return NULL;
  }

  function getChEdge($ch, $edge){
    foreach ($this->ch as $c){
      if($c->getChannel() == $ch && $c->getEdge() == $edge){
	return $c;
      }
    }
    return NULL;
  }

  function getChannelEdgeVal($ch, $edge){
    foreach($this->ch as $c){
      if($c->getChannel() == $ch && $c->getEdge() == $edge){
	return $c->getVal();
      }
      return -1;
    }
  }

  function setChannelVal($ch, $val){
    foreach ($this->ch as $c){
      if($c->getChannel() == $ch){
	$c->setVal($val);
	return;
      }
    }
    $this->addChannel($ch);
    $c = end($this->ch);
    $c->setVal($val);
  }

  function setChannelEdgeVal($ch, $edge, $val){
    foreach ($this->ch as $c){
      if($c->getChannel() == $ch && $c->getEdge() == $edge){
	$c->setVal($val);
	$c->setEdge($edge);
	return;
      }
    }
    $this->addChannel($ch);
    $c = end($this->ch);
    $c->setVal($val);
    $c->setEdge($edge);
  }

}


?>
