<?php
require_once('../class/WEBVSTA.class.php');
require_once('../class/moduleidlist.class.php');
require_once('../babilib.php');
require_once('../babixlib.php');
require_once('../class/Spyc.php');

$tstart = microtime(true);

#### constants #####
$crdir = '../dat/vsta_current';
$bkdir = '../dat/vsta_backup';

#### setup file ####
$conffile = '../dat/webvsta.conf';

$html = new BBHtml();
$title = 'Web VME Stat';

$mode = 0; // 0=index, 1=host, 2=online, 3=offline
$host = '';
$mid = 0;
$connect = '';


### Check GET ###
if(isset($_GET)){
  if(isset($_GET['host'])){
    $host = $_GET['host'];
    $mode = 1;
  }

  if(isset($_GET['MID'])){
    $mid = $_GET['MID'];
  }

  if(isset($_GET['connect'])){
    if(!strcmp($_GET['connect'], 'off')){
      $connect = 'off';
      $mode = 2;
    }else{
      $connect = 'on';
      $mode = 3;
    }
  }
}

$bkname = '';
$restfile = '';
$bk = 0;
$rest = 0;
$del = 0;
$delfile = '';
$delyes = 0;
$frommodule = 0;
$genvsta = 0;
$init = 0;
$deleteyes = '';
$copy = 0;
if(isset($_POST)){
  $post = $_POST;
  if(isset($_POST['backup'])){
    $bk =1;
  }
  if(isset($_POST['restore'])){
    $rest =1;
  }
  if(isset($_POST['copy'])){
    $copy =1;
  }
  if(isset($_POST['delete'])){
    $del = 1;
  }
  if($bk==1 && isset($_POST['bkname'])){
    $bkname = $_POST['bkname'];
  }
  if($rest==1 && isset($_POST['restfile'])){
    $restfile = $_POST['restfile'];
  }
  if($copy==1 && isset($_POST['restfile'])){
    $restfile = $_POST['restfile'];
  }
  if($del==1 && isset($_POST['restfile'])){
    $delfile = $_POST['restfile'];
  }
  if(isset($_POST['readfrommodule'])){
    $frommodule = 1;
  }
  if(isset($_POST['hdreadfrommodule'])){
    $frommodule = 1;
  }
  if(isset($_POST['readfromdb'])){
    $frommodule = 0;
  }
  if(isset($_POST['genvsta'])){
    $genvsta = 1;
  }
  if(isset($_POST['init'])){
    $init = 1;
  }
  if(isset($_POST['deleteyes'])){
    $deleteyes = $_POST['deleteyes'];
  }
}

$html->std_html_header($title);
if(strlen($host)){
  $html->heading(2, $title." (".$host.")");
}else{
  $html->heading(2, $title);
}
$html->hr();
$fsave = $html->save_check($post);

$thisfile = basename($_SERVER['PHP_SELF']);
list($tmp, $user, $tp2) = explode('/', $_SERVER['REQUEST_URI']);
$user = str_replace('~', '', $user);

$date = date("Y-m-d_H-i-s");
if($bk){
  $bkfile = $bkdir."/".$host."_".$bkname."_".$date.".sq3";
  $dbf = $crdir."/".$host."_vsta.sq3";
  if(strlen($bkname)){
    copy($dbf, $bkfile);
    echo "Backup file to ".$bkfile."<br>\n";
  }else{
    echo "<font color=magenta>Need to put comments for the backup file.</font><br>\n";
  }
}

if($rest || $copy){
  $dbf = $crdir."/".$host."_vsta.sq3";
  if(strlen($restfile)){
    if($rest){
			echo "Restore file from ".$restfile."<br>\n";
		}else{
			echo "Copy file from ".$restfile."<br>\n";
		}
    copy($restfile, $dbf);
  }
}

if($del){
  if(strlen($delfile)){
    $u = $thisfile."?host=".$host;
    echo "<font color=blue>Delete the backup file ".$delfile."?</font><br>";
    $html->form_start($u);
    echo $html->button_generic('deleteyes', $delfile, '<font color=red>Yes</font>');
    echo "&nbsp;";
    echo $html->button_generic('reflesh', $delfile, 'No');
    $html->form_end();
  }
}

if(strlen($deleteyes)){
	echo "Delete ".$deleteyes."<br>\n";
	unlink($deleteyes);
}

$ldir = '/home/daq/daqconfig';
$rdir = '/home/daq/daqconfig';
$multiuser = 1;
$yamlcom = 1;
$rcopy = 0;

### read conf file ###
if(file_exists($conffile)){
	$yaml = Spyc::YAMLLoad($conffile);
	if(isset($yaml['ldir'])){
		$ldir = $yaml['ldir'];
	}
	if(isset($yaml['rdir'])){
		$rdir = $yaml['rdir'];
	}
	if(isset($yaml['multiuser'])){
		if($yaml['multiuser']){
			$multiuser = 1;
		}else{
			$multiuser = 0;
		}
	}
	if(isset($yaml['useyamlcom'])){
		if($yaml['useyamlcom']){
			$yamlcom = 1;
		}else{
			$yamlcom = 0;
		}
	}

	if(isset($yaml['rcopy'])){
		if($yaml['rcopy']){
			$rcopy = 1;
		}else{
			$rcopy = 0;
		}
	}
}


################## Index ####################
if($mode == 0){ // index
  $html->link('Back to index page', '../index.php');
  $html->hr();
  if(!($hostlist = simplexml_load_file("../dat/hostlist.xml"))){
    echo "Cannot find dat/hostlist.xml";
    die();
  }
  
  $arserver = array();
  foreach($hostlist->host as $node){
    if(!isset($node->disabled)){
      array_push($arserver, $node->name);
    }
  }
  echo "<br><br>\n";
  echo 'List of hosts';
  $html->ul();
  foreach($arserver as $h){
    $html->li();
    $u = $thisfile."?host=".$h;
    $html->link($h, $u);
  }
  $html->ul(1);

  echo 'to add hosts, please edit host list';

################## List ####################
}else if($mode == 1){
  // Find vsta class
  $mlist = new CModuleIDList;
  foreach ($mlist->idlist as $m){
    require_once("../class/".$m['file']);
  }
  $classar = preg_grep("/VSTA/", get_declared_classes());

  $msel = array();
  foreach ($classar as $cl){
    $r = explode('_', $cl);
    if(!isset($msel[$r[0]])){
      $msel[$r[0]] = array();
    }
    $x = new $cl();
    $ad = array($r[1], $x::FUNC, $x->am);
    array_push($msel[$r[0]], $ad);
    unset($x);
  }
  ksort($msel);


  $html->link('Back to top page', './webvsta.php');
  $html->hr();

  echo "\n";
  $dbf = $crdir."/".$host."_vsta.sq3";
  $u = $thisfile."?host=".$host;

  $db = new BBSQLite($dbf);

  $table = 'ModuleList';
  $id = 0;

  $db->query(CDBUtil::sql_create_modulelist());
  $sql='';
  if($fsave){
    $modn = $html->modify_check($post);
    $tlist = new CModuleList();
    foreach ($modn as $n){
      $fquery = 0;
      $tup = $tlist->getpostlist($post, $n);
      if(strlen($tup['ID'])){
	$fquery = 1;
	$sql = $db->sql_update($table, $tup);
	if(!strcmp('delete', $tup['Module'])){
	  $wh = 'ID = '.$tup['ID'];
	  $sql = $db->sql_delete($table, $wh);
	  $fquery = 1;
	}else if(!strlen($tup['Address'])){
	  $fquery = 0;
	}
      }else{
	if(strlen($tup['Module']) && strlen($tup['Address'])){
	  $sql = $db->sql_insert($table, $tup);
	  $fquery = 1;
	}else{
	  $sql = 'nothing';
	}
      }
      if($fquery){
	//echo $sql."<br>\n";
	$db->query($sql);
      }else{
	echo "<font color=red>To update/insert modue, need specify Module and Address</font><br>\n";
      }
    }
  }



  $sql = $db->select('*', $table, '', 'Module, Address');
  $list = $db->query_assoc($sql);
  // genvsta
  if($genvsta){
    echo "<font color=blue>Generate init script to init/webvsta.sh </font>\n";
    echo " (init/webvsta.sh should be called from init/daqinitrc)<br>\n";
		$date = date("Y-m-d_H-i-s");

    $wr = array();
    array_push($wr, "#/bin/sh");
    array_push($wr, "/bin/touch /tmp/babiesblk");
    array_push($wr, "### Generated by Web VME Stat");
    array_push($wr, "### Host = ".$host);
    foreach($list as $li){
      $tab = $li['Module']."_at_".$li['Address'];
      $class = $li['Module']."_VSTA";
      //$module = new $class($li['Comment']);
      $module = new $class('');
      $module->setBaseAddress("0x".$li['Address']);

      $dbpr = $db->query_assoc("select * from ".$tab);
      $dbrd = $module->dbprtord($dbpr);
      //print_r($dbrd);
      array_push($wr, "### Module = ".$li['Module']." 0x".$li['Address']);
      $wcoms = $module->getWriteCommandDB($dbrd);

      array_push($wr, $module->vmeAM($module->am));
      foreach($wcoms as $c){
				array_push($wr, $c);
				//echo $c."<br>\n";
      }
      array_push($wr, $module->vmeAM($module::A32));
    }
    array_push($wr, "/bin/rm -f /tmp/babiesblk");
    array_push($wr, ' ');

    $pt = $ldir.'/'.$host.'/init/webvsta.sh';
    file_put_contents($pt, implode("\n", $wr));
    echo $pt." is generated ".$date."<br>\n";

		$lst = $user."=".$date;
		$lastf = $ldir.'/'.$host.'/init/lastwebvsta';
		file_put_contents($lastf, $lst);

		if($rcopy){
			$rpt = $rdir.'/'.$host.'/init/webvsta.sh';
			if(moncopyfile($host, $pt, $rpt) >= 0){
				echo $pt." is copied to ".$host.":".$rpt."<br>\n";
			}else{
				echo "error during file copy to ".$host."<br>\n";
			}
		}

    $html->hr();
  }

  // init
  if($init){
    echo "<font color=blue>Initialize VME (init/daqinitrc) </font><br>";
    $pt = $rdir.'/'.$host.'/init/daqinitrc';

    if(monchkbabies($host) == 1){
      echo "<font color=red>Now DAQ is running. Initialization is canceled</font><br>\n";
    }else{
      execarg($host, $pt);
      echo "Initializing...<br>\n";
      if(waitbabiesblk($host)){
				echo "Done<br>\n";
      }else{
				echo "Timeout<br>\n";
      }
    }
    $html->hr();
  }


  $html->heading(3, "Module List");
  $clist = new CModuleList();

  // Form
  $html->std_form_header($u);
  $html->table_start('');
  $html->tr_start('#ccffcc');
  echo $clist->tablehead();
  $html->tr_end();
  $idx = 1;
  foreach($list as $li){
    echo $clist->tableedit($li, $id, $msel, $idx);
    $li = $u."&MID=".$li['ID'];
    echo "<td>";
    $st = "style='background-color:#22ccff'";
    $html->button_link("Offline", $li."&connect=off", $st);
    echo "<td>";
    $st = "style='background-color:#ffcc22'";
    $html->button_link("Online", $li."&connect=on", $st);
    $id++;
    $idx++;
  }
  echo $clist->tableadd($id, $msel);
  $id++;
  $html->table_end();
  $html->input_modify($id);  // hidden inputs for modification check
  
  $html->std_form_ender('', '../class');
  $html->select_connect('ven','mod',$id);
  unset($db);

	$own = posix_getpwuid(fileowner($dbf));
	if(!strcmp($own['name'], 'apache')){
		chmod($dbf, 0666);
	}


  // daqinitrc
  $html->hr();
  $html->heading(3, "Init script");
  $html->form_start($u);
  echo "Generate init script to init/webvsta.sh ";
  echo " (init/webvsta.sh should be called from init/daqinitrc)<br>\n";
	if($rcopy){
		$tcom = 'Generate webvsta.sh and Copy to '.$host;
		echo $html->button_generic('genvsta', 'genvsta', $tcom);
	}else{
		echo $html->button_generic('genvsta', 'genvsta', 'Generate webvsta.sh');
	}
  echo $html->button_generic('init', 'init', 'Initialize modules (init/daqinitrc)');
  $html->form_end();

	echo "<p>";
	$lastf = $ldir.'/'.$host.'/init/lastwebvsta';
	$lst = file_get_contents($lastf);
	list($nm, $dt) = explode("=", $lst);
	list($yd, $tm) = explode('_', $dt);
	$tm = str_replace("-", ':', $tm);
	$ldt = $yd." ".$tm;

	echo "Last generated = ".$ldt." by ";
	if(!strcmp($nm, $user)){
		echo $nm."<br>\n";
	}else{
		echo "<b><font color='#ff1111'>".$nm."</font></b><br>\n";
	}
	$ddate = date("Y-m-d H:i:s");

	echo "(Now=".$ddate." User=".$user.")<br>\n";

  // Backup
  $html->hr();
  $html->heading(3, "Backup");
  $html->form_start($u);
  echo "Comments for the backup file : ";
  echo "<input type='text' size=20 name='bkname' pattern=\"^[A-Za-z0-9]+$\" title='only alphanumeric characters'>\n";
  echo $html->button_generic('backup', 'backup', 'Backup');

  $html->form_end();


  // Restore
  $html->hr();
  $html->heading(3, "Restore / Delete backup file");
  $html->form_start($u);

  $files = glob($bkdir."/".$host."_*.sq3");
  $fl = array();
  foreach($files as $fname){
    $f = str_replace($bkdir."/", '', $fname);
    $f = str_replace(".sq3", '', $f);
    $far = explode('_', $f);
    $t = str_replace("-", ':', $far[3]);
    $fk = $far[2]." ".$t;
    $fadd = array($fname, $far[1]);
    $fl[$fk] = $fadd;
  }
  krsort($fl);
  echo "<table><tr><td>";
  echo "Restore from : ";
  echo "<td><select name='restfile' size=5>\n";
  foreach($fl as $k => $f){
    echo "<option value='".$f[0]."'>".$k." ".$f[1]."</option>\n";
  }
  echo "</select>\n<td>";
  echo $html->button_generic('restore', 'restore', 'Restore');
  echo "<td>&nbsp;<td>";
  echo $html->button_generic('delete', 'delete', 'Delete');
  echo "</table>\n";
  $html->form_end();


	// from other user
	$mydate = filemtime('../dat/vsta_current/'.$host."_vsta.sq3");
	$ofl = array();
	$dirlist = scandir('/home/');
	foreach($dirlist as $li){
		if(!strcmp(".", $li)) continue;
		if(!strcmp("..", $li)) continue;

		$f = '/home/'.$li.'/public_html/dat/vsta_current/'.$host."_vsta.sq3";
		if(is_readable($f) && strcmp($user, $li)){
			$ndate = filemtime($f);
			$t = date("Y-m-d H:i:s", $ndate);
			$ofl[$t] = array($f, $li, $ndate);
		}
	}

  $html->form_start($u);
  krsort($fl);
  echo "<table><tr><td>";
	if($multiuser){
		echo "Copy from others: ";
		echo "<td><select name='restfile' size=5>\n";
		foreach($ofl as $k => $f){
			echo $f[2]." ".$mydate."<br>\n";
			if($f[2] > $mydate){
				echo "<option value='".$f[0]."' style='color:#ff2200'>".$k." ".$f[1]."</option>\n";
			}else{
				echo "<option value='".$f[0]."'>".$k." ".$f[1]."</option>\n";
			}
		}
		echo "</select>\n<td>";
		echo $html->button_generic('copy', 'copy', 'Copy');
		echo "</table>\n";
	}
	echo "(My file date = ".date("Y-m-d H:i:s", $mydate).")<br>\n";


  // YAML
	if($yamlcom){
		$html->hr();
		$html->heading(3, 'From / To YAML file (not yet implemented)');
		$html->form_start($u);
		echo "DB to YAML : ";
		echo "<input type='button' value='to YAML'><br>\n";
		echo "YAML from local File : ";
		echo "<input type='file' id='filein'><br>\n";
		$html->fileupload_textarea('yaml', 'filein');
		echo "<br>\n";
		$html->clear_textarea('clrbtn', 'yaml', 'Clear');
		echo "<br>Save to local File : ";
		echo "<input type='button' value='to Local'><br>\n";
		$html->form_end();
	}


################## Online / Offline ####################
}else if($mode == 2 || $mode == 3){
  $dbf = $crdir."/".$host."_vsta.sq3";
  $u = $thisfile."?host=".$host."&MID=".$mid."&connect=".$connect;

  $db = new BBSQLite($dbf);

  $res = $db->query_assoc(CDBUtil::sql_select_modulelist_id($mid));

  $tab = $res[0]['Module']."_at_".$res[0]['Address'];
  $db->query(CDBUtil::sql_create_moduledata($tab));

  $mlist = new CModuleIDList;
  foreach ($mlist->idlist as $m){
    require_once("../class/".$m['file']);
  }

  $class = $res[0]['Module']."_VSTA";
  $module = new $class('');
  $module->setBaseAddress("0x".$res[0]['Address']);
  $module->echoJScript();

  $html->link('Back to top page', "./webvsta.php?host=".$host);
  $html->hr();


  if($mode == 3 && $frommodule == 1){
    $ret = monchkbabies($host);
    if($ret == 1){
      echo '<font color=blue>Now DAQ is Running (use offline mode)</font><br><br>';
      $frommodule = 0;
    }else{
    }
  }

  $r = 1;
  if($fsave){
    $wcoms = $module->getWriteCommand($post);
    $dbcoms = $module->getDBCommand($post);
    if($mode == 3){
      $frommodule = 1;
      $r = execcmdvme($host, $module->vmeAM($module->am));
      if($r != -1){
				foreach($wcoms as $c){
					execcmdvme($host, $c);
				}
				execcmdvme($host, $module->vmeAM($module::A32));
      }else{
				echo "Connection error ".$res[0]['Module']." 0x".$res[0]['Address'];
      }
		}
		
    $db->begin();
    foreach($dbcoms as $c){
      $s = CDBUtil::sql_replace_moduledata($tab, $c);
      $db->exec($s);
    }
    $db->commit();


		if(isset($post['Comment'])){
			$ttab = 'ModuleList';
			$ar = array();
			$ar['ID'] = $mid;
			$ar['Comment'] = $post['Comment'];
			$sql = $db->sql_update($ttab, $ar);
			$db->query($sql);
			$res = $db->query_assoc(CDBUtil::sql_select_modulelist_id($mid));
		}
		
  }

  if($mode == 3){
    $html->form_start($u);
    echo $html->button_generic('readfromdb', 'readfromdb', 'Read from DB');
    echo "&nbsp;";
    echo $html->button_generic('readfrommodule', 'readfrommodule', 'Read from Module');
    $html->form_end();
  }

  if($mode == 3){
    $html->std_form_header($u, '', 'Save into DB and Module');
  }else{
    $html->std_form_header($u, '', 'Save into DB');
  }

  echo "<p>";
  $html->heading(3, $res[0]['Module']." 0x".$res[0]['Address']);
  if($frommodule == 0){
    echo "Data from DB <font color=green>(offline)</font><br>\n";
  }else{
    echo "Data from Module <font color=red>(online)</font><br>\n";
  }
  echo "</p>";


  if($frommodule == 0){
    $dbpr = $db->query_assoc("select * from ".$tab);
    $dbrd = $module->dbprtord($dbpr);

    $module->setOfflineData($dbrd);
  }else{
    $r = execcmdvme($host, $module->vmeAM($module->am));
    if($r != -1){
      $coms = $module->getReadCommand();
      $val = array();
      foreach($coms as $c){
				$r = execcmdvme($host, $c);
				foreach($r as $rr){
					array_push($val, $rr);
				}
      }
      $module->setReadData($val);
      execcmdvme($host, $module->vmeAM($module::A32));
    }else{
      echo "Connection error ".$res[0]['Module']." 0x".$res[0]['Address'];
    }
  }
    
  if($r) $module->showData();


	echo "Comment = <br>\n";
	echo "<textarea name='Comment' cols=40 rows=10>\n";
  echo $res[0]['Comment'];
	echo "</textarea>\n";


  $html->hr();

  if($mode == 3 && $frommodule == 1){
    echo $html->hidden('hdreadfrommodule', 'hdreadfrommodule');
  }else{
    echo $html->hidden('hdreadfromdb', 'hdreadfromdb');
  }

  
  if($mode == 3){
    $html->std_form_ender('', '../class', 'Save into DB and Module');
  }else{
    $html->std_form_ender('', '../class', 'Save into DB');
  }    

  if($mode == 3){
    $html->form_start($u);
    echo $html->button_generic('readfromdb', 'readfromdb', 'Read from DB');
    echo "&nbsp;";
    echo $html->button_generic('readfrommodule', 'readfrommodule', 'Read from Module');
    $html->form_end();
  }

################## Online ####################
}



$tstop = microtime(true) - $tstart;

echo "<br><br>Transaction Time=".sprintf("%10.3f", $tstop)." ms<br>\n";

$html->std_html_ender();


?>
