<html>
<title>Make user definition file</title>
<body>
<h2>Make user definition file</h2>
<A href='./index.php'>Back to index page</a>
<p>
<?php
$title = '';
$linkurl = array();
$linktext = array();

$post = $_POST;

$save = 0;
$userfile = './dat/userdef.xml';

$dom = new DomDocument('1.0');
$userdef = $dom->appendChild($dom->createElement('userdef'));

while(list($key, $val) = each($post)){
  //  echo "$key $val<br>\n";
  if(!strcmp($key, 'save')){
    $save = 1;
  }else if(!strcmp($key, 'title')){
    $userdef->appendChild($dom->createElement('title', $val));
  }
  for($i=0;$i<5;$i++){
    $ti = "text".$i;
    $ui = "url".$i;
    if(!strcmp($key, $ti)){
      if(!isset($link[$i]) && $val){
	$link[$i] = $userdef->appendChild($dom->createElement('link'));
      }
      if($val){
	$link[$i]->appendChild($dom->createElement('text', $val));
      }
    }
    if(!strcmp($key, $ui)){
      if(!isset($link[$i]) && $val){
	$link[$i] = $userdef->appendChild($dom->createElement('link'));
      }
      if($val){
	$link[$i]->appendChild($dom->createElement('url', $val));
      }
    }
  }
 }

if($save){
  $dom->formatOutput = true;
  $dom->save($userfile);
  chmod($userfile, 0666);
  echo "<font color='red'>Saved</font>\n";
 }

$mkdefmode = 0;
if(!($userdef = simplexml_load_file($userfile))){
  $mkdefmode = 1;
  $userdef = 0;
 }

$thisfile = basename($_SERVER['SCRIPT_NAME']);

echo "<form action='".$thisfile."' method='post'>\n";
echo "<button type='submit' name='refresh'>Refresh</button> \n ";
echo "<button type='submit' name='save'>Save</button> \n ";
echo "<table><tr bgcolor='#33ffff'><td>Title\n";
if($userdef){
  $title = $userdef->title;
 }else{
  $title = '';
 }
echo "<tr><td><input type=text size=30px name=title value='".$title."'>\n";
echo "</table>\n";

echo "<table><tr bgcolor='#33ffff'><td>Link\n";
echo "<tr bgcolor='#ffccff'><td>Comment<td>URL\n";

if($mkdefmode){
  for($i=0;$i<2;$i++){
    $li = "text".$i;
    $ui = "url".$i;
    echo "<tr><td><input type=text size=30px name=".$li." value=''>\n";
    echo "<td><input type=text size=50px name=".$ui." value=''>\n";
  }
 }else{
  $mi = count($userdef->link);
  for($i=0;$i<$mi;$i++){
    $li = "text".$i;
    $ui = "url".$i;
    echo "<tr><td><input type=text size=30px name=".$li." value='".$userdef->link[$i]->text."'>\n";
    echo "<td><input type=text size=50px name=".$ui." value='".$userdef->link[$i]->url."'>\n";
  }
  $li = "text".$i;
  $ui = "url".$i;
  echo "<tr><td><input type=text size=30px name=".$li." value=''>\n";
  echo "<td><input type=text size=50px name=".$ui." value=''>\n";
 }

echo "</table>\n";
echo "</form>\n";
?>

</p>
</body>
</html>
