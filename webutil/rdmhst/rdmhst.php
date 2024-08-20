<?php

## require ##
require_once('../class/bbhtml.class.php');
require_once('../class/babinfo.class.php');
require_once('../class/ridf.class.php');
##

#$title = 'RawDataMonitor + Hist';
$title = 'RawDataMonitor (development)';

$html = new Cbbhtml();
$html->std_html_header($title);
$html->heading(2, $title);
$html->hr();

$server = '';
$file = '';
// check server name
if(isset($_GET['server'])){
  $server = $_GET['server'];
}else if(isset($_GET['file'])){
  $file = $_GET['file'];
}else if($argc == 2){
  $server = $argv[1];
}else{
  echo 'server=hostname or file=filename is not given';
  $html->std_html_ender();
  die();
}

if($server){
  $babinfo = new Cbabinfo($server);
  $blkn = $babinfo->get_blknum();
  $buff = $babinfo->get_rawdata();
}else{
  $ridf = new CRIDF($file);
  if(!$ridf->fd){
    echo 'cannot open file='.$file;
    $html->std_html_ender();
    die();
  }
  $buff = $ridf->get_rawdata();
  $buff = $ridf->get_rawdata();
}

$parse = new CParseRIDF($buff);
echo "<tt>";
$parse->show_html_table();
echo "</tt>";


$html->std_html_ender();

?>
