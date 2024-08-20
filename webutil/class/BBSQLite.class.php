<?php

class BBSQLite{
  private $db;

  function __construct($file){
    if(!file_exists($file)){
      //echo 'No DB file '.$file;
      //return NULL;
    }
    try{
      $this->db = new PDO('sqlite:'.$file);
      $this->db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    }catch(PDOException $e){
      echo 'Cannnot open : '.$e->getMessage();
      return NULL;
    }
  }

  function __destruct(){
    unset($this->db);
  }

  function begin(){
    $this->db->beginTransaction();
  }

  function commit(){
    $this->db->commit();
  }

  function exec($sql = ''){
    $this->db->exec($sql);
  }

  function query($sql = ''){
    if(!$sql) return NULL;
    $sql = mb_convert_encoding($sql, 'UTF-8', 'auto');

    try {
      $ret = $this->db->query($sql);
    }catch(PDOException $e){
      print $e->getMessage();
      return NULL;
    }

    return $ret;
  }

  function allrow($sth, $col){
    if(!$sth) return NULL;
    while($line = $sth->fetch(PDO::FETCH_ASSOC)){
      echo $line[$col]."\n";
    }
  }

  function select($val='',$tab='',$wh='', $od=''){
    if(!$val || !$tab) return '';
    $ret = 'select '.$val.' from '.$tab;
    if($wh){
      $ret .= ' where '.$wh;
    }
    if($od){
      $ret .= ' order by '.$od;
    }

    return $ret;
  }

  function fetchall_assoc($sth){
    return $sth->fetchAll(PDO::FETCH_ASSOC);
  }

  function query_assoc($sql){
    $sth = $this->query($sql);
    return $this->fetchall_assoc($sth);
  }

  function sql_insert($table, $ar, $idname='', $id=''){
    $n = count($ar);
    $sql = "insert into ".$table." (";
    if(strlen($idname) && strlen($id)){
      $sql .= $idname.", ";
    }
    $keys = array_keys($ar);
    for($i=1;$i<$n;$i++){
      $sql .= $keys[$i];
      if($i<$n-1){
				$sql .= ", ";
      }
    }
    $sql .= ") values (";
    if(strlen($idname) && strlen($id)){
      $sql .= "'".$id."', ";
    }
    for($i=1;$i<$n;$i++){
      $sql .= "'".$ar[$keys[$i]]."'";
      if($i<$n-1){
	$sql .= ", ";
      }
    }
    $sql .= ")";
    return $sql;
  }

  function sql_update($table, $ar){
    $n = count($ar);
    $sql = "update ".$table." set ";
    $keys = array_keys($ar);
    for($i=1;$i<$n;$i++){
      $sql .= $keys[$i]." = '".$ar[$keys[$i]]."'";
      if($i<$n-1){
				$sql .= ", ";
      }
    }
    $sql .= " where ".$keys[0]." = '".$ar[$keys[0]]."'";
    return $sql;
  }

  function sql_delete($table, $wh){
    $sql = "delete from ".$table." where ". $wh;
    return $sql;
  }

  function sql_drop($table){
    $sql = "drop table if exists ".$table;
    return $sql;
  }
  


  } # end of BBSQLite



?>
