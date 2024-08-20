<?php

function printxdaqinfo($dom){
  global $ofstr, $runstr;

#   DAQINFO
  echo "<b>EB Information</b><br>\n";
  $of = $ofstr[(int)($dom->daqinfo->babildes)];
  echo "<table><tr><td bgcolor='#33ffff'>EF Number<td>". $dom->daqinfo->efn ."</tr>\n";
  $kb = (int)((int)($dom->daqinfo->ebsize)*2/1024);
  echo "<tr><td bgcolor='#33ffff'>Event Build Size<td>". $dom->daqinfo->ebsize ." ($kb kB)</tr>\n";
  echo "<tr><td bgcolor='#33ffff'>Babildes mode<td>$of</tr>\n";
  echo "<tr><td bgcolor='#33ffff'>RUN Name<td>". $dom->daqinfo->runname ."</tr>\n";
  echo "</table><br>\n";

}

function printxeflist($dom){
  global $ofstr, $runstr;

  $efnar = array();

#   EFLIST
  echo "<b>Event fragment</b><br>\n";
  echo "<table><tr bgcolor='#33ffff'><td>EFN<td>Hostname<td>Nickname<td>on/off</tr>\n";
  foreach($dom->daqinfo->eflist as $ef){
    $of = $ofstr[(int)$ef->of];
    echo "<tr><td>".$ef->efn."<td>".$ef->host."<td>".$ef->name."<td>$of</tr>\n";
    array_push($efnar, $ef->efn);
  }
  echo "</table><br>\n";

  return $efnar;
}

function editxeflist($fid, $dom){
  global $ofstr, $runstr;

  $efnar = array();
  $i = $fid;
#   EFLIST
  echo "<b>Event fragment</b><br>\n";
  echo "<table><tr bgcolor='#33ffff'><td>EFN<td>Hostname<td>Nichname<td>on/off</tr>\n";
  $intd = "<td><input type=text size=7 ";
  $chktd = "<td><input type=checkbox ";
  foreach($dom->daqinfo->eflist as $ef){
    $of = $ofstr[(int)$ef->of];
    $ni = "efn".$i;
    $nh = "host".$i;
    $nn = "name".$i;
    $no = "of".$i;
    echo "<tr><td><input type=text maxlength=3 size=1 name=".$ni." value=".$ef->efn.">\n";
    echo $intd." name=".$nh." value=".$ef->host.">\n";
    echo $intd." name=".$nn." value=".$ef->name.">\n";
    echo "<td><select name=".$no.">";

    for($j=0;$j<3;$j++){
      if((int)($ef->of) == $j){
	$sel = 'selected';
      }else{
	$sel = '';
      }
      echo "<option value=".$j." $sel>".$ofstr[$j];
    }
    echo "</select>\n";
    echo "</tr>\n";
    array_push($efnar, $ef->efn);
    $i++;
  }

  $ni = "efn".$i;
  $nh = "host".$i;
  $nn = "name".$i;
  $no = "of".$i;
  $dl = "dl".$i;
  echo "<tr><td><input type=text maxlength=3 size=1 name=".$ni.">\n";
  echo $intd." name=".$nh.">\n";
  echo $intd." name=".$nn.">\n";
  echo "<td><select name=".$no.">";
  echo "<option value=0 selected>".$ofstr[0];
  echo "<option value=1 >".$ofstr[1];
  echo "<option value=2 >".$ofstr[2];
  echo "</select></tr>\n";
  
  $i++;
  echo "</table><br>\n";

  return array($i, $efnar);
}

function printxruninfo($dom){
  global $ofstr, $runstr;

#   DAQINFO
  echo "<b>RUN Information</b><br>\n";
  echo "<table><tr><td bgcolor='#ff9933'>RUN Number<td>". $dom->runinfo->runnumber ."</tr>\n";
  echo "<tr><td bgcolor='#ff9933'>Status<td>". $dom->runinfo->runstat ."</tr>\n";
  echo "<tr><td bgcolor='#ff9933'>Start Date<td>". $dom->runinfo->starttime ."</tr>\n";
  echo "<tr><td bgcolor='#ff9933'>Stop Date<td>". $dom->runinfo->stoptime ."</tr>\n";
  echo "<tr><td bgcolor='#ff9933'>Header<td>". $dom->runinfo->header ."</tr>\n";
  echo "<tr><td bgcolor='#ff9933'>Ender<td>". $dom->runinfo->ender ."</tr>\n";

  echo "</table><br>\n";

}

function getxrunstat($dom){
  return $dom->runinfo->runstat;
}

function hdpathopt($hi, $xhdlist){
  foreach($xhdlist->babild->hd as $thd){
    if($hi == (int)$thd->id){
      echo "<option value=".$thd->path.">".$thd->path."\n";
    }
  }
}

function editxhdlist($fid, $dom, $xhdlist){
  global $ofstr, $runstr;

  $i = $fid;
#   HDLIST
  echo "<b>HD List</b><br>\n";
  echo "<table><tr bgcolor='#66ccff'><td>HDN<td>Path<td>on/off</tr>\n";

  for($thdn=0;$thdn<4; $thdn++){
    $hd = 0;
    $ni = "hdhdn".$i;
    $np = "hdpath".$i;
    $no = "hdof".$i;
    foreach($dom->daqinfo->hdlist as $thd){
      if($thdn == (int)$thd->hdn){
	$hd = $thd;
      }
    }
    echo "<tr><td>".$thdn."\n";
    echo "<input type=hidden name=".$ni." value=".$thdn.">\n";
    echo "<td><select name=".$np.">";
    if(!$hd){
      $of = $ofstr[0];
      echo "<option value='' selected> \n";
      hdpathopt($thdn, $xhdlist);
      echo "</select>\n";
      echo "<td><select name=".$no.">";
      echo "<option value=0 selected>".$ofstr[0];
      echo "<option value=1 >".$ofstr[1];
      echo "</select>\n";
    }else{
      $of = $ofstr[(int)$hd->of];
      echo "<option value=".$hd->path." selected>".$hd->path;
      hdpathopt($thdn, $xhdlist);
      echo "</select>\n";
      echo "<td><select name=".$no.">";
      for($j=0;$j<2;$j++){
	if((int)($hd->of) == $j){
	  $sel = 'selected';
	}else{
	  $sel = '';
	}
	echo "<option value=".$j." $sel>".$ofstr[$j];
      }
      echo "</select>\n";
    }
    echo "</tr>\n";
    $i++;
  }
  echo "</table><br>\n";

  return $i;
}

function editxruninfo($fid, $dom, $xhdlist){
  global $ofstr, $runstr;

  $i = $fid;
#   HDLIST
  echo "<b>RUN Information</b><br>\n";
  echo "<table><tr><td bgcolor='#88ffcc'>Run name\n";
  
  $rnamei = "runname".$i;
  $i++;
  $rnumi = "runnumber".$i;
  $i++;

  echo "<td><input type=text maxlength=10 size=10 name=".$rnamei;
  echo " value=".$dom->daqinfo->runname."></tr>\n";
  echo "<tr><td bgcolor='#88ffcc'>Run number\n";
  echo "<td><input type=text maxlength=10 size=5 name=".$rnumi;
  echo " value=".$dom->daqinfo->runnumber.">\n";
  echo "</table>\n";
  return $i;
}

function printxhdlist($dom){
  global $ofstr, $runstr;

#   HDLIST
  echo "<b>Hd List</b><br>\n";
  echo "<table><tr bgcolor='#33ffff'><td>Id<td>Path<td>On/Off</tr>\n";
  foreach($dom->daqinfo->hdlist as $hd){
    $of = $ofstr[(int)($hd->of)];
    echo "<tr><td>".$hd->hdn."<td>".$hd->path."<td>$of</tr>\n";
  }
  echo "</table><br>\n";
}

function printesconfig($dom){
  global $erstr;

  $bg = "bgcolor = '#44cc99'";

  foreach($dom->esconfig->node as $ef){
    if($ef->error){
      echo "<B><font color=magenta>".$ef->error."</font></B><br>\n";
      continue;
    }else{
      echo "<b>".$ef->myhost." EFN=".$ef->efn." Configuration</b><br>\n";
      echo "<table><tr><td $bg>EBHost<td>".$ef->host;
      $er = $erstr[(int)($ef->isdrv)];
      echo "<tr><td $bg>Driver Existence<td>".$er;
      echo "<tr><td $bg>Driver Dir<td>".$ef->rtdrv;
      echo "</table>\n";
    }
  }
}


function esebhostopt($hostlist){
  foreach($hostlist->host as $thost){
    if(isset($thost->babild)){
      echo "<option value=".$thost->name.">".$thost->name."\n";
    }
  }
}

function esrtdrvopt($mylist){
  if(isset($mylist->rtdrv)){
    foreach($mylist->rtdrv->path as $tdrv){
      echo "<option value=".$tdrv.">".$tdrv."\n";
    }
  }
}

function editxesconfig($fid, $dom, $hostlist){
  global $erstr;

  $bg = "bgcolor = '#44ff99'";
  $i = $fid;

  foreach($dom->esconfig->node as $ef){
    if($ef->error){
      echo "<B><font color=magenta>".$ef->error."</font></B><br>\n";
      continue;
    }else{
      echo "<b>".$ef->myhost." EFN=".$ef->efn." Configuration</b><br>\n";
      echo "<table><tr><td $bg>EBHost<td>";
      $si = "ebhost".$i;
      $di = "rtdrv".$i;
      $chki = "reload".($i + 1);
      $efni = "esefn".$i;
      echo "<input type=hidden name=".$efni." value=".$ef->efn." >";
      echo "<select name=".$si." >";
      echo "<option value=".$ef->host." selected>".$ef->host;
      esebhostopt($hostlist);
      echo "</select>\n";
      $er = $erstr[(int)($ef->isdrv)];
      echo "<tr><td $bg>ISDriver<td>".$er;
      echo "<tr><td $bg>Driver Dir";
      echo "<td><select name=".$di." >";
      echo "<option value=".$ef->rtdrv." selected>".$ef->rtdrv;
      foreach($hostlist->host as $thost){
	if(!strcmp($thost->name, $ef->myhost)){
	  $mh = $thost;
	}
      }
      esrtdrvopt($mh);
      echo "</selected>\n";
      echo "<tr><td $bg>Reload Driver";
      echo "<td><input type=checkbox name=".$chki." value=".$ef->efn." >";
      echo "</table><br>\n";
      $i+=2;
    }
  }

  return $i;
}


function chkboxchkd($of){
  if(!strcmp($of, 'on')) return 'checked';
  return '';
}

function chkisboxchkd($is){
  if($is) return 'checked';
  return '';
}

function bchkbox($name, $text, $of){
  if(strlen($name)){
    $name .= '_';
  }
  $ret = "<input type=checkbox name='".$name.$text."' ".chkboxchkd($of)." value='on'>".$text."\n";
  
  return $ret;
}

function bchkbox_name($name, $text, $of){
  $ret = "<input type=checkbox name='".$name.'_'.$text."' ".chkboxchkd($of)." value='on'>".$name."\n";
  
  return $ret;
}

function bchkboxn($name, $of){
  $ret = "<input type=checkbox name='".$name."' ".chkboxchkd($of)." value='on'>\n";
  return $ret;
}

function bischkbox($name, $is){
  $ret = "<input type=checkbox name='".$name."' ".chkisboxchkd($is)." value='on'>".$name."\n";
  
  return $ret;
}

function btxt($name, $sz, $mx, $val){
  $smx = '';
  $ssz = '';
  if($mx){
    $smx = "maxlength=".$mx;
  }
  if($sz){
    $ssz = "size=".$sz;
  }
  $ret = "<input type=text ".$ssz." name='".$name."' value='".$val."'>";

  return $ret;
}

function bopt($name, $optlist, $def){
  $optname = $name."_option";
  $ret = "<select name=".$optname.">\n";
  $i = 0;
  foreach($optlist as $opt){
    $sel = '';
    if(!strcmp($opt, $def)) $sel = 'selected';
    $ret .= "<option ".$sel." value=".$opt." >".$opt;
    $i++;
  }
  $ret .= "</select>\n";

  return $ret;
}

function babildopt($def){
  $optlist[0] = '-l';
  $optlist[1] = ' ';

  $ret = bopt('babild', $optlist, $def);

  return $ret;
}

function babiauopt($def){
  $optlist[0] = '-lt';
  $optlist[1] = '-l';
  $optlist[2] = ' ';

  $ret = bopt('babiau', $optlist, $def);

  return $ret;
}

function babiesopt($def){
  $optlist = array('-l','-r','-s','-c','-d');

  $ret = bopt('babies', $optlist, $def);

  return $ret;
}


function addcompost(&$dom, $tag, $okey, $key, $val){
  if(!strcmp($okey, $key)){
    $dom->appendChild($dom->ownerDocument->createElement($tag, $val));
  }
}

function printxml($xml){
  $rxml = str_replace("<", "&lt;", $xml);
  $rxml = str_replace(">", "&gt;", $rxml);
  echo "<pre>";
  echo $rxml;
  echo "</pre>";
}


function dom_loadfile(&$dom, $file){
  $dom = new DomDocument('1.0');
  if(!$dom->load($file)){
    return 0;
  }else{
    return $dom->firstChild;
  }
}

function dom_loadfile_node(&$dom, $file, $node){
  $dom = new DomDocument('1.0');
  if(!$dom->load($file)){
    $rnode = $dom->appendChild($dom->createElement($node));
  }else{
    $chkdom = $dom->getElementsByTagName($node);
    if($chkdom->length){
      $rnode = $chkdom->item(0);
    }else{
      $rnode = 0;
    }
  }
  return $rnode;
}

function dom_savefile(&$dom, $file){
  $dom->formatOutput = true;
  $dom->preserveWhiteSpace = false;
  $dom->save($file);

  if(chkowner($file)) chmod($file, 0666);
}

function chkowner($file){
  if(!function_exists('posix_getpwuid')) return 1;

  $u = posix_getpwuid(fileowner($file));

  if(fileowner($file) == posix_geteuid()){
    return 1;
  }else{
    return 0;
  }
}

function dom_find_hostname(&$dom, $name){
  $hostdom = $dom->getElementsByTagName('host');
  foreach($hostdom as $hd){
    $namedom = $hd->getElementsByTagName('name');
    if($namedom->length){
      $tname = $namedom->item(0);
      if(!strcmp($tname->nodeValue, $name)){
	//xmlecho('test host search', $tname->nodeValue);
	return $hd;
      }
    }
  }
  return 0;
}

function dom_append_simple_hostlist(&$dom, $sxml){
  $hl = $dom->firstChild;
  //xmlecho('fistnode', $hl->nodeName);

  if(strcmp($hl->nodeName, $sxml->getName())){
    return 0;
  }

  foreach($sxml->children() as $n){
    if(!strcmp($n->getName(), 'host')){
      if(isset($n->name)){
	//xmlecho('sxml name', $n->name);
	$th = dom_find_hostname($hl, $n->name);
	if(!$th){
	  $th = $hl->appendChild($dom->createElement('host'));
	  $th->appendChild($dom->createElement('name', $n->name));
	}
      }else{
	/* no name */
      }
    }
  }
}

function dom_remove_simple_hostlist_tag(&$dom, $sxml, $tag){
  $hl = $dom->firstChild;
  //xmlecho('fistnode', $hl->nodeName);

  if(strcmp($hl->nodeName, $sxml->getName())){
    return 0;
  }

  foreach($sxml->children() as $n){
    if(!strcmp($n->getName(), 'host')){
      if(isset($n->name)){
	//xmlecho('sxml name', $n->name);
	$th = dom_find_hostname($hl, $n->name);
	if($th){
	  $thl = $th->getElementsByTagName($tag);
	  foreach($thl as $thle){
	    $th->removeChild($thle);
	  }
	}
      }else{
	/* no name */
      }
    }
  }
}

function dom_remove_simple_babild(&$dom, $sxml, $bst){
  $hl = $dom->firstChild;
  //xmlecho('fistnode', $hl->nodeName);

  if(strcmp($hl->nodeName, $sxml->getName())){
    return 0;
  }

  foreach($sxml->children() as $n){
    if(!strcmp($n->getName(), 'host')){
      if(isset($n->name)){
	//xmlecho('sxml name', $n->name);
	$th = dom_find_hostname($hl, $n->name);
	$f = 0;
	foreach($bst as $nm){
	  if(!strcmp($n->name, $nm)){
	    $f = 1;
	  }
	}
	if($f == 0){
	  $thl = $th->getElementsByTagName('babild');
	  foreach($thl as $thle){
	    $th->removeChild($thle);
	  }
	}
      }else{
	/* no name */
      }
    }
  }
}


function dom_disable_simple_hostlist(&$dom, $sxml){
  $hl = $dom->firstChild;
  //xmlecho('fistnode', $hl->nodeName);

  if(strcmp($hl->nodeName, $sxml->getName())){
    return 0;
  }

  $hostdom = $dom->getElementsByTagName('host');
  foreach($hostdom as $hd){
    $namedom = $hd->getElementsByTagName('name');
    if($namedom->length){
      $tname = $namedom->item(0);
      $flag = 0;
      foreach($sxml->children() as $n){
	if(!strcmp($n->getName(), 'host')){
	  if(isset($n->name)){
	    if(!strcmp($tname->nodeValue, $n->name)){
	      //xmlecho('find', $tname->nodeValue);
	      $flag = 1;
	    }
	  }
	}
      }
      $edi = $hd->getElementsByTagName('disabled');
      if($flag){
	if($edi->length){
	  $hd->removeChild($edi->item(0));
	}
      }else{
	if(!$edi->length){
	  $hd->appendChild($dom->createElement('disabled'));
	}
      }
    }else{
      $hl->removeChild($hd);
    }
  }
}

function dom_dup_append_tag(&$dom, &$th, $tag){
  $tl = $th->getElementsByTagName($tag);
  if(!$tl->length){
    $th->appendChild($dom->createElement($tag));
  }
}

function dom_sort_simple_hostlist(&$dom, $sxml){
  $i = 0;
  foreach($sxml->host as $th){
    if(isset($th->name)){
      $tn = $th->name;
      $nidx["$tn"] = $i;
      $i++;
    }
  }
  $j = $i;
  $hostdom = $dom->getElementsByTagName('host');
  foreach($hostdom as $hd){
    $namedom = $hd->getElementsByTagName('name');
    if($namedom->length){
      $tname = $namedom->item(0);
      if(isset($nidx[$tname->nodeValue])){
	$slist[$nidx[$tname->nodeValue]] = $hd;
      }else{
	$slist[$j] = $hd;
	$j++;
      }
    }
  }

  $rdom = new DomDocument('1.0');
  $hlist = $rdom->appendChild($rdom->createElement('hostlist'));
  
  for($i=0;$i<$j;$i++){
    $hlist->appendChild($rdom->importNode($slist[$i], TRUE));
  }

  $dom = $rdom;
}

function dom_node_isset(&$dom, $tag){
  $dl = $dom->getElementsByTagName($tag);
  return $dl->length;
}

function dom_remove_children_tag(&$dom, $tag){
  $dl = $dom->getElementsByTagName($tag);
  for($i=$dl->length-1;$i>=0;$i--){
    //echo "remove ".$dl->item($i)->nodeName."  ".$dl->item($i)->tagName."<br>\n";
    $dom->removeChild($dl->item($i));
  }
}

function dom_find_node(&$dom, $tag){
  $dl = $dom->getElementsByTagName($tag);
  if($dl->length){
    return $dl->item(0);
  }else{
    return 0;
  }
}

function dom_find_node_value(&$dom, $tag){
  $dl = $dom->getElementsByTagName($tag);
  if($dl->length){
    return $dl->item(0)->nodeValue;
  }else{
    return 0;
  }
}


function xmlecho($a, $b){
  echo "$a = $b<br>\n";
}
