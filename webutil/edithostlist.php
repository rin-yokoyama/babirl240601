<html>
<head>
<title>Edit host list</title>
<SCRIPT LANGUAGE="JavaScript" SRC="selectbox.js">
</SCRIPT> 
<SCRIPT LANGUAGE="JavaScript">
   function setstr(com, txt, sel, btxt, bsel){
   var str = new String();
   var bstr = new String();

   for(var i=0;i<sel.options.length;i++){
     str = str + "," + sel.options[i].value;
   }
   txt.value = str;

   for(var i=0;i<bsel.options.length;i++){
     bstr = bstr + "," + bsel.options[i].value;
   }
   btxt.value = bstr;

   com.value = "save";

 }
</SCRIPT> 
</head>
<body>
<h1>babirl edit host list</h1>
<a href="./index.php">Back to index page</a>

<p>
<FORM action="edithostlist.php" method="post" name="edit">

<input type="hidden" name="poststr" value="aaa">
<input type="hidden" name="postbabild" value="bbb">
<input type="hidden" name="com" value="non">
<input type="submit" value="Save" onclick="setstr(this.form['com'], this.form['poststr'], this.form['selectedlist'], this.form['postbabild'], this.form['babildlist'])">
<input type="submit" value="Reset">

<?php
  require "babixlib.php";

if(isset($_POST['com'])){
  $com = $_POST['com'];
}else{
  $com = '';
 }

if(!strcmp($com, "save")){
  echo "<font color=red>Saved</font>\n";

  $st = split(",", $_POST['poststr']);
  $bst = split(",", $_POST['postbabild']);
  $file = "dat/hostlist.xml";

  $fnode = dom_loadfile_node($dom, $file, 'hostlist');

  $shost = simplexml_load_string('<hostlist></hostlist>');
  foreach($st as $nm){
    if(strlen($nm)){
      $th = $shost->addChild('host');
      $th->addChild('name', $nm);
    }
  }
  
  dom_append_simple_hostlist($dom, $shost);
  //dom_remove_simple_hostlist_tag($dom, $shost, 'babild');
  dom_remove_simple_babild($dom, $shost, $bst);
  foreach($bst as $nm){
    $th = dom_find_hostname($dom, $nm);
    if($th){
      dom_dup_append_tag($dom, $th, 'babild');
    }
  }

  dom_disable_simple_hostlist($dom, $shost);
  dom_sort_simple_hostlist($dom, $shost);
  dom_savefile($dom, $file);
 }
?>
</p>

<p>
<TABLE BORDER="0">
<tr><td>All list
<td><td>Seleted
<td><td>Babild
<TR><TD>
<SELECT NAME="allhostlist" MULTIPLE SIZE=20 onDblClick="copySelectedOptions(this.form['allhostlist'],this.form['selectedlist'],false);return false;"> 
<?php

$list = glob("./xml/*.xml");

foreach($list as $tname){
  if(strcmp($tname, "")){
    $nname = str_replace("./xml/","", $tname);
    $name = str_replace(".xml","", $nname);
    echo "<OPTION VALUE=\"$name\"> $name\n";
  }
}
?>
</SELECT> 
</TD> 
<TD VALIGN=MIDDLE ALIGN=CENTER width=80> 
  <INPUT TYPE="button" NAME="right" VALUE="&gt;&gt;" ONCLICK="copySelectedOptions(this.form['allhostlist'],this.form['selectedlist'],false)"><BR><BR> 
<INPUT TYPE="button" NAME="allright" VALUE="All &gt;&gt;" ONCLICK="copyAllOptions(this.form['allhostlist'],this.form['selectedlist'],false)"><BR><BR> 
<INPUT TYPE="button" NAME="left" VALUE="&lt;&lt;" ONCLICK="removeSelectedOptions(this.form['selectedlist'])"><BR><BR> 
<INPUT TYPE="button" NAME="allleft" VALUE="&lt;&lt; All" ONCLICK="removeAllOptions(this.form['selectedlist'])"><BR><BR> 
</TD> 

<TD>
<SELECT NAME="selectedlist" SIZE="20" MULTIPLE ondblCLICK="removeSelectedOptions(this.form['selectedlist'])">
<?php
if(!($hostlist = simplexml_load_file("./dat/hostlist.xml"))){
#  die();
  unset($hostlist);
}else{
  foreach($hostlist->host as $host){
    if(isset($host->name)){
      if(!isset($host->disabled)){
	echo "<OPTION VALUE=\"$host->name\">$host->name\n";
      }
    }
  }
 }

echo "</SELECT>";
?>

<TD VALIGN=MIDDLE ALIGN=CENTER width=80> 
  <INPUT TYPE="button" NAME="right" VALUE="&gt;&gt;" ONCLICK="copySelectedOptions(this.form['selectedlist'],this.form['babildlist'],false)"><BR><BR> 
<INPUT TYPE="button" NAME="allright" VALUE="All &gt;&gt;" ONCLICK="copyAllOptions(this.form['selectedlist'],this.form['babildlist'],false)"><BR><BR> 
<INPUT TYPE="button" NAME="left" VALUE="&lt;&lt;" ONCLICK="removeSelectedOptions(this.form['babildlist'])"><BR><BR> 
<INPUT TYPE="button" NAME="allleft" VALUE="&lt;&lt; All" ONCLICK="removeAllOptions(this.form['babildlist'])"><BR><BR> 
</TD> 

<td>
<SELECT NAME="babildlist" SIZE="20" MULTIPLE ondblCLICK="removeSelectedOptions(this.form['babildlist'])">
<?php

if(isset($hostlist)){
  foreach($hostlist->host as $host){
    if(isset($host->name)){
      if(!isset($host->disabled)){
	if(isset($host->babild)){
	  echo "<OPTION VALUE=\"$host->name\">$host->name\n";
	}
      }
    }
  }
}

?>
</SELECT> 


<tr><td><td>
<TD ALIGN="CENTER" VALIGN="MIDDLE" width=80> 
  <INPUT TYPE="button" VALUE="&nbsp;Up&nbsp;" onClick="moveOptionUp(this.form['selectedlist'])"> 
  <BR><BR> 
<INPUT TYPE="button" VALUE="Down" onClick="moveOptionDown(this.form['selectedlist'])"> <BR><BR> 
<INPUT TYPE="button" VALUE="Sort" onClick="sortSelect(this.form['selectedlist'])"> 
  </TD> 
</tr>
<td>
</TABLE> 

</FORM>
</p>


</body>
</html>
