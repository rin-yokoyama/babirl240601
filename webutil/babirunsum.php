<html>
<head>
<title>babirl RUN Summary</title>
</head>
<body>
<h1>babirl RUN Summary</h1>
<a href="./index.php">Back to index page</a>

<p><?php

# data[0] Run name
# data[1] Run number
# data[2] Start time
# data[3] Stop time
# data[4] Date
# data[5] Header
# data[6] Ender
# data[7] Size

if(!($alldata = file_get_contents("./runinfo.csv", "r"))){
  die();
 }

$alldata2 = split("\n", $alldata);

echo "<table>\n";

$n = -1;
foreach ($alldata2 as $line){
  $tdata = split("\"", $line);

  if(count($tdata) != 17) {continue;}

  for($i=0;$i<8;$i++){
    $data[$i] = $tdata[1+$i*2];
  }

  $n++;
  if($n == 0) continue;
#  if($n % 10 == 1){
    echo "<tr bgcolor=lightblue><td>$data[1]<td>File<td>Size<td>Date<td>Start<td>Stop<td>Time\n";
#  }

  $filename = $data[0].$data[1].".ridf";

  $sta = strtotime($data[4]." ".$data[2]);
  $sto = strtotime($data[4]." ".$data[3]);
  if($sta > $sto){
    $sto += 60 * 60 * 24;
  }
  $rtime = $sto - $sta;
  $timeh = (int)($rtime / 3600);
  $timem = (int)(($rtime - $timeh * 3600)/60);
  $times = $rtime - $timeh * 3600 - $timem * 60;
  $time  = sprintf("%dh%2dm%2ds",$timeh, $timem, $times);

  $date   = date('Y/m/d', $sta);
  $stastr = date('H:i:s', $sta);
  $stostr = date('H:i:s', $sto);


  echo "<tr bgcolor='#ffffcc'><td><td>$filename<td align=right>$data[7] MB";
  echo "<td>$date<td>$stastr<td>$stostr<td>$time";
  echo "<tr><td  bgcolor='#ffccff'>Header<td colspan=6>$data[5]";
  echo "<tr><td bgcolor='#ffccff'>Ender<td colspan=6>$data[6]";
  echo "<tr><td>&nbsp;\n";
 }

echo "</table>\n";
?></p>
</body>
</html>
