<?php
require_once 'class/SelGTO.class.php';
require_once 'class/bbhtml.class.php';

//print_r(unpack("C*", $data));

$thisfile = basename($_SERVER['PHP_SELF']);
$title = "Selector GTO";

$namefile = 'dat/selgto.xml';


// HTML
$html = new Cbbhtml();
$html->std_html_header($title);

// GET
if(isset($_GET['host'])){
  $host = $_GET['host'];
}else if($argc == 2){
	$host = $argv[1];
}else{
  echo 'server=hostname or file=filename is not given';
  $html->std_html_ender();
  die();
}
$thislink = $thisfile."?host=".$host;
$indexlink = 'index.php';

$html->heading(2, $title." (".$host.")");
$html->hr();

// optain status
$gto = new CSelGTO($host);
$ret = $gto->open();
if(strlen($ret)){
	echo $ret;
	$html->std_html_ender();
	die();
}
$data = $gto->read();
$gto->close();
//

// Read Names from XML file
if(!($namexml = simplexml_load_file($namefile))){
	$dom = new DomDocument('1.0');
	$dom->formatOutput = true;
	$domselgto = $dom->appendChild($dom->createElement('selgto'));
	$i = 0;
	$ndom = $domselgto->appendChild($dom->createElement('weblock', 'no'));
	for($i=0;$i<$gto::NOFIN;$i++){
		$ndom = $domselgto->appendChild($dom->createElement('input'));
		$ndom->appendChild($dom->createElement('id', $i));
		$ndom->appendChild($dom->createElement('name', $gto->seladv[$i]['param']));
	}
	for($i=0;$i<$gto::NOFOUT;$i++){
		$ndom = $domselgto->appendChild($dom->createElement('output'));
		$ndom->appendChild($dom->createElement('id', $i));
		$ndom->appendChild($dom->createElement('lock','no'));
		$tn = "out".$i;
		$ndom->appendChild($dom->createElement('name', $tn));
	}	
	$namexml = $dom->saveXML();
	$fp = fopen($namefile, "w");
	fwrite($fp, $namexml);
	fclose($fp);
}

// Check save
$fsave = 0;
if(isset($_POST['save'])){
	$fsave = 1;
}else if(isset($_POST['eepwrite'])){
	$gto->open();
	$gto->eepwrite();
	$data = $gto->read();
	$gto->close();
	echo "<br><b>EEPWrite Done</b><br><br>\n";
}else if(isset($_POST['eepread'])){
	$gto->open();
	$gto->eepread();
	$data = $gto->read();
	$gto->close();
	echo "<br><b>EEPRead Done</b><br><br>\n";
}else if(isset($_POST['weblock'])){
	$namexml->weblock = 'yes';
	$fp = fopen($namefile, "w");
	fwrite($fp, $namexml->asXML());
	fclose($fp);
}else if(isset($_POST['webunlock'])){
	$namexml->weblock = 'no';
	$fp = fopen($namefile, "w");
	fwrite($fp, $namexml->asXML());
	fclose($fp);
}

if(!strcmp($namexml->weblock, 'yes')){
	$weblock = 1;
}else{
	$weblock = 0;
}


$mode = 0;
// Check thd display mode
if(isset($_POST['modifymode'])){
	$html->link('Back to the top page', $thislink);
	$html->br();
	$modnum = $_POST['modifymode'];
	$html->heading(3, 'Modify Output '.$modnum);
	$mode = 1;
	if($fsave){
		$chs = array();
		foreach($_POST as $key => $val){
			if(strcmp($key, 'save') && strcmp($key, 'modifymode')){
				array_push($chs, $key);
			}
		}
		$gto->open();
		$gto->selout($modnum, $chs);
		$data = $gto->read();
		$gto->close();
	}
	$gto->decode($data);

	$html->table_start_collapse("#ffcc99");
	echo "<tr>";
	for($j=0;$j<$gto::NOFPARAM;$j++){
		if($gto->chkout($modnum, $j)){
			echo $html->td_border($html::BDALL, 150);
			echo $gto->seladv[$j]['param']." : ";
			echo $gto->displayname($j, $namexml);
		}
	}
	echo "</tr>";
	$html->table_end();
	$html->br();
}else if(isset($_POST['dsmode'])){
	$html->link('Back to the top page', $thislink);
	$html->br();

	$html->heading(3, 'Modify Down Scaler');
	$mode = 2;
	if($fsave){
		//print_r($_POST);
		for($i=0;$i<8;$i++){
			$md = "mod".$i;
			$nm = "dssel".$i;
			$nr = "dsrate".$i;
			if($_POST[$md]){
				$gto->open();
				$gto->dssel($i, $_POST[$nm]);
				$gto->dsrate($i, $_POST[$nr]);
				$data = $gto->read();
				$gto->close();
			}
		}
	}

}else if(isset($_POST['definemode'])){
	$html->link('Back to the top page', $thislink);
	$html->br();

	$html->heading(3, 'Define Input Channel Name');
	$mode = 3;
	if($fsave){
		//var_dump($_POST);
		for($i=0;$i<$gto::NOFIN;$i++){
			$md = "mod".$i;
			$nm = "name".$i;
			if($_POST[$md]){
				$namexml->input[$i]->name = $_POST[$nm];
			}
		}
		for($i=20;$i<20+$gto::NOFOUT;$i++){
			$md = "mod".$i;
			$nm = "name".$i;
			if(isset($_POST[$md])){
				$namexml->output[$i-20]->name = $_POST[$nm];
			}
		}
		$fp = fopen($namefile, "w");
		fwrite($fp, $namexml->asXML());
		fclose($fp);
	}
}else if(isset($_POST['lockmode'])){
	$html->link('Back to the top page', $thislink);
	$html->br();

	$html->heading(3, 'Lock output channel');
	$mode = 4;
	if($fsave){
		for($i=0;$i<$gto::NOFOUT;$i++){
			$md = "mod".$i;
			$nm = "lock".$i;
			if($_POST[$md]){
				if($_POST[$nm] == 1){
					$lk = 'yes';
				}else{
					$lk = 'no';
				}
					
				$namexml->output[$i]->lock = $lk;
			}
		}
		//var_dump($_POST);

		$fp = fopen($namefile, "w");
		fwrite($fp, $namexml->asXML());
		fclose($fp);
	}
}else{
	$html->link('Back to the index page', $indexlink);
	$html->br();

	$mode = 0;
}

$gto->decode($data);


$col[0] = '#ffeeff';
$col[1] = '#ffffee';

// Display mode
$html->form_start($thislink);
$html->button_refresh();
echo "<br>\n";
echo "<br>\n";
// Default
if($mode == 0){
	for($i=0;$i<8;$i++){
		$html->table_start_collapse($col[$i%2]);
		echo "<td>Output ".$i;
		echo "<tr><td>\n";
		if($weblock == 0 && !strcmp($namexml->output[$i]->lock, 'no')){
			echo $html->button_generic('modifymode', $i, 'Modify');
		}
		echo "<td width=100><b>".$namexml->output[$i]->name."</b>\n";
		for($j=0;$j<$gto::NOFPARAM;$j++){
			if($gto->chkout($i, $j)){
				echo $html->td_border($html::BDUPPER, 100).$gto->seladv[$j]['param']."\n";
			}
		}
		echo "<tr><td>&nbsp;<td>";
		for($j=0;$j<$gto::NOFPARAM;$j++){
			if($gto->chkout($i, $j)){
				$disp = $gto->displayname($j, $namexml);
				echo $html->td_border($html::BDLOWER, 100).$disp."\n";
			}
		}

		$html->table_end();
	}
}else if($mode == 1){ // Output Select Mode
	echo $html->button_save();
	$id = 0;

	$html->table_start('');
	echo "<tr>\n";
	echo "<td><td>Output ".$modnum."\n";
	echo "</tr>\n";
	
	//Checkbox
	for($j=0;$j<$gto::NOFPARAM;$j++){
		echo "<tr>\n";
		if($gto->chkout($modnum, $j)){
			$chk = 1;
		}else{
			$chk = 0;
		}
			
		echo $html->checkbox_td($gto->seladv[$j]['param'], $chk);
		echo "<td>".$gto->seladv[$j]['param']."\n";
		echo "<td>".$gto->displayname($j, $namexml);
		echo "</tr>\n";
	}
	$html->table_end();
	echo $html->hidden('modifymode', $modnum);
	$html->button_save();
	$html->br(); $html->br();
	$html->input_modify($id);

}else if($mode == 2){ // DS Mode
	$html->button_save();
	$id = 0;

	// Option
	$opt = array();
	for($i=0;$i<20;$i++){
		$k = $gto->seladv[$i]['param'];
		$v = $gto->seladv[$i]['param'].":".$namexml->input[$i]->name;
		$opt[$k] = $v;
	}
	$opt['50M'] = '50M clock';

	$html->table_start();
	echo "<tr bgcolor='#ccffcc'><td><td>Input<td>Divide<td></tr>\n";
	for($i=0;$i<8;$i++){
		echo "<tr><td>";
		echo $gto->seladv[$i+$gto::NOFIN]['param'];
		echo $html->select_tdid('dssel', $id, $opt, $gto->selectedds($i), 1);
		echo $html->text_tdid('dsrate', $gto->rate[$i], $id, 10);
		echo "<td>".$gto->convdivide($gto->rate[$i]);
		echo "</tr>\n";
		$id++;
	}
	$html->table_end();


	echo $html->hidden('dsmode', 'ds');
	$html->button_save();
	$html->br();
	$html->br();
	$html->input_modify($id);
	echo "<p><pre>The range of the divide rate is 0-16777215 (16M).\n";
	echo "Can use 'k' and 'M' marks, e.g. 1k=1000, 1M=1000000.\n";
	echo "Divie = 0 is through (no rate downscale)\n";
	echo "</pre></p>";
}else if($mode == 3){ // Define Name
	$html->button_save();
	$id = 0;
	$html->table_start();
	for($i=0;$i<$gto::NOFIN;$i++){
		echo "<tr><td>";
		$prm = $gto->seladv[$i]['param'];
		echo $prm;

		$nm = $namexml->input[$i]->name;
		echo $html->text_tdid("name", $nm, $id, 10, '');

		if($i<$gto::NOFOUT){
			$onm = '';
			if(isset($namexml->output[$i]->name)){
				$onm = $namexml->output[$i]->name;
			}
			echo "<td>&nbsp;&nbsp;&nbsp<td>out".$i;
			echo $html->text_tdid("name", $onm, $id+20, 10, '');
		}

		$id++;
	}
	$id += 8;

	$html->table_end();
	echo $html->hidden('definemode', 'define');
	$html->input_modify($id);
	$html->button_save();
}else if($mode == 4){ // Lock mode
	$html->button_save();
	$id = 0;

	for($i=0;$i<8;$i++){
		$html->table_start_collapse($col[$i%2]);
		echo "<td>Output ".$i;
		echo "<tr><td>Lock\n";
		if(!strcmp($namexml->output[$i]->lock, 'yes')){
			$chk = 1;
		}else{
			$chk = 0;
		}
		echo "<td width=100><b>".$namexml->output[$i]->name."</b>\n";
		for($j=0;$j<$gto::NOFPARAM;$j++){
			if($gto->chkout($i, $j)){
				echo $html->td_border($html::BDUPPER, 100).$gto->seladv[$j]['param']."\n";
			}
		}
		echo "<tr>";
		$nm = 'lock'.$id;
		echo $html->checkbox_td($nm, $chk);
		echo "<td>";
		for($j=0;$j<$gto::NOFPARAM;$j++){
			if($gto->chkout($i, $j)){
				$disp = $gto->displayname($j, $namexml);
				echo $html->td_border($html::BDLOWER, 100).$disp."\n";
			}
		}

		$html->table_end();
		$id++;
	}
	echo $html->hidden('lockmode', 'lock');
	$html->br(); $html->br();
	$html->input_modify($id);
	
}


if($mode == 0 && $weblock == 0){
	$html->br();
	echo $html->button_generic('dsmode','ds','Modify Down Scaler');
	$html->br();
	$html->br();
	echo $html->button_generic('definemode','define','Define Channel Name');
	$html->br();
	$html->br();
	echo $html->button_generic('EEPWrite');
	echo $html->button_generic('EEPRead');
	$html->br();
	$html->br();

	echo $html->button_generic('lockmode', 'lock', 'Lock Output');
	$html->br();
	$html->br();

	echo $html->button_generic('WebLock');
}

if($mode == 0 && $weblock == 1){
	$html->br();
	$html->br();
	echo $html->button_generic('WebUnLock');
}

$html->br();
$html->br();
$html->button_refresh();
$html->script('formcol.js');
$html->form_end();


$html->std_html_ender();

?>
