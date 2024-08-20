<?php

require_once('BBHtml.class.php');
require_once('BBSQLite.class.php');
require_once('Spyc.php');

class CDBUtil {
  const CRTABLE = "create table if not exists ";

  const CRMLISTTABLE = " (
  ID         Integer Primary Key Autoincrement not null,
  Module     Text,
  Address    Text,
  Use        Integer,
  Comment    Text
  );";

  const CRMDATATABLE = " (
  Function   Text Primary Key not null,
  Sub        Text,
  Value      Text,
  Reserve    Text
  );";

  const SELMLISTBYMID = "select * from ModuleList ";

  public function sql_create_modulelist(){
    return self::CRTABLE."ModuleList".self::CRMLISTTABLE;
  }

  public function sql_create_moduledata($table){
    return self::CRTABLE.$table.self::CRMDATATABLE;
  }

  public function sql_select_modulelist_id($mid){
    return self::SELMLISTBYMID." where ID=".$mid;
  }

  public function sql_select_modulelist_mod($md){
		list($name, $addr) = explode("_at_", $md);
    return self::SELMLISTBYMID." where Module='".$name
															."' AND Address='"
															.$addr."'";
  }

	public function mod_to_insert($md, $cm){
		list($name, $addr) = explode("_at_", $md);
		$tup[0] = "";
		$tup['Module'] = $name;
		$tup['Address'] = $addr;
		$tup['Use'] = 1;
		$tup['Comment'] = $cm;

		return $tup;
	}

  public function sql_replace_moduledata($tab, $c){
    $ret = "REPLACE INTO ".$tab." VALUES ('";
    $ret .= $c['Function']."', '".$c['Sub']."', '";
    $ret .= $c['Value']."', '".$c['Reserve']."');";
    return $ret;
  }


  public function tdkeys($ar){
    $html = '';
    $keys = array_keys($ar);
    foreach($keys as $k){
      $html .= "<td>".$k;
    }
    $html .= "\n";
    return $html;
  }

  public function array2assoc($aar){
    $ret = array();
    foreach($aar as $ar){
      $keys = array_keys($ar);
      $tar = array($ar[$keys[0]] => $ar[$keys[1]]);
      $ret += $tar;
    }
    return $ret;
  }

  public function tabcolor($db, $table){
    $sql = $db->select('Name, Color', $table);
    $tar = $db->query_assoc($sql);
    return CDBUtil::array2assoc($tar);
  }

  public function sqlattendlist($db, $dateid, $planid){
    $sql = "select MemberList.FamilyName as FamilyName";
    $sql .= " from AttendanceList left outer join";
    $sql .= " MemberList on AttendanceList.MemberID = MemberList.MemberID";
    $sql .= " where AttendanceList.DateID = '".$dateid."'";
    $sql .= " and AttendanceList.PlanID = '".$planid."'";
    $ret = $db->query_assoc($sql);
    return $ret;
  }


  } // End of CDBUtil

class CDBParameter {
  public $param = array('ID'    => 2,
                        'Name'  => 2);
  public $table = 'Table';
  public $sort = 'Sort';
  public $sublist = 0;

  public function cleanparam(){
    $keys = array_keys($this->param);
    foreach($keys as $key){
      $this->param[$key] = '';
    }
  }

  public function copyparam($new){
    $keys = array_keys($this->param);
    foreach($keys as $key){
      if(isset($new[$key])){
        $this->param[$key] = $new[$key];
      }
    }
  }
  
  public function tablehead(){
    return CDBUtil::tdkeys($this->param);
  }
  public function tableedit($line, $id, $opt=null, $idx=''){
    $keys = array_keys($this->param);
    $html = "<tr>\n";
    if(!strlen($idx)){
      $html .= "<td>".$line[$keys[0]];
    }else{
      $html .= "<td>".$idx;
    }
    $html .= BBHtml::hidden_id($keys[0], $line[$keys[0]], $id);
    $paramn = count($this->param);
    for($i=1;$i<$paramn;$i++){
      if(!strcmp($keys[$i], "Use") || !strcmp($keys[$i], "PXE")
	 || !strcmp($keys[$i], "GRUB") || !strcmp($keys[$i], "DHCP")){
	$html .= BBHtml::checkbox_tdid($keys[$i], $line[$keys[$i]], $id)."\n";
      }else if(!strcmp($keys[$i], "Module")){
	$html .= BBHtml::select_grptdid($keys[$i], $id, $opt, $line[$keys[$i]], '300','delete');
      }else if(!strcmp($keys[$i], "Address")){
	$pt = "pattern='[0-9A-Fa-f]{8}' title='8-digit hexadecimal (e.g. 12340000)'";
	$html .= BBHtml::text_tdid($keys[$i], $line[$keys[$i]],
				   $id, $this->param[$keys[$i]], $pt,'0x',
				   '')."\n";
      }else if(!strcmp($keys[$i], 'Comment')){
				$html .= BBHtml::text_tdid($keys[$i], $line[$keys[$i]],
																	 $id, $this->param[$keys[$i]], 'readonly')."\n";
      }else{
				$html .= BBHtml::text_tdid($keys[$i], $line[$keys[$i]],
																	 $id, $this->param[$keys[$i]])."\n";
      }
    }

    //    $html .= "</tr>\n";
    return $html;
  }

  public function tableadd($id, $opt=null){
    $keys = array_keys($this->param);
    $html = "<tr>\n";
    $html .= "<td>";
    $html .= BBHtml::hidden_id($keys[0], "", $id);
    $paramn = count($this->param);
    for($i=1;$i<$paramn;$i++){
      if(!strcmp($keys[$i], "Use") || !strcmp($keys[$i], "PXE")
	 || !strcmp($keys[$i], "GRUB")){
	$html .= BBHtml::checkbox_tdid($keys[$i], '1', $id)."\n";
      }else if(!strcmp($keys[$i], "Module")){
	$html .= BBHtml::select_grptdid($keys[$i], $id, $opt, '', '300','');
      }else if(!strcmp($keys[$i], "Address")){
	$pt = "pattern='[0-9A-Fa-f]{8}' title='8-digit hexadecimal (e.g. 12340000)'";
	$html .= BBHtml::text_tdid($keys[$i], '',
				   $id, $this->param[$keys[$i]], $pt,'0x')."\n";
      }else{
	$html .= BBHtml::text_tdid($keys[$i], '',
				   $id, $this->param[$keys[$i]])."\n";
      }
    }
    $html .= "</tr>\n";
    return $html;
  }

  public function tablelabel($line){
    $keys = array_keys($this->param);
    $html = "<tr>\n";
    $html .= "<td>".$line[$keys[0]];
    $paramn = count($this->param);
    for($i=1;$i<$paramn;$i++){
			$html .= "<td><label>".$line[$keys[$i]]."</label>\n";
    }
    $html .= "</tr>\n";
    return $html;
  }

  public function getpostlist($post, $x){
    $keys = array_keys($this->param);
    foreach($keys as $k){
      $chk = $k.$x;

      if(isset($post[$chk])){
        $this->param[$k] = $post[$chk];
      }else{
        $this->param[$k] = '';
      }
    }
    return $this->param;
  }

  public function list2idname($list, $sublist=0){
    $keys = array_keys($this->param);
    $ret = array();
    foreach($list as $li){
      if(isset($li[$keys[0]])){
        $ret[$li[$keys[0]]] = $li[$keys[1]];
        if($sublist){
          $ret[$li[$keys[0]]] .= " ".$li[$keys[$sublist]];
        }
      }
    }
    return $ret;
  }

  public function idname($db){
    $sql = $db->select('*', $this->table, '', $this->sort);
    $tlist = $db->query_assoc($sql);
    return $this->list2idname($tlist, $this->sublist);
  }


} // End of CDBParameter

class CModuleList extends CDBParameter {
  public $param = array('ID'          => 2,
                        'Module'      => 20,
                        'Address'     => 12,
												'Use'         => 2,
                        'Comment'     => 20);
  public $table = 'ModuleList';
  public $sort = 'Module';

  public $selcolor = array ('CAEN'    => '#bb2233',
			    'PC'      => '#33cc88');

  public function typecolor($type){
    $keys = array_keys($this->selcolor);
    $col = '#000000';
    foreach($keys as $k){
      if(!strcmp($type, $k)){
				$col = $this->selcolor[$k];
				break;
      }
    }
    return "<font color='".$col."'>".$type."</font>\n";
  }
  
} // End of CZoneList


class CYamlUtil {
	public function array2yaml($ar){
		return Spyc::YAMLDump($ar);
	}

	public function sq32yaml($db){
		$table = 'ModuleList';
		$id = 0;
		$sql = $db->select('*', $table, '', 'Module, Address');
		$list = $db->query_assoc($sql);
		
		$ar = array();
		foreach($list as $li){
			$tb = $li['Module']."_at_".$li['Address'];
			$sql = $db->select('*', $tb);
			$tl = $db->query_assoc($sql);
			$tar = array();
			if(isset($li['Comment'])){
				$tar['Comment'] = $li['Comment'];
			}
			foreach($tl as $f){
				$v = CYamlUtil::convval($f['Function'], $f['Value']);
				if($v != null)
					$tar[$f['Function']] = $v;
			}
			$ar[$tb] = $tar;
		}
		return CYamlUtil::array2yaml($ar);

	}

	public function yaml2sq3($yaml, $db){
		$db->query(CDBUtil::sql_create_modulelist());
		foreach($yaml as $md => $fn){
			$table = 'ModuleList';
			$res = $db->query_assoc(CDBUtil::sql_select_modulelist_mod($md));
			
			$cm = '';
			if(isset($fn['Comment'])){
				$cm = $fn['Comment'];
			}
			if(isset($res[0]['ID'])){
				$tup = $res[0];
				if(strlen($cm)){
					$tup['Comment'] = $cm;
				}
				$sql = $db->sql_update($table, $tup);
				$db->query($sql);
			}else{
				$tup = CDBUtil::mod_to_insert($md, $cm);
				$sql = $db->sql_insert($table, $tup);
				$db->query($sql);
			}
			
			$db->query(CDBUtil::sql_create_moduledata($md));
			foreach($fn as $k => $v){
				if(!strcmp($k, "Comment")){
					continue;
				}
				$v = intval($v, 0);
				$c['Function'] = $k;
				$c['Sub'] = '';
				$c['Value'] = $v;
				$c['Reserve'] = '';
				$db->query(CDBUtil::sql_replace_moduledata($md, $c));
			}
		}
	}

	public function convval($func, $val){
	  $ignore = array('reset', 'bitclear2', 'sleep0', 'sleep1', 'clear');
	  $tohex = array('bitset2', 'ctrl', 'mcst');
	  
	  foreach($ignore as $ig){
	    if(!strcmp($ig, $func)){
	      return null;
	    }
	  }
	  
	  foreach($tohex as $th){
	    if(!strcmp($th, $func)){
	      $tval = intval($val, 0);
	      $val = sprintf("0x%04x", $tval);
	      break;
	    }
	  }
	  
	  return $val;
	}

}

