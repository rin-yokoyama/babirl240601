<html>
<head>
<title>Show XML file</title>
</head>
<body>

<h2>Show XML file</h2>
<a href="./index.php">Back to index page</a>

<p>
<table>
<?php
$tdname   = "<td bgcolor='#99ffff'>";
$tdefn    = "<td bgcolor='#99cccc'>";
$tdhalt   = "<td bgcolor='#9999ff'>";
$tdreboot = "<td bgcolor='#999999'>";
$tdinit   = "<td bgcolor='#66ffcc'>";
$tdbabinfo= "<td bgcolor='#9966ff'>";
$tdbabild = "<td bgcolor='#99ff66'>";
$tdbabiau = "<td bgcolor='#cccc99'>";
$tdbabies = "<td bgcolor='#9999cc'>";
$tdbabissm= "<td bgcolor='#ff33ff'>";
$tdbabian = "<td bgcolor='#ffff99'>";
$tddescription = "<td bgcolor='#ddffff'>";
$tdcomment = "<td bgcolor='#ffddff'>";

echo "<tr>";
echo $tdname . "Name";
echo $tdefn . "EFN";
echo $tdhalt . "Halt";
echo $tdreboot . "Reboot";
echo $tdinit . "Init";
echo $tdbabinfo . "babinfo";
echo $tdbabild . "babild";
echo $tdbabild . "opt";
echo $tdbabild . "efn";
echo $tdbabiau . "babiau";
echo $tdbabiau . "opt";
echo $tdbabiau . "efn";
echo $tdbabies . "babies";
echo $tdbabies . "opt";
echo $tdbabies . "efn";
echo $tdbabissm . "babissm";
echo $tdbabian . "babian";
echo $tddescription . "description";
echo $tdcomment . "comment";
echo "\n";

if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
  die();
 }

foreach($hostlist->host as $node){
  $path = "./xml/" . $node->name . ".xml";
  $host = simplexml_load_file($path);
  
  echo "<tr>";
  echo $tdname . $node->name;
  echo $tdefn . $host->efn;
  echo $tdhalt . $host->halt;
  echo $tdreboot . $host->reboot;
  echo $tdinit . $host->init;
  echo $tdbabinfo . $host->babinfo->onoff;
  echo $tdbabild . $host->babild->onoff;
  echo $tdbabild . $host->babild->option;
  echo $tdbabild . $host->babild->efn;
  echo $tdbabiau . $host->babiau->onoff;
  echo $tdbabiau . $host->babiau->option;
  echo $tdbabiau . $host->babiau->efn;
  echo $tdbabies . $host->babies->onoff;
  echo $tdbabies . $host->babies->option;
  echo $tdbabies . $host->babies->efn;
  echo $tdbabissm . $host->babissm->onoff;
  echo $tdbabian .  $host->babian->onoff;
  echo $tddescription . $host->description;
  echo $tdcomment . $host->comment;
  echo "\n";
}

?>
</p>
</body>
</html>
