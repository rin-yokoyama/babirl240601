<html>
<head><title>Lock/Unlock the configuration</title>
</head>
<body>
<h3>Lock/Unlock the configuration</h3>
<form method='POST' action='lockconfig.php'>
<?php

$lkfile = 'dat/conflock';

if(isset($_POST['lk'])){
  $lk = $_POST['lk'];
  if(!strcmp($lk, 'Unlock')){
    unlink($lkfile);
    echo "<font color='red'>Unlocked</font><br>\n";
  }

  if(!strcmp($lk, 'Lock')){
    touch($lkfile);
    echo "<font color='blue'>Locked</font><br>\n";
  }
}


if(file_exists('dat/conflock')){
  echo "<input type='submit' name='lk' value='Unlock'>";
  echo "<input type='submit' name='lk' value='Cancel'>";
}else{
  echo "<input type='submit' name='lk' value='Lock'>";
  echo "<input type='submit' name='lk' value='Cancel'>";
}

?>
</form>

<br>
<a href='index.php'>Back to the index page</a>

</body>
</html>
