require 'socket'

class Babirl
  WORDSIZE = 2
  def get(command)
    @conn = TCPSocket.open(@host,@port)
    size = command.size/WORDSIZE
    @conn.write([size].pack("I*"))
    @conn.write([command].pack("I*"))
    size = @conn.recv(4,Socket::MSG_WAITALL)
    result = @conn.recv(size.unpack("I*")[0],Socket::MSG_WAITALL)
    @conn.close
    return result
  end
end

class Babild < Babirl
  PORT = 17511
  EB_GET_DAQINFO = 1
  EB_GET_RUNINFO = 3
  EB_GET_EFNUM   = 5
  EB_GET_EFLIST  = 6
  EB_GET_MTLIST  = 13
  EB_GET_HDLIST  = 17
  EB_GET_SSMINFO = 51
  STAT_RUN_IDLE  = 0
  STAT_RUN_START = 1
  STAT_RUN_NSSTA = 2
  STAT_RUN_WAITSTOP = 3

  EFLIST_TEMPLATE = "I2A80A80"
  HDLIST_TEMPLATE = "I2A80Q3"
  MTLIST_TEMPLATE = "I2A80Q1"
  DAQINFO_TEMPLATE = "A80I3" + EFLIST_TEMPLATE*256 + "I" +
    HDLIST_TEMPLATE * 2 + "I" +  MTLIST_TEMPLATE
  def initialize(host)
    @host = host
    @port = PORT
  end
  def get_runinfo
    rval = self.get(EB_GET_RUNINFO)
    (@runnumber,@runstatus,@startdate,@stopdate,@header,@ender) =
      rval.unpack("I4,A80,A80")
  end
  def get_daqinfo
    array = self.get(EB_GET_DAQINFO).unpack(DAQINFO_TEMPLATE)
    @runname  = array[0].to_s
    @runnumber = array[1].to_i
    @ebsieze   = array[2].to_i
    @efn       = array[3].to_i
    @eflist = Array.new
    aEflist = Struct.new("EFlist",:id,:ex,:of,:efname,:efhost)
    (0..255).each{ |index|
      ex = array[4+(index)*4]
      of = array[5+(index)*4]
      name = array[6+(index)*4]
      host = array[7+(index)*4]
      if (ex == 1) then
        eflist.push(aEflist.new(index,ex,of,name,host))
      end
    }
    @hdlist = Array.new
    aHdlist = Struct.new("Hdlist",:ex,:of,:path,:free,:full,:maxsize)
    (0..1).each {|index|
      ex = array[1029 + 6*index]
      of = array[1030 + 6*index]
      path = array[1031 + 6*index]
      free = array[1032 + 6*index]
      full = array[1033 + 6*index]
      maxsize = array[1034 + 6*index]
      hdlist.push(aHdlist.new(ex,of,path,free,full,maxsize))
    }
    @mtlist = Array.new
    aMtlist = Struct.new("Mtlist",:ex,:of,:path,:maxsize)
    (0..0).each {|index|
      ex = array[1041 + index]
      of = array[1042 + index]
      path = array[1043 + index]
      maxsize = array[1044 + index]
      mtlist.push(aMtlist.new(ex,of,path,maxsize))
    }
    return 
  end
  def runstatus
    case @runstatus
    when STAT_RUN_IDLE
      return 'IDLE'
    when STAT_RUN_START
      return 'START'
    when STAT_RUN_NSSTA
      return 'NSSTA'
    when STAT_RUN_WAITSTOP
      return 'WAIT-STOP'
    end
  end
  def starttime
    return Time.at(@startdate)
  end
  def stoptime
    return Time.at(@stopdate)
  end
  attr_reader :runname,:runnumber,:header,:ender, :eflist,:hdlist,:mtlist
end

class Babinfo < Babirl
  PORT = 17516
  INF_GET_DAQINFO  =   1
  INF_GET_RUNINFO  =   2
  INF_GET_RAWDATA  =  10
  INF_QUIT         =  99
  INF_GET_CLIHOSTS = 101
  INF_SET_CLIHOST  = 102
  INF_GET_COMLIST  = 201
  INF_GET_SCRLIST  = 301
  INF_SET_SCRNAME  = 302
  INF_GET_SCRDATA  = 303
  INF_DEL_SCR      = 311
  INF_GET_STATLIST = 401
  def initialize(host)
    @host = host
    @port = PORT
  end
  def get_rawdata
    rval = self.get(INF_GET_RAWDATA)
    return rval
  end
end
