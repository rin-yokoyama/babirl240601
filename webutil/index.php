<?php
$mkdefmode = 0;
$title = '';
$lkfile = 'dat/conflock';

if(!($userdef = simplexml_load_file("./dat/userdef.xml"))){
  $mkdefmode = 1;
 }else{
  $title = $userdef->title;
 }

echo "<html>\n";
echo "<title>$title Index page</title>\n";
echo "<body>\n";
echo "<h1>$title Index page</h1>\n";
//echo "<ul><li><a href="./babirunsum.php">RUN Summary</a></ul>\n";

$locking = 0; // variable for the lock the configuration
if(file_exists($lkfile)){
  $locking = 1;
}

if($mkdefmode){
  echo "Please edit user definition file : ";
  echo "<a href='./edituserdef.php'>edituserdef.php\n";
 }else{
  echo "<p><h3>DAQ Tool</h3>\n";
  echo "<ul>\n";
  echo "<li><a href='./babixst.php'>DAQ Status Monitor</a>\n";
  echo "<li><a href='./babiscr.php'>DAQ Scaler Monitor</a>\n";
  echo "<li><a href='./babipm.php'>DAQ Process Manager</a> (for advanced user)\n";
  echo "<li><a href='./expdbrunsum.php'>Run summary</a>\n";
  echo "<li><a href='./expdbscaler.php'>Scaler configurator</a>\n";
  echo "<font size=-1>From here, parameters in DB are changed only.</font>\n";
  echo "<font size=-1>To reflest on DAQ, dbgetscr command from babicon is required.</font>\n";
  echo "</ul>\n";
  echo "<ul><li>To retrive scaler values, please see <a href=http://ribf.riken.jp/RIBFDAQ/index.php?DAQ%2FManual%2FUtility#lfa1eddb>here</a></ul>";
	echo "<ul>\n";
  echo "<li><a href='./vmestat/webvsta.php'>Web VME Stat</a><font color=red><i>New</i></font>\n";
  echo "</ul>\n";
  
  $arserver = array();

  $mkhostlist = 0;
  if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
    $mkhostlist = 1;
  }

  if($mkhostlist){
    echo "<b><font color=red>Please edit hostlist : </font></b>";
    echo "<ul><li><a href='edithostlist.php'>Edit host list</a>\n";
    echo "<li><a href='edithostinfo.php'>Edit host information</a>\n";
    echo "<li><a href='./edituserdef.php'>Edit user definition</a></ul>\n";
  }else{
    echo "<ul>\n";
    foreach($hostlist->host as $node){
      if(!isset($node->disabled)){
	if($node->babild){
	  echo "<li><a href='rdmhst/rdmhst.php?server=";
	  echo "$node->name'>Raw data monitor ($node->name)</a>";
	}
      }
    }
    echo "</ul>\n";

    // Confifurators
    if(!$locking){
      echo "<ul>\n";
      foreach($hostlist->host as $node){
	if(!isset($node->disabled)){
	  if(isset($node->babild)){
	    echo "<li><a href='babixconfig.php?host=";
	    echo "$node->name'>Web configurator ($node->name)</a>";
	    echo "<li><a href='editbabildlist.php?host=";
	    echo "$node->name'>Edit configuration ($node->name)</a>";
	  }
	}
      }
      echo "</ul>\n";
      //echo "<ul>\n";
      //echo "<li><a href='editinitfile.php'>Edit initialization for front-ends</a>\n";
      //echo "</ul>\n";
      echo "<ul>\n";
      echo "<li><a href='edithostlist.php'>Edit host list</a>\n";
      echo "<li><a href='edithostinfo.php'>Edit host information</a>\n";
      echo "<li><a href='edituserdef.php'>Edit user definition</a>\n";
      echo "<li><a href='showxml.php'>Show XML</a>\n";
      echo "</ul>\n";
    }

    echo "<ul>\n";
    if($locking){
      echo "<li><a href='lockconfig.php'>Unlock the configuration</a>\n";
    }else{
      echo "<li><a href='lockconfig.php'>Lock the configuration</a>\n";
    }
    echo "</ul>\n";
  }

  echo "<h3>Link</h3>\n";
  echo "<ul>\n";
  foreach($userdef->link as $link){
    echo "<li><a href='".$link->url."'>".$link->text."\n";
  }
  echo "</ul>\n";
 }

?>

</body>
</html>
