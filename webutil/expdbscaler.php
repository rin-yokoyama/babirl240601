<html>
<head>
<title>EXPDB Scaler</title>
</head>
<body>
<?php
require "expdblib.php";

$thisfile = 'expdbscaler.php';
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

##### for post #####
$deln = -1;
$yesn = -1;
$post = $_POST;
$mode = 0; // show exp and scalerlist
$shn = show_check($post);
$deln = delete_check($post);
$yesn = deleteyes_check($post);

if($yesn != -1){
  $deln = $yesn;
};

if($deln != -1 && $yesn == -1){
  $shn = $deln;
}

if(isset($post['return'])){
  $mode = 0;
}else if($shn >= 0 || isset($post['showchannel'])){
  $mode = 1; // show scaler channel
  if($shn >= 0){
    $scrid = $post['ScalerID'.$shn];
    $scrname = $post['Name'.$shn];
  }else{
    $scrid = $post['showchannel'];
    $scrname = $post['showname'];
  }
}

$upn = update_check($post);
$psd = "";
if(isset($_POST['password'])){
  $psd = $_POST['password'];
}


$fdelete = -1;
if($mode == 0){
  # for scaler/exp update
  if($upn >= 0){
    if(!strlen($psd)){
      echo "<font color=red>password is required</font><br>\n";
    }else{
      $modn = modify_check($post);
      foreach ($modn as $n){
	if($n == 0 && $upn == 0){
	  $texp = getexp_post($post, $n);
	  $msg = updateexplist($server, $texp, $psd);
	}else if($n == $upn){
	  $tscr = getscr_post($post, $n);
	  $msg = updatescrlist($server, $expid, $tscr, $psd);
	}
      }
    }
  }
}else{
  # for channel update
  $fsave = save_check($post);
  if($fsave){
    if(!strlen($psd)){
      echo "<font color=red>password is required</font><br>\n";
    }else{
      $modn = modify_check($post);
      foreach ($modn as $n){
	$tch = getchannel_post($post, $n);
	$msg = updatechannellist($server, $expid, $scrid, $tch, $psd);
      }
    }
  }


  $fdelete = deleteyes_check($post);
  if($fdelete != -1){
    if(!strlen($psd)){
      echo "<font color=red>password is required</font><br>\n";
      $fdelete = -1;
    }else{
      $msg = deletescrinfo($server, $expid, $scrid, $psd);
    }
  }    
}



# number of forms
$linen = 0;


##### print html header #####
$explist = getexplist($server, $expid);

echo "<h1>EXPDB Scaler (".$explist[0]->param['Name'].")</h1>\n";
echo "<a href='index.php'>back to index</a>\n<p>\n";

form_start($thisfile);
if($mode == 1){
  echo hidden('showchannel', $scrid);
  echo hidden('showname', $scrname);
  button_return();
  echo "<br>\n";
  echo "<h3>Scaler for ".$scrname." (ID=".$scrid.")</h3>\n";

  ## check delete mode
  if(($deln != -1) && ($fdelete == -1)){
    echo "<b><font color=red> Delete this scaler ?</font></b><br><br>\n";
  }else if(($deln != -1) && ($fdelete != -1)){
    echo "<b><font color=blue> Scaler ID=".$scrid." has been deleted.</font></b><br><br>\n";
  }

}
if($deln == -1){
  button_refresh();
}
if($mode == 1){
  if($deln == -1){
    button_save();
  }else{
    if($fdelete == -1){
      button_delete_yes($scrid);
    }else{
      button_return();
    }
  }
}
input_password($psd);

if($mode == 0){
  table_start("");
  echo "<tr bgcolor=#ccffcc>".$explist[0]->tablehead()."</tr>\n";
  echo "<tr>".$explist[0]->tableeditscaler($linen)."<td>";
  button_update($linen);
  echo "</tr>\n";
  $linen++;
  table_end();
  
  ##### for scaler ######
  echo "<br>\n";
  $scrlist = getscalerlist($server, $expid);
  table_start("");
  echo "<tr bgcolor=#ffcccc>".$scrlist[0]->tablehead()."</tr>\n";
  foreach($scrlist as $scr){
    echo "<tr>".$scr->tableeditscaler($linen)."<td>";
    button_update($linen);
    button_show($linen);
    echo "&nbsp;"; echo "&nbsp;"; echo "&nbsp;"; echo "&nbsp;";
    button_delete($linen);
    $linen++;
    echo "</tr>\n";
  }
  table_end();
}else if($mode == 1 && $fdelete == -1){
  # show scaler channel
  $chlist = getchannellist($server, $expid, $scrid);
  table_start("");
  echo "<tr bgcolor=#ffcccc>".$chlist[0]->tablehead()."</tr>\n";
  foreach($chlist as $ch){
    if($deln == -1){
      echo "<tr>".$ch->tableeditchannel($linen)."<td>";
    }else{
      echo "<tr>".$ch->tabledisablechannel($linen)."<td>";
    }
    $linen++;
    echo "</tr>\n";
  }
  table_end();
}

input_modify($linen);
if(!$fdelete){
  button_refresh();
}
form_end();
?>
</p>
<script src="./formcol.js" type="text/javascript"></script>
</body>
</html>
