# Header Rule
#   2bit      2bit     6bit       22bit
#| Reserve | Layer | Class ID | Block Size |
#|                  EFN                    |
#                  32bit
require 'babirl'

class RIDFheader
  RIDF_LY0 = 0
  RIDF_LY1 = 1
  RIDF_LY2 = 2
  RIDF_LY3 = 3

  RIDF_EF_BLOCK   = 0
  RIDF_EA_BLOCK   = 1
  RIDF_EAEF_BLOCK = 2
  RIDF_EVENT      = 3
  RIDF_SEGMENT    = 4
  RIDF_COMMENT    = 5
  RIDF_EVENTTS    = 6
  RIDF_BLOCK_NUMBER = 8
  RIDF_END_BLOCK    = 9
  RIDF_SCALER     = 11
  RIDF_CSCALER    = 12
  RIDF_NCSCALER32 = 13
  RIDF_TIMESTAMP  = 16
  RIDF_STATUS     = 21

  RIDF_COMMENT_TEXT  = 0
  RIDF_COMMENT_RUNINFO_ASC = 1
  RIDF_COMMENT_EFINFO_ASC  = 2
  RIDF_COMMENT_DAQINFO_BIN = 3
  RIDF_COMMENT_RUNINFO_BIN = 4

  def initialize(d1,d2)
    @hd1 = d1
    @hd2 = d2
  end
  def layer
    return ((@hd1 & 0x30000000) >> 28) 
  end
  def class
    return ((@hd1 & 0x0fc00000) >> 22) 
  end
  def size
    return  (@hd1 & 0x003fffff)
  end
  def efn
    return  (@hd2 & 0xffffffff)
  end
end

class RIDFSegid
# Segid Rule
#   6bit      6bit     6bit      6 bit      8 bit
#| Reserve | Device |  Focal | Detector | Module |
  load 'segid.def'
  def initialize(segid)
    @segid = segid
    @device = Array.new
    @focal = Array.new
    @detector = Array.new
    @module = Array.new
#   self.instance_eval(File.read("segid.rb"))
  
  end
  def device
    deviceid = ((@segid >> 20) & 0x0000003f)
    string = eval "RIDFSegid::Device_#{deviceid}"
    return deviceid,string
  end
  def focal
    focalid = (@segid >> 14) & 0x0000003f
    string = eval "RIDFSegid::Focal_#{focalid}"
    return focalid,string
  end
  def detector
    detectorid = (@segid >> 8) & 0x0000003f
    string = eval "RIDFSegid::Detector_#{detectorid}"
    return detectorid,string
  end
  def module
    moduleid = (@segid) & 0x000000ff
    string = eval "RIDFSegid::Module_#{moduleid}"
    return moduleid,string
  end
end

class RIDFSegdata
  @efn = 0
  @size = 0 
end

class RIDFEvent
  def initialize(eventblock)
    @segmnet = Array.new
    @segment.push RIDFSegdata.new()
  end
end

class RIDFdata
  WORDSIZE = 4  # 4 byte
  ONLINE = true
  OFFLINE = false
  def initialize(filename)
    @filename = filename
    @rawdata = Array.new
    @blkn = 0
    @type = 0
    @point = 0
  end
  def open
    if (self.type) then
      babinfo = Babinfo.new(@filename)
      @netdata = babinfo.get_rawdata
#      @netdata.unpack("S*").each {|data|
#        printf("%04x ",data)
#      }
#      @point = 0
    else
      @fd = File.open(@filename,"rb")
      return @fd
    end
  end
  def close
    if !(self.type) then
      @fd.close()
    end
  end
  def read(wsize,string)
    datasize= (WORDSIZE*wsize).to_i
    data = ""
    if (self.type) then
      data = @netdata[@point,datasize]
      @point += datasize
    else
      data = @fd.read(datasize)
    end
    return false if data == nil
    rdata = data.unpack(string)[0]
    crawdata = data.unpack("S*")
    crawdata.each {|adata|
      @rawdata.push adata
    }
    return rdata
  end
  def readarray(wsize,string)
    datasize= (WORDSIZE*wsize).to_i
    if (self.type) then
      data = @netdata[@point,datasize]
      @point += datasize
    else
      data = @fd.read(datasize)
    end
    rdata = data.unpack(string)
#    @rawdata += rdata
    return rdata
  end
  def readhead
    d1 = self.read(1,"L")
    d2 = self.read(1,"L")
    return false if (d1 == nil)||(d2 == nil)
    header = RIDFheader.new(d1,d2)
    return header
  end
  def read_cdata(size)
    date = self.read(1,"L")
    id = self.read(1,"L")
    datasize = (size - 8)/2
    data = self.read(datasize,"A*")
    return data
  end 
  def read_sdata(size)
    date = self.read(1,"L")
    id = self.read(1,"L")
    datasize = (size - 8)/2
    data = self.read(datasize,"A*")
    return date, id
  end 
  def read_segdata(size)
    segid = self.read(1,"L")
    datasize = ((size.to_f - 6.to_f)/2).to_f
    dataarray= self.readarray(datasize,"S*")
    return segid,dataarray
  end
  def read_tsdata(size)
    segid = self.read(1,"L")
    datasize = size/2
    dataarray= self.readarray(datasize,"S*")
    return dataarray
  end
  attr_accessor :rawdata, :type
end
