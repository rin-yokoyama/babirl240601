<html>
<head>
<title>EXPDB Run Summary</title>
</head>
<body>
<?php
require "expdblib.php";

$thisfile = 'expdbrunsum.php';
$server = 'ribfdb';

$rcfile = "dat/expdb.xml";

##### Check expid #####
if(!($expxml = simplexml_load_file($rcfile))){
  echo "<font color=red>Configuration file ".$rcfile." is required!</font><br>";
  die();
}
if(!isset($expxml->expid)){
  echo "<font color=red>expid tag is required in the configuration file ".$rcfile. "</font><br>\n";
  die();
}else{
  $expid = $expxml->expid;
}

if(isset($expxml->server)){
  $server = $expxml->server;
}

##### for post ######
$post = $_POST;
$desc = 0;  // sort runid

if(isset($post['descending'])){
  $desc = 1;
}

##### print html header #####

$explist = getexplist($server, $expid);

echo "<h1>EXPDB Run Summary (".$explist[0]->param['Name'].")</h1>\n";
echo "<a href='index.php'>back to index</a>\n<p>\n";

form_start($thisfile);
button_refresh();
button_descending();

$runlist = getrunlist($server, $expid, $desc);
table_start("");
$n = 0;
foreach($runlist as $run){
  if($n % 10 == 0){
    echo "<tr bgcolor=#ffccff>".$runlist[0]->tablehead()."</tr>\n";
  }
  echo "<tr>".$run->tableshowrun()."</tr>";
  $n++;
}
table_end();

button_refresh();
button_descending();
form_end();

?>
</p>
<script src="./formcol.js" type="text/javascript"></script>
</body>
</html>
