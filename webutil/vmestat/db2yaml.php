<?php

require_once('../class/WEBVSTA.class.php');


if($argc != 2 && $argc != 3){
	echo "db2yaml.php INPUTFILE [OUTPUTFILE]\n";
	die();
}

$db = new BBSQLite($argv[1]);
if(!$db){
	echo "Cannot open ".$argv[1]."\n";
	die();
}

$yaml = CYamlUtil::sq32yaml($db);

unset($db);


if($argc == 3){
  $fp = fopen($argv[2], "w");
}else{
	$fp = STDOUT;
}

fprintf($fp, "%s", $yaml);

fclose($fp);

?>

