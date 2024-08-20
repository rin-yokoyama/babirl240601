#!/usr/bin/ruby -Ke

require 'ridf.rb'
require 'ridf_cgi.rb'
require 'cgi.rb'

cgi = CGI.new('html4Fr')
ridf_cgi = RIDFcgi.new
dataname = $ARGV[0]
datafile = RIDFdata.new(dataname)
if ($ARGV[0] =~ /\.ridf/) then
  # Offline
  datafile.type = RIDFdata::OFFLINE
else
  # Onfline
  datafile.type = RIDFdata::ONLINE
end
datafile.open
table = ""
blkn = 0
rblkn = 0
endlen = 0
while (header=datafile.readhead) do
  rstring = sprintf("ly:%d,class:%d,size:%d,efn:%d ",header.layer,header.class,header.size,header.efn)
  case header.class
    when RIDFheader::RIDF_EA_BLOCK
      blkn += 1
      rstring += "EA BLOCK"
    when RIDFheader::RIDF_EAEF_BLOCK
      blkn += 1
      rstring += "EAEF BLOCK"
    when RIDFheader::RIDF_BLOCK_NUMBER
      rblkn = datafile.read(1,"L")
      rstring += "BLOCK Number=" + rblkn.to_s
    when RIDFheader::RIDF_END_BLOCK
      endlen = datafile.read(1,"L")
      rstring += "Size of This Block=" + endlen.to_s
    when RIDFheader::RIDF_EVENT
      rstring += "Event BLOCK" + cgi.br
      evtn = datafile.read(1,"L")
      rstring += "EVENT Number=" + evtn.to_s
    when RIDFheader::RIDF_EVENTTS
      rstring += "Event BLOCK with Time Stamp" + cgi.br
      evtn = datafile.read(1,"L")
      rstring += "EVENT Number=" + evtn.to_s
      evtn = datafile.read(1,"L")
      evtn = datafile.read(1,"L")
    when RIDFheader::RIDF_TIMESTAMP
      rstring += "TS Segment BLOCK" + cgi.br
      segdata = datafile.read_tsdata(header.size)
    when RIDFheader::RIDF_SEGMENT
      rstring += "Segment BLOCK" + cgi.br
      segid,segdata = datafile.read_segdata(header.size)
      seg = RIDFSegid.new(segid)
      rstring += "Segment ID=" + segid.to_s + cgi.br
      rstring += "Device=" + cgi.font("color"=>ridf_cgi.segcolor('device')){seg.device.join(" ").to_s}
      rstring += " Focal=" + cgi.font("color"=>ridf_cgi.segcolor('focal')){seg.focal.join(" ").to_s}
      rstring += " Detector=" + cgi.font("color"=>ridf_cgi.segcolor('detector')){seg.detector.join(" ").to_s}
      rstring += " Module=" + cgi.font("color"=>ridf_cgi.segcolor('module')){seg.module.join(" ").to_s}
      datafile.rawdata += segdata
    when RIDFheader::RIDF_COMMENT
      rstring += "Comment BLOCK" + cgi.br
      rstring += datafile.read_cdata(header.size).delete("\000")
    when RIDFheader::RIDF_SCALER
      rstring += "SCALER BLOCK" + cgi.br
      date, id = datafile.read_sdata(header.size)
      rstring += "Date=" + date.to_s + " ID=" + id.to_s
    when RIDFheader::RIDF_CSCALER
      rstring += "CSCALER BLOCK" + cgi.br
      date, id = datafile.read_sdata(header.size)
      rstring += "Date=" + date.to_s + " ID=" + id.to_s
    when RIDFheader::RIDF_NCSCALER32
      rstring += "SCALER32 BLOCK" + cgi.br
      date, id = datafile.read_sdata(header.size)
      rstring += "Date=" + date.to_s + " ID=" + id.to_s
    when RIDFheader::RIDF_STATUS
      rstring += "STATUS BLOCK" + cgi.br
      rstring += datafile.read_cdata(header.size)
  end
  lstring = ""
  (0..datafile.rawdata.size-1).each{|key|
    if key<=5 then 
      lstring += cgi.font("color"=>ridf_cgi.color(header.class)){sprintf("%04x ",datafile.rawdata[key])}
#    if ((key<=5)&& ((header.class == RIDFheader::RIDF_EVENT)||
#                         (header.class == RIDFheader::RIDF_SEGMENT))) then
#      lstring += cgi.font("color"=>ridf_cgi.color(header.class)){sprintf("%04x ",datafile.rawdata[key])}
    else
      lstring += sprintf("%04x ",datafile.rawdata[key])
    end
    lstring += cgi.br if (((key+1)%8) == 0)
  }
  table += cgi.tr{cgi.td{lstring} + cgi.td{rstring}}
  datafile.rawdata = Array.new
  break if (blkn > 3)
end
datafile.close
cgi.out("charset"=>"euc-jp") do
  cgi.html() do
    cgi.head{cgi.title{"Rawdata Monitor"}} +
      cgi.body() do
      cgi.table('border'=>'1'){table}
    end
  end
end

