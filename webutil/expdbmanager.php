<html>
<head>
<title>EXPDB Manager</title>
</head>
<body>
<h1>EXPDB Manager</h1>
<p>
<?php
require "expdblib.php";
//require "babiformlib.php";

$thisfile = 'expdbmanager.php';

$server = 'ribfdb';
$rcfile = "dat/expdb.xml";
if(($expxml = simplexml_load_file($rcfile))){
   if(isset($expxml->server)){
     $server = $expxml->server;
   }
}

#for post
$post = $_POST;

#var_dump($post);
$fsave = save_check($post);

$psd = "";
if(isset($_POST['password'])){
  $psd = $_POST['password'];
}

if($fsave){
  if(!strlen($psd)){
    echo "<font color=red>password is required</font>\n";
  }else{
    $modn = modify_check($post);
    foreach ($modn as $n){
      $texp = getexp_post($post, $n);
      $msg = updateexplist($server, $texp, $psd);
    echo "<pre>".$msg."</pre>";
    }
  }
}

# show list
$explist = getexplist($server, 0);
# number of forms
$linen = 0;
form_start($thisfile);
button_refresh();
button_save();
input_password($psd);
table_start("");
echo "<tr bgcolor=#ccffcc>".$explist[0]->tablehead()."</tr>\n";
foreach ($explist as $exp){
  echo "<tr>".$exp->tableeditall($linen)."</tr>\n";
  $linen++;
}
$nexp = array();
$nexp = new Explist;
echo "<tr>".$nexp->tableeditall($linen)."</tr>\n";
$linen++;
table_end();
input_modify($linen);

button_refresh();
button_save();
form_end();
?>
</p>
<script src="./formcol.js" type="text/javascript"></script>
</body>
</html>
