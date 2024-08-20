<?php

function form_start($file){
  echo "<form action='".$file."' method='post' autocomplete='off'>";
}

function form_end(){
  echo "</form>";
}

function button_descending(){
  echo "<button type='submit' name='descending' value='descending'>Descending</button>\n";
}

function button_refresh(){
  echo "<button type='submit' name='refresh' value='refresh'>Refresh</button>\n";
}

function button_return(){
  echo "<button type='submit' name='return' value='return'>Return</button>\n";
}

function button_save(){
  echo "<button type='submit' name='save' value='save'>Save</button>\n";
}

function button_update($id){
  echo "<button type='submit' name='update";
  echo "' value='".$id."'>Update</button>\n";
}

function button_show($id){
  echo "<button type='submit' name='show";
  echo "' value='".$id."'>Show</button>\n";
}

function button_delete($id){
  echo "<button type='submit' name='delete";
  echo "' value='".$id."'>Delete</button>\n";
}

function button_delete_yes(){
  echo "<button type='submit' name='deleteyes";
  echo "' value='deleteyes'>Delete</button>\n";
}


function table_start($col){
  if(strlen($col)){
    $color = " bgcolor=".$col;
  }else{
    $color = "";
  }
  echo "<table".$color.">\n";
}

function table_end(){
  echo "</table>\n";
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

function delete_check($post){
  if(isset($post['delete'])){
    return $post['delete'];
  }
  return -1;
}

function deleteyes_check($post){
  if(isset($post['deleteyes'])){
    return $post['deleteyes'];
  }
  return -1;
}

function text_tdid($name, $val, $id, $size){
  if($size > 0){
    $sz = 'size='.$size;
  }else{
    $sz = '';
  }
  $ret = "<td><input type='text' ".$sz." value='".$val."' name='".$name.$id."'>";
  return $ret;
}

function text_disable_tdid($name, $val, $id, $size){
  if($size > 0){
    $sz = 'size='.$size;
  }else{
    $sz = '';
  }
  $ret = "<td><input type='text' disabled ".$sz." value='".$val."' name='".$name.$id."'>";
  return $ret;
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
?>

