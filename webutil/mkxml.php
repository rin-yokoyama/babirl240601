<?
$name = "ssm";
$file = $name + ".xml";

$efn = "33";
$halt = "on";
$reboot = "on";
$init = "on";
$babildoption = "";
$babildefn = "";
$babiesoption = "-s";
$babiesefn = "";

$description = "Start stop manager";
$comment = "";

$dom = new DomDocument('1.0');

$host = $dom->appendChild($dom->createElement('host'));

$host->appendChild($dom->createElement('name', $name));
if($efn){
  $host->appendChild($dom->createElement('efn', $efn));
 }
if($halt){
  $host->appendChild($dom->createElement('halt', $halt));
 }
if($reboot){
  $host->appendChild($dom->createElement('reboot', $reboot));
 }
if($init){
  $host->appendChild($dom->createElement('init', $init));
 }
if($description){
  $host->appendChild($dom->createElement('descrition', $description));
 }
if($comment){
  $host->appendChild($dom->createElement('comment', $comment));
 }


$babild = $host->appendChild($dom->createElement('babild'));
if($babildoption){
  $babild->appendChild($dom->createElement('option', $babildoption));
 }
if($babildefn){
  $babild->appendChild($dom->createElement('efn', $babildefn));
 }


$babies = $host->appendChild($dom->createElement('babies'));
if($babiesoption){
  $babies->appendChild($dom->createElement('option', $babiesoption));
 }
if($babiesefn){
  $babies->appendChild($dom->createElement('efn', $babiesefn));
 }




$dom->formatOutput = true;

echo $dom->saveXML();

?>
