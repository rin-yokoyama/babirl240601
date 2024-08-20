<?php

class BBYaml {
  
  function yaml2array($yaml){
		$ident = 0;
		$ret = array();
		$rar[$ident] = array();
		$sar[$ident] = $ident;
		$kar[$ident] = '';

		$linen = 0;

    $lines = explode("\n", $yaml);
    foreach($lines as $line){
			$linen++;
			//printf("linen=%d  ident=%d\n", $linen, $ident);
			if(preg_match("/^#/", $line)){
				continue;
			}
      $line = rtrim($line, "\r");
      $line = rtrim($line, "\n");
			if(!strlen($line)){
				continue;
			}
      $cont = preg_replace('/^ +/', '', $line);
			$spn = strpos($line, $cont);

			if($spn < $sar[$ident]){
				while($spn != $sar[$ident]){
					if(isset($rar[$ident])){
						//$rar[$ident-1][$kar[$ident]] = $rar[$ident];
						$tar = array($kar[$ident] => $rar[$ident]);
						if($ident == 1){
							array_push($ret, $tar);
						}else{
							array_push($rar[$ident-1], $tar);
						}
						unset($rar[$ident]);
						unset($kar[$ident]);
						unset($sar[$ident]);
					}
					$ident --;
				}
			}

			if($spn > $sar[$ident]){
				$ident++;
			}

			if(preg_match('/^-/', $cont)){
				$cont = preg_replace('/^-/', '', $cont);
				$cont = preg_replace('/^ +/', '', $cont);
				$cont = preg_replace('/ +$/', '', $cont);
				array_push($rar[$ident], $cont);
			}else if(preg_match('/:/', $cont)){
				$kv = explode(':', $cont);
				$kv[0] = preg_replace('/ +$/', '', $kv[0]);
				$kv[1] = preg_replace('/^ +/', '', $kv[1]);

				// same or next level
				//echo "ident = ".$ident."\n";
				if(!strlen($kv[1])){
					$kar[$ident+1] = $kv[0];
					$rar[$ident+1] = array();
					//echo "new ar, ident=".$ident." spn=".$spn."\n";
				}else{
					//$tar = array($kv[0] => $kv[1]);
					//array_push($rar[$ident], $tar);
					$rar[$ident][$kv[0]] = $kv[1];
					//$rar[$ident][$kv[0]] = $kv[1];
					//echo "add cont ident=".$ident." $spn=".$spn."\n";
				}
			}
			$sar[$ident] = $spn;
			//printf("xx spn=%d sar[ident]=%d ident=%d\n",
			//				 $spn, $sar[$ident], $ident);
		}
		
		while($ident){
			if(isset($rar[$ident])){
				//print_r($rar[$ident]);
				//echo "key=".$kar[$ident]."\n";
				$tar = array($kar[$ident] => $rar[$ident]);
				if($ident == 1){
					array_push($ret, $tar);
				}else{
					array_push($rar[$ident-1], $tar);
				}
				//$rar[$ident-1][$kar[$ident]] = $rar[$ident];
				unset($rar[$ident]);
				unset($kar[$ident]);
				unset($sar[$ident]);
			}
			$ident --;
		}

		return $ret;
  }
  
	function file2array($file){
		$yaml = file_get_contents($file);
		return $this->yaml2array($yaml);
  }

}

?>
