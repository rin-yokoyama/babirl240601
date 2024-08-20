<?php

class BBHtml{
  public $week = array('日','月','火','水','木','金','土');

  function contenttype(){
    header("Content-Type: text/html; charset=utf-8");
  }

  function html($n = NULL){
    if(!$n){
      echo "<html>\n";
    }else{
      echo "</html>\n";
    }
  }

  function head($n = NULL){
    if(!$n){
      echo "<head>\n";
    }else{
      echo "</head>\n";
    }
  }

  function body($n = NULL){
    if(!$n){
      echo "<body>\n";
    }else{
      echo "</body>\n";
    }
  }

  function title($title = "Title"){
    echo "<title>".$title."</title>\n";
  }

  function heading($lv = "1", $val = "H1"){
    echo "<h".$lv.">".$val."</h".$lv.">\n";
  }

  function hr($wid=NULL){
    echo "<hr";
    if($wid){
      echo " width=".$wid;
    }
    echo ">\n";
  }

  function script($file){
    echo "<script src='".$file."' type='text/javascript'></script>\n";
  }

  function std_html_header($title){
    $this->contenttype();
    $this->html(0);
    $this->head(0);
    $this->title($title);
    $this->head(1);
    $this->body(0);
    echo "<p>\n";
  }

  function std_html_ender(){
    echo "</p>\n";
    $this->body(1);
    $this->html(1);
  }


  // Form functions
  function form_start($file){
    echo "<form action='".$file."' method='post' autocomplete='off'>";
  }

  function form_end(){
    echo "</form>\n";
  }
  
  function button_descending(){
    echo "<button type='submit' name='descending' value='descending'>Descending</button>\n";
  }

  function button_generic($name, $value='', $vname=''){
    $sname = strtolower($name);
    if(!strlen($value)){
      $value = $sname;
    }
    if(!strlen($vname)){
      $vname = $name;
    }
    $ret = "<button type='submit' name='".$sname."' value='";
    $ret .= $value."'>".$vname."</button>\n"; 
    return $ret;
  }
  
  function button_refresh(){
    echo $this->button_generic('Refresh');
  }
  
  function button_return(){
    echo $this->button_generic('Return');
  }
  
  function button_save(){
    echo $this->button_generic('Save');
  }
  
  function button_update($id){
    echo "<button type='submit' name='update";
    echo "' value='".$id."'>Update</button>\n";
  }
  
  function button_show($id){
    echo "<button type='submit' name='show";
    echo "' value='".$id."'>Show</button>\n";
  }

  function button_disabled($name){
    return "<button type='submit' disabled>".$name."</button>\n";
  }

  function button_link($title, $li, $st=''){
    echo "<input type='button' ".$st;
    echo " onclick=\"location.href='".$li."'\" value='".$title."'>\n";
  }

  
  function table_start($col = '', $arg=''){
    if(strlen($col)){
      $color = " bgcolor=".$col;
    }else{
      $color = "";
    }
    echo "<table".$color." ".$arg.">\n";
  }
  
  function table_end(){
    echo "</table>\n";
  }

  function tr_start($col = ''){
    if(strlen($col)){
      $color = " bgcolor=".$col;
    }else{
      $color = '';
    }
    echo "<tr".$color.">\n";
  }

  function tr_end(){
    echo "</tr>\n";
  }

 function input_modify($n){
  for($i=0;$i<$n;$i++){
    echo "<input type=hidden name=mod".$i." value=0>";
  }
}

 function modify_check($post){
   $ret = array();
   while(list($key, $val) = each($post)){
     $n = ereg_replace("[a-zA-Z]", "", $key);
     if(!strncmp($key, "mod", 3)){
       if($val > 0){
				 array_push($ret, $n);
       }
     }
   }
   return $ret;
 }
 
 function save_check($post){
   if(isset($post['save'])){
     return 1;
   }
   return 0;
 }

 function generic_check($post, $chk){
   if(isset($post[$chk])){
     return 1;
   }
   return 0;
 }

 function generic_check_value($post, $chk){
   if(isset($post[$chk])){
     return $post[$chk];
   }
   return -1;
 }
 
 function update_check($post){
   if(isset($post['update'])){
     return $post['update'];
   }
   return -1;
 }
 
 function show_check($post){
   if(isset($post['show'])){
     return $post['show'];
   }
   return -1;
 }
 
 
 function text_tdid($name, $val, $id, $size, $dis='', $tx=''){
   if($size > 0){
     $sz = 'size='.$size;
   }else{
     $sz = '';
   }

   if(!strcmp($dis, 'readonly')){
     $dis = "readonly style='background-color:#ccccdd'";
   }
   
   $ret = "<td>".$tx;
   $ret .= "<input ".$dis." type='text' ".$sz." value='".$val."' name='".$name.$id."'>";
   return $ret;
 }

 function date_tdid($name, $val, $id, $size=0, $dis=''){
   if($size > 0){
     $sz = 'size='.$size;
   }else{
     $sz = '';
   }
   $ret = "<td><input type='date' ".$sz." value='".$val."' ".$dis;
   $ret .= " name='".$name.$id."'>";
   return $ret;
 }

 function time_tdid($name, $val, $id, $size=0){
   if($size > 0){
     $sz = 'size='.$size;
   }else{
     $sz = '';
   }
   $ret = "<td><input type='time' ".$sz." value='".$val."' name='".$name.$id."'>";
   return $ret;
 }

	function number_tdid($name, $val, $r, $id, $size){
   if($size > 0){
     $sz = 'size='.$size;
   }else{
     $sz = '';
   }

	 if(strlen($r)){
		 $m = explode(":", $r);
	 }else{
		 $m[0] = 0;
		 $m[1] = 16777215;
	 }

   $ret = "<td>";
   $ret .= "<input type='number' ".$sz." value='".$val."'";
	 $ret .= " min=".$m[0]." max=".$m[1]." name='".$name.$id."'>";
   return $ret;
 }
 
	function checkbox_tdid($name, $val, $id, $vv='1'){
		$ret = "<td><input type='checkbox' name='".$name.$id."' value='".$vv."' ";
		if($val == 1){
			$ret .= "checked";
		}
		$ret .= ">";
		return $ret;
	}

	function checkbox_disp($c){
		if($c == 1){
			return "<input type=checkbox disabled checked>";
		}else{
			return "<input type=checkbox disabled>";
		}
	}

 function hidden_id($name, $val, $id){
   $ret = "<input type='hidden' value='".$val."' name='".$name.$id."'>";
   return $ret;
 }
 
 
 function hidden($name, $val){
   $ret = "<input type='hidden' value='".$val."' name='".$name."'>";
   return $ret;
 }
 
 
 function input_password($psd){
   echo "<input type='password' name='password' value='".$psd."'>";
 }
 

 function debug_print($t, $e){
   echo "<b><font color=green>".$t."</font>=<font color=blue>".$e."</font></b><br>\n";
 }

 function std_form_header($thisfile, $jp='', $cus=''){
   $this->form_start($thisfile);
   if(!strlen($jp) && !strlen($cus)){
     $this->button_save();
     $this->button_refresh();
   }else if(strlen($cus)){
     echo $this->button_generic('save', 'save', $cus);
     echo $this->button_generic('reflesh', 'reflesh', 'Refresh');
   }else{
     echo $this->button_generic('save', 'save', '登録');
     echo $this->button_generic('reflesh', 'reflesh', 'クリア');
   }
 }

 function std_form_ender($jp='', $jsdir=null, $cus=''){
   if(!strlen($jp) && !strlen($cus)){
     $this->button_save();
     $this->button_refresh();
   }else if(strlen($cus)){
     echo $this->button_generic('save', 'save', $cus);
     echo $this->button_generic('reflesh', 'reflesh', 'Refresh');
   }else{
     echo $this->button_generic('save', 'save', '登録');
     echo $this->button_generic('reflesh', 'reflesh', 'クリア');
   }
   $this->form_end();
   if($jsdir==null){
     $this->script('class/formcol.js');
   }else{
     $this->script($jsdir.'/formcol.js');
   }     
 }

 function link_str($text='', $url=''){
   if(!strlen($url)) $url = $text;
   return "<a href='".$url."'>".$text."</a>\n";
 }

 function link($text='', $url=''){
   echo $this->link_str($text, $url);
 }



 function ul($n = 0){
   if($n){
     echo "</ul>\n";
   }else{
     echo "<ul>\n";
   }
 }

 function li($n = 0){
   if($n){
     echo "</li>\n";
   }else{
     echo "<li>\n";
   }
 }

 function chksel($opt, $sel){
   if(!strcmp($opt, $sel)){
     return 'selected';
   }
   return '';
 }

	function radio_tdid($name, $chk, $id, $opt, $sel, $val=''){
		$ret = '';
		if(!strlen($val)){
			$val = $opt;
		}
		if($chk == 1){
			$ccc = 'checked';
		}else{
			$ccc ='';
		}

		$ret .= "<td><input type='radio' name='".$name.$id."' value='".$val;
		$ret .= "' ".$ccc.">";
		$ret .= "<td>".$opt."\n";

		return $ret;
	}

 function select_tdid($name, $id, $opt, $sel, $size = '', $blank = ''){
   if($size > 0){
     $sz = 'size='.$size;
   }else{
     $sz = '';
   }
   $ret = "<td><select ".$sz." name='".$name.$id."'>";
   if(strlen($blank)){
     if(!strlen($sel)){
       $ret .= "<option></option>";
     }
   }

   $keys = array_keys($opt);
   foreach($keys as $op){
     $sl = $this->chksel($op, $sel);
     $ret .= "<option ".$sl." value='".$op."'>".$opt[$op];
   }
   $ret .= "</select></td>\n";
   return $ret;
 }

 function td_selcolor($color, $val){
   $bg = '';
   if(isset($color[$val])){
     $bg = " bgcolor='".$color[$val]."' ";
   }
   return "<td".$bg.">".$val;
 }

 function errormsg($text){
   echo "<font color='red'>".$text."</text><br>\n";
 }

 function daytime2date($day, $time){
   $datetime = new DateTime($day);
   $w = (int)$datetime->format('w');
   $ret = $datetime->format('m/d');
   $ret .= "(".$this->week[$w].") ".$time."-";
   return $ret;
 }


 function find_optsel($opt, $sel){
   $ret = '';
   $keys = array_keys($opt);
   foreach($keys as $op){
     if(!strcmp($op, $sel)){
       $ret = $opt[$op];
     }
   }
   return $ret;
 }
 

 function select_dualtdid($name, $id, $pid, $cid, $opt, $sel, $pwid = '',
			  $cwid = '', $blank = ''){
   if($pwid > 0){
     $pst = " style='width:".$pwid."px'";
   }else{
     $pst = '';
   }
   if($cwid > 0){
     $cst = " style='width:".$cwid."px'";
   }else{
     $cst = '';
   }

   $ret = "<td><select ".$pst." name='' id='".$pid.$id."'>\n";
   if(strlen($blank)){
     if(!strlen($sel)){
       $ret .= "<option></option>\n";
     }
   }

   $sels = explode('_', $sel);
   foreach ($opt as $k => $v){
     $sl = BBHtml::chksel($k, $sels[0]);
     $ret .= "<option ".$sl." value='".$k."'>".$k."</option>\n";
   }
   $ret .= "</select>\n";

   $ret .= "<select ".$cst." name='".$name.$id."' id='".$cid.$id."'>\n";
   foreach ($opt as $k => $v){
     $ret .= "<optgroup label='".$k."'>\n";
     asort($v);
     $ret .= "<option value=''></option>\n";
     foreach($v as $d){
       $nm = $k."_".$d[0];
       $sl = BBHtml::chksel($nm, $sel);
       $ret .= "<option ".$sl." value='".$nm."'>".$d[0]." : ".$d[1]."</option>\n";
     }
   }
   $ret .= "</select></td>\n";
   return $ret;
 }
 
 function select_connect($pid, $cid, $n){
   echo "<script type='text/javascript'>\n";
   for($i=0;$i<$n;$i++){
     $p = $pid.$i;
     $c = $cid.$i;
     echo "ConnectedSelect(['".$p."', '".$c."']);\n";
     echo "ConnectedSelectDefault(['".$p."', '".$c."']);\n";
   }
   echo "</script>\n";
 
 }

 function select_grptdid($name, $id, $opt, $sel, $wid = '', $blank = ''){
   if($wid > 0){
     $st = "style='width:".$wid."px'";
   }else{
     $st = '';
   }

   $ret = "<td><select ".$st." name='".$name.$id."'>\n";
   if(strlen($blank)){
     $ret .= "<option value='delete'>delete</option>\n";
   }else{
     $ret .= "<option value=''></option>\n";
   }
   foreach ($opt as $k => $v){
     $ret .= "<optgroup label='".$k."'>\n";
     asort($v);
     foreach($v as $d){
       $nm = $k."_".$d[0];
       $sl = BBHtml::chksel($nm, $sel);
       $ret .= "<option ".$sl." value='".$nm."'>".$nm." : ".$d[1]."</option>\n";
     }
   }
   $ret .= "</select></td>\n";
   return $ret;
 }


	function fileupload_textarea($name, $id='tmpid', $ini='', $rows=30, $cols=70){
		echo "<textarea name='".$name."' rows='".$rows;
		echo "' cols='".$cols."' id='".$name."'>\n";
		echo $ini;
		echo "</textarea>\n";
		echo "<script>\n";
		echo "  var obj1 = document.getElementById('".$id."');\n";
		echo "  obj1.addEventListener('change',function(evt){\n";
		echo "    var file = evt.target.files;\n";
		echo "    var reader = new FileReader();\n";
		echo "    reader.readAsText(file[0]);\n";
		echo "    reader.onload = function(ev){\n";
		echo "    var txt = document.getElementById('".$name."');\n";
		echo "    txt.value = reader.result;\n";
		echo "  }},false);\n";
		echo "</script>\n";
	}

	function clear_textarea($btid, $txtid, $btlabel){
		echo "<input type='button' id='".$btid."' value='".$btlabel."'>\n";
		echo "<script>\n";
		echo "  var btn = document.getElementById('".$btid."');\n";
		echo "  btn.addEventListener('click',function(evt){\n";
		echo "    var reset_target = document.getElementById('".$txtid."');\n";
		echo "    reset_target.value = '';\n";
		echo "  },false);\n";
		echo "</script>\n";
	}

	function pr($n){
		echo $n."<br>\n";
	}


} # end of BBHtml

?>
