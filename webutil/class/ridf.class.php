<?php

require_once('segidlist.class.php');
require_once('moduleidlist.class.php');

function hdclass($buff){
  $list = unpack("I*", $buff);
  return ($list[1] & 0x0fc00000) >> 22;
  }

function hdsize($buff){
  $list = unpack("I*", $buff);
  return ($list[1] & 0x003fffff);
  }

class CRIDFHeader {
  public $hd0, $hd1;
  public $layer = -1, $classid=-1, $size=-1, $efn=-1;
  public $buff;
  protected $hcol = "#000011";
  protected $hname = "Gloval Heander";

  const EVENT_FLAGMENT          = 0;
  const EVENT_ASSEMBLY          = 1;
  const EVENT_ASSEMBLY_FLAGMENT = 2;
  const EVENT                   = 3;
  const SEGMENT                 = 4;
  const COMMENT                 = 5;
  const EVENT_WITH_TIMESTAMP    = 6;
  const UNKNOWN7                = 7;
  const BLOCK_NUMBER            = 8;
  const END_OF_BLOCK            = 9;
  const UNKNOWN10               = 10;
  const SLCAER_NON_CLEAR24      = 11;
  const SLCAER_CLEAR24          = 12;
  const SLCAER_NON_CLEAR32      = 13;
  const TIMESTAMP_DATA          = 16;
  const STATUS_DATA             = 21;


  function __construct($buff){
    $this->buff = $buff;
    $list = unpack("I*", $buff);
    $this->hd0 = $list[1];
    $this->hd1 = $list[2];
    $this->layer   = ($this->hd0 & 0x30000000) >> 28;
    $this->classid = ($this->hd0 & 0x0fc00000) >> 22;
    $this->size    = ($this->hd0 & 0x003fffff);
    $this->efn     = ($list[2] & 0xffffffff);

    switch ($this->classid){
    case self::EVENT_FLAGMENT:
      $this->hname = "Event Flagment";
      break;
    case self::EVENT_ASSEMBLY:
      $this->hname = "Event Assembly";
      break;
    case self::EVENT_ASSEMBLY_FLAGMENT:
      $this->hname = "Event Assembly Flagment";
      break;
    }
  }

  function show_rawdata(){
    echo "<font color=#119922>";
    printf("0x %08x %08x", $this->hd0, $this->hd1);
    echo "</font>";
  }

  function show_efn(){
    echo $this->efn;
  }

  function show_header(){
    echo "<font color=".$this->hcol."><b>".$this->hname."</b></font><br>\n";
    echo "ly=".$this->layer." cid=".$this->classid." size=".$this->size;
  }

  function show_content(){
  }

  function show_decode(){
  }

  // end of class header
  }

class CRIDFBlockNumberHeader extends CRIDFHeader {
  public $blkn = -1;
  protected $hcol = "#000011";
  protected $hname = "Block Number";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 4);
    $list = unpack("I*", $blk);
    $this->blkn = $list[1];
  }

  function show_rawdata(){
    parent::show_rawdata();
    printf(" %08x", $this->blkn);
  }

  function show_content(){
    printf("Block Number=%d", $this->blkn);
  }

}

class CRIDFEventHeader extends CRIDFHeader{
  public $evtn = -1;
  protected $hcol = "#1111ff";
  protected $hname = "Event";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 4);
    $list = unpack("I*", $blk);
    $this->evtn = $list[1];
  }

  function show_content(){
    printf("Event Number=%d", $this->evtn);
  }

  function show_rawdata(){
    parent::show_rawdata();
    printf(" %08x", $this->evtn);
  }
}

class CRIDFSegmentHeader extends CRIDFHeader{
  public $segid = -1, $revision = -1, $device = -1;
  public $focal = -1, $detector = -1, $module = -1;
  public $segidlist;
  protected $hcol = "#11cc33";
  protected $hname = "Segment";
  
  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 4);
    $list = unpack("I*", $blk);
    $this->segid = $list[1];

    $this->revision  = ($this->segid & 0xfc000000) >> 26;
    $this->device    = ($this->segid & 0x03f00000) >> 20;
    $this->focal     = ($this->segid & 0x000fc000) >> 14;
    $this->detector  = ($this->segid & 0x00003f00) >> 8;
    $this->module    = ($this->segid & 0x000000ff);

  }

  function show_rawdata(){
    parent::show_rawdata();
    printf(" %08x", $this->segid);

    $dt = substr($this->buff, 12);
    $sval = unpack("S*", $dt);
    for($i=1;$i<=$this->size-6;$i++){
      if(($i-1)%8 == 0){
	echo "<br>";
      }
      printf("%04x ", $sval[$i]);
    }
  }

  function show_content(){
    $segidlist = new CSegIDList();
    printf("SegmentID=0x%08x<br>", $this->segid);
    echo $segidlist->Device[$this->device]." (Dev=".$this->device.")<br>\n";
    echo $segidlist->Focal[$this->focal]." (FP=".$this->focal.")<br>\n";
    echo $segidlist->Detector[$this->detector]." (Det=".$this->detector.")<br>\n";
    echo $segidlist->Module[$this->module]." (Mod=".$this->module.")\n";
  }

  function show_decode(){
    $segidlist = new CSegIDList();
    $data = substr($this->buff, 12);
    $modidlist = new CModuleIDList();

    if(isset($modidlist->idlist[$this->module])){
      require_once($modidlist->idlist[$this->module]['file']);
      $mod = new $modidlist->idlist[$this->module]['class']();
      $mod->decode($data);
      $mod->show_decode();
    }
  }

}

class CRIDFCommentHeader extends CRIDFHeader{
  public $comid = -1, $date = -1;
  protected $hcol = "#cc0011";
  protected $hname = "Comment Header";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 8);
    $list = unpack("I*", $blk);
    $this->date = $list[1];
    $this->comid = $list[2];
  }

  function show_content(){
    printf("CommentID=%d<br>", $this->comid);
  }

  function show_decode(){
    $dec = substr($this->buff, 16);
    echo "<pre>".$dec."</pre>\n";
  }

}

class CRIDFEventTimeStampHeader extends CRIDFHeader{
  public $evtn = -1, $ts = -1;
  protected $hcol = "#1133ff";
  protected $hname = "Event with TS";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 24);
    $list = unpack("I*", $blk);
    $this->evtn = $list[1];
    $this->ts = ($list[2] | ($list[3]<<32))
      & 0x00ffffffffffffff;
  }

  function show_content(){
    printf("Event Number=%d<br>", $this->evtn);
    printf("TS=%u", $this->ts);
  }

  function show_rawdata(){
    parent::show_rawdata();
    $blk = substr($this->buff, 8, 24);
    $list = unpack("I*", $blk);
    printf(" %08x<br>", $list[1]&0x0ffffffff);
    printf(" %08x", $list[2]&0x0ffffffff);
    printf(" %08x", $list[3]&0x0ffffffff);
  }
}

class CRIDFEndOfBlockHeader extends CRIDFHeader {
  public $blocksize = -1;
  protected $hcol = "#003322";
  protected $hname = "End of Block";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 8);
    $list = unpack("I*", $blk);
    $this->blocksize = $list[1];
  }

  function show_content(){
    printf("Block Size=%d", $this->blocksize);
  }

}

class CRIDFScalerHeader extends CRIDFHeader {
  public $scalerid = -1, $date = -1;
  protected $hcol = "#883399";
  protected $hname = "Scaler";

  function __construct($buff){
    parent::__construct($buff);
    $blk = substr($this->buff, 8, 8);
    $list = unpack("I*", $blk);
    $this->date = $list[1];
    $this->scalerid = $list[2];
  }

  function get_scaler(){
    $blk = substr($this->buff, 16);
    $list = unpack("I*", $blk);
    return $list;
  }

  function show_content(){
    printf("ScalerID=%d", $this->scalerid);
  }

  function show_rawdata(){
    parent::show_rawdata();
    $blk = substr($this->buff, 16);
    $list = unpack("I*", $blk);
    print "<br>";
    for($i=1;$i<count($list)+1;$i++){
      printf("%08x ", $list[$i] & 0x0ffffffff);
      if($i%4 == 0){
	print "<br>";
      }
    }
  }

  function show_decode(){
    $blk = substr($this->buff, 16);
    $list = unpack("I*", $blk);
    echo "<pre><tt>";
    for($i=1;$i<count($list)+1;$i++){
      printf("%12u ", $list[$i]);
      if($i%4 == 0){
	print "\n";
      }
    }
    echo "</tt></pre>";
  }
}

class CRIDFGeneric extends CRIDFHeader {
}
  

class CParseRIDF {
  public $buff;
  public $parse = array();
  public $size = 0;

  function __construct($buff){
    $idx = 0;
    $this->buff = $buff;

    // for global header
    $hdb = substr($this->buff, 0, 8);
    $hd = new CRIDFHeader($hdb);
    array_push($this->parse, $hd);
    $this->size = $hd->size;

    $idx += 8;

    while($idx < ($this->size * 2)){
      $hdb = substr($this->buff, $idx, 8);
      $cid = hdclass($hdb);
      $csz = hdsize($hdb) * 2;
      $cbf = substr($this->buff, $idx, $csz);
      switch($cid){
      case CRIDFHeader::EVENT_FLAGMENT:
      case CRIDFHeader::EVENT_ASSEMBLY:
      case CRIDFHeader::EVENT_ASSEMBLY_FLAGMENT:
	$hd = new CRIDFHeader($hdb);
	$idx += 8;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::EVENT:
	$hd = new CRIDFEventHeader($cbf);
	$idx += 12;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::SEGMENT:
	$hd = new CRIDFSegmentHeader($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::COMMENT:
	$hd = new CRIDFCommentHeader($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::EVENT_WITH_TIMESTAMP:
	$hd = new CRIDFEventTimeStampHeader($cbf);
	$idx += 20;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::BLOCK_NUMBER:
	$hd = new CRIDFBlockNumberHeader($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::END_OF_BLOCK:
	$hd = new CRIDFEndOfBlockHeader($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::SLCAER_NON_CLEAR24:
      case CRIDFHeader::SLCAER_CLEAR24:
      case CRIDFHeader::SLCAER_NON_CLEAR32:
	$hd = new CRIDFScalerHeader($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      case CRIDFHeader::TIMESTAMP_DATA:
      case CRIDFHeader::STATUS_DATA:
	$hd = new CRIDFGeneric($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      default:
	$hd = new CRIDFGeneric($cbf);
	$idx += $csz;
	array_push($this->parse, $hd);
	break;
      }
    }
  }

  function show_html_table(){
    echo "<table><tr><td>Header<td>EFN<td>Rawdata<td>Content<td>Decode</tr>\n";
    foreach($this->parse as $line){
      echo "<tr><td>";
      $line->show_header();
      echo "<td>";
      $line->show_efn();
      echo "<td>";
      $line->show_rawdata();
      echo "<td>";
      $line->show_content();
      echo "<td>";
      $line->show_decode();
      echo "</tr>\n";
    }
    echo "</table>\n";
  }

    // end of parse ridf
}


// obtain data
class CRIDF {
  public $fd = NULL;
  private $idx = 0;

  function __construct($file){
    if(file_exists($file)){
      $this->fd = fopen($file, "rb");
    }else{
      $this->fd = NULL;
    }
  }

  function __destruct(){
    if($this->fd){
      fclose($this->fd);
    }
  }

  function get_rawdata(){
    $hd = fread($this->fd, 8);
    $sz = hdsize($hd)*2 - 8;
    $bf = fread($this->fd, $sz);
    return $hd.$bf;
  }

}

