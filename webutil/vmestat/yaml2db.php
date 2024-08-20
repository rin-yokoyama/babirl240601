<?php

require_once('../class/WEBVSTA.class.php');


if($argc != 3){
	echo "yaml2db.php INPUTFILE OUTPUTFILE\n";
	die();
}

$yaml = Spyc::YAMLLoad($argv[1]);
if(count($yaml) == 1){
	echo "Invalid file ".$argv[1]."\n";
	die();
}

$db = new BBSQLite($argv[2]);
if(!$db){
	echo "Cannot open ".$argv[2];
	die();
}

CYamlUtil::yaml2sq3($yaml, $db);

unset($db);


?>

