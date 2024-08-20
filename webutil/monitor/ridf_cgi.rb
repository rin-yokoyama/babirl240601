require 'ridf.rb'
class RIDFcgi
  COLOR_EF_BLOCK   = 'red'
  COLOR_EA_BLOCK   = 'red'
  COLOR_EAEF_BLOCK = 'red'
  COLOR_EVENT      = 'blue'
  COLOR_SEGMENT    = 'blue'
  COLOR_COMMENT    = 'green'
  COLOR_SCALER     = 'green'
  COLOR_CSCALER    = 'green'
  COLOR_STATUS     = 'green'

  COLOR_DEVICE     = 'red'
  COLOR_FOCAL      = 'blue'
  COLOR_DETECTOR   = 'green'
  COLOR_MODULE     = 'magenta'

  def initialize
    @color = Array.new
    @color[RIDFheader::RIDF_EF_BLOCK] = COLOR_EF_BLOCK
    @color[RIDFheader::RIDF_EA_BLOCK] = COLOR_EA_BLOCK
    @color[RIDFheader::RIDF_EAEF_BLOCK] = COLOR_EAEF_BLOCK
    @color[RIDFheader::RIDF_EVENT] = COLOR_EVENT
    @color[RIDFheader::RIDF_SEGMENT] = COLOR_SEGMENT
    @color[RIDFheader::RIDF_COMMENT] = COLOR_COMMENT
    @color[RIDFheader::RIDF_SCALER] = COLOR_SCALER
    @color[RIDFheader::RIDF_CSCALER] = COLOR_CSCALER
    @color[RIDFheader::RIDF_STATUS] = COLOR_STATUS
    @segcolor = Hash.new
    @segcolor['device'] = COLOR_DEVICE
    @segcolor['focal'] = COLOR_FOCAL
    @segcolor['detector'] = COLOR_DETECTOR
    @segcolor['module'] = COLOR_MODULE
  end

  def color (headerclass)
    return @color[headerclass]
  end
  def segcolor(string)
    return @segcolor[string]
  end
end
