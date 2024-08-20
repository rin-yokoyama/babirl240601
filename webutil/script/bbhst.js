"use strict"

var hxmaxlist = [256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536];
var nbinlist  = [32, 64, 128, 256, 512, 1024, 2048, 4096, 8192];

//         Name, offColor, onColor, Xoff, Yoff, Wid, Height
//           0      1           2      3   4    5    6
var LogBtnAr = ['Log', '#999999', '#3333cc', 5, -25, 25, 16];
var UnZBtnAr = ['Unz', '#999999', '#33cc33', 5, -45, 25, 16];
var BnNBtnAr = ['<',   '#999999', '#cc3333', 7,  55, 12, 12];
var BnWBtnAr = ['>',   '#999999', '#cc3333', 22, 55, 12, 12];


function cButton(cv, btnar, tlim){
  this.cv = cv;
  this.btnar = btnar;
  this.tlim = tlim;
}

cButton.prototype.Draw = function(of){
  var ct = this.cv.getContext("2d");
  var hoff = this.cv.height;;

  if(this.btnar[4] > 0){
    hoff = this.tlim;
  }
  
  if(of == 0){
    ct.fillStyle = this.btnar[1];
  }else{
    ct.fillStyle = this.btnar[2];
  }
  ct.fillRect(this.btnar[3], hoff+this.btnar[4],
	      this.btnar[5], this.btnar[6]);
  ct.fillStyle = '#000000';
  ct.strokeRect(this.btnar[3], hoff+this.btnar[4],
		this.btnar[5], this.btnar[6]);
  
  ct.font = '12px Century Gothic';
  ct.textAlign = 'center';
  ct.fillStyle = '#000000';
  ct.fillText(this.btnar[0], this.btnar[3]+(this.btnar[5])/2,
	      hoff+this.btnar[4]+this.btnar[6]/2+4, this.btnar[5]);
}

cButton.prototype.inBtn = function(x, y){
  var hoff = this.cv.height;;

  if(this.btnar[4] > 0){
    hoff = this.tlim;
  }

  if(x > this.btnar[3] && x < (this.btnar[3]+this.btnar[5]) &&
     y > (hoff+this.btnar[4]) &&
     y < (hoff+this.btnar[4]+this.btnar[6])){
    return 1;
  }else{
    return 0;
  }
}


function HProp(cv, title, xar, yar,
	       tlim, blim, llim, rlim,
	       hxmin, hxmax, hymin, hymax,
	       cbin, ibin,gwid,ghi,xmag,ymag,
	       log, logbtn, unzbtn, bnnbtn, bnwbtn){

  this.cv    = cv;
  this.title = title;
  this.xar   = xar;
  this.yar   = yar;
  this.tlim  = tlim;
  this.blim  = blim;
  this.llim  = llim;
  this.rlim  = rlim;
  this.hxmin  = hxmin;
  this.hxmax  = hxmax;
  this.hymin  = hymin;
  this.hymax  = hymax;
  this.cbin  = cbin;
  this.nbin  = nbinlist[ibin];
  this.gwid  = gwid;
  this.ghi   = ghi;
  this.xmag  = xmag; 
  this.ymag  = ymag;
  this.log   = log;
  this.start = -1;
  this.stop  = -1;
  this.bbin  = 0;
  this.ebin  = nbinlist[ibin];
  this.logbtn = logbtn;
  this.unzbtn = unzbtn;
  this.bnnbtn = bnnbtn;
  this.bnwbtn = bnwbtn;
  this.wbnn   = 0;
  this.wbnw   = 0;
  this.ibin   = ibin;

}

HProp.prototype.incNbin = function(){
  if(this.ibin < (nbinlist.length-1)){
    this.ibin = this.ibin + 1;
    this.nbin = nbinlist[this.ibin];
  }
}

HProp.prototype.decNbin = function(){
  if(this.ibin > 0){
    this.ibin = this.ibin - 1;
    this.nbin = nbinlist[this.ibin];
  }
}


function inHist(hp, x, y){
  if(x > hp.llim &&
     x < hp.cv.width - hp.rlim &&
     y > hp.tlim &&
     y < hp.cv.height - hp.blim){
    return 1;
  }else{
    return 0;
  }
}

function cXtoNBin(hp, x){
  var hx, ix;

  hx = (x-hp.llim)/hp.xmag + hp.bbin;
  ix = Math.floor(hx);

  return ix;
}

function cXtoHX(hp, x){
  var hx, rx, dbin;

  hx = (x-hp.llim)/hp.xmag + hp.bbin;
  dbin = (hp.hxmax-hp.hxmin)/hp.nbin;
  rx = Math.round(hx *dbin);

  return rx;
}

function showXY(hp, x, y){
  var ct = hp.cv.getContext("2d");
  var ix, rx, dbin;

  ix = cXtoNBin(hp, x);
  rx = cXtoHX(hp, x);
  
  var str = 'X='+rx+' BinN='+ix+' BinY='+hp.cbin[ix];
  ct.fillStyle = '#000000';
  ct.font = '12px Century Gothic';
  ct.textAlign = 'right';
  ct.fillText(str, hp.cv.width - hp.rlim - 10,
	      hp.tlim+15, (hp.cv.width-hp.rlim)/2);
}

function exStart(hp){
  return function(e){
    var cv = e.target;
    var x = e.clientX - cv.offsetLeft;
    var y = e.clientY - cv.offsetTop;
    if(inHist(hp, x,y)){
      hp.start = x - hp.llim;
      hp.stop  = -1;
    }

    if(hp.bnnbtn.inBtn(x, y)){
      hp.bnnbtn.Draw(1);
      hp.wbnn = 1;
    }

    if(hp.bnwbtn.inBtn(x, y)){
      hp.bnwbtn.Draw(1);
      hp.wbnw = 1;
    }

  }
}

function exStop(hp){
  return function(e){
    var cv = e.target;
    var x = e.clientX - cv.offsetLeft;
    var y = e.clientY - cv.offsetTop;
    if(inHist(hp, x,y) && hp.start != -1){
      var tb, te;
      hp.stop = x - hp.llim;

      if(hp.start < hp.stop){
	tb = cXtoNBin(hp, hp.start+hp.llim);
	te = cXtoNBin(hp, hp.stop+hp.llim);
      }else{
	tb = cXtoNBin(hp, hp.stop+hp.llim);
	te = cXtoNBin(hp, hp.start+hp.llim);
      }
      if((te - tb) > 2){
	hp.bbin = tb;
	hp.ebin = te;
	clearCanvas(hp);
	calcXYmag(hp);
	drawHist(hp);
	drawFrame(hp);
	drawAxis(hp);
      }
    }

    if(hp.wbnn && hp.bnnbtn.inBtn(x, y)){
      hp.bnnbtn.Draw(0);
      hp.wbnn = 0;
      hp.decNbin();
      clearCanvas(hp);
      calcBinning(hp);      
      calcXYmag(hp);
      drawHist(hp);
      drawFrame(hp);
      drawAxis(hp);
    }else if(hp.wbnn){
      hp.bnnbtn.Draw(0);
      hp.wbnn = 0;
    }

    if(hp.wbnw && hp.bnwbtn.inBtn(x, y)){
      hp.bnwbtn.Draw(0);
      hp.wbnw = 0;
      hp.incNbin();
      clearCanvas(hp);
      calcBinning(hp);      
      calcXYmag(hp);
      drawHist(hp);
      drawFrame(hp);
      drawAxis(hp);
    }else if(hp.wbnw){
      hp.bnwbtn.Draw(0);
      hp.wbnw = 0;
    }
  }
}
  
function vBar(hp){
  return function(e){
    var cv = e.target;
    var x = e.clientX - cv.offsetLeft;
    var y = e.clientY - cv.offsetTop;
    if(inHist(hp, x, y)){
      var ct = cv.getContext("2d");
      if(hp.start == -1){
	drawHist(hp);
	ct.strokeStyle = 'rgba(30, 220, 80, 0.4)';
	ct.beginPath();
	ct.lineTo(x, hp.tlim);
	ct.lineTo(x, cv.height-hp.blim);
	ct.closePath();
	ct.stroke();
	drawFrame(hp);
	showXY(hp, x, y);
      }else{
	ct.clearRect(hp.start + hp.llim, hp.tlim,
		 x - hp.start - hp.llim,
		 cv.height-hp.tlim-hp.blim);
	drawHist(hp);
	ct.fillStyle = 'rgba(30, 30, 200, 0.3)';
	ct.fillRect(hp.start + hp.llim, hp.tlim,
		 x - hp.start - hp.llim,
		 cv.height-hp.tlim-hp.blim);
	drawFrame(hp);
	showXY(hp, x, y);
      }	
    }else{
      drawHist(hp);
      drawFrame(hp);
    }
  }
}

function showHist(hp){
  drawHist(hp);
  drawFrame(hp);
  hp.wbnn = 0;
  hp.wbnw = 0;
}


function onClick(hp){
  return function(e){
    var cv = e.target;
    var x = e.clientX - cv.offsetLeft;
    var y = e.clientY - cv.offsetTop;

    if(hp.logbtn.inBtn(x, y)){
      if(hp.log == 0){
	hp.log = 1;
      }else{
	hp.log = 0;
      }
      clearCanvas(hp);
      calcXYmag(hp);
      drawHist(hp);
      drawFrame(hp);
      drawAxis(hp);
    }

    if(hp.unzbtn.inBtn(x, y)){
      if(hp.bbin != 0 || hp.ebin != hp.nbin){
	hp.bbin = 0;
	hp.ebin = hp.nbin;
	clearCanvas(hp);
	calcXYmag(hp);
	drawHist(hp);
	drawFrame(hp);
	drawAxis(hp);
      }
    }

    hp.start = -1;
  }
}

function drawRect(cv, x,y,width,height){
  var ct = cv.getContext("2d");
  ct.fillStyle = 'rgba(255, 30, 200, 0.6)';

  ct.fillRect(x-width/2,y-height/2,width,height);
}

function drawFrame(hp){
  var ct = hp.cv.getContext("2d");

  ct.strokeStyle = '#000000';
  ct.beginPath();
  ct.moveTo(hp.llim, hp.tlim);
  ct.lineTo(hp.cv.width-hp.rlim, hp.tlim);
  ct.lineTo(hp.cv.width-hp.rlim, hp.cv.height-hp.blim);
  ct.lineTo(hp.llim, hp.cv.height-hp.blim);
  ct.closePath();
  ct.stroke();

}

function drawAxis(hp){
  var ct = hp.cv.getContext("2d");

  var dbin = (hp.hxmax-hp.hxmin)/hp.nbin;
  var smin = dbin * hp.bbin;
  var smax = dbin * hp.ebin;
  var tymax = Math.max.apply(null, hp.cbin.slice(hp.bbin, hp.ebin));

  ct.fillStyle = '#000066';
  ct.font = '14px Century Gothic';
  ct.textAlign = 'left';
  ct.fillText(Math.floor(smin), hp.llim, hp.cv.height-hp.blim+16, 20);
  ct.textAlign = 'right';
  ct.fillText(Math.floor(smax), hp.cv.width-hp.rlim, hp.cv.height-hp.blim+16);
  ct.textAlign = 'center';
  ct.fillText(Math.floor((smax-smin)/2), (hp.cv.width-hp.llim)/2+hp.llim,
	      hp.cv.height-hp.blim+16);

  ct.textAlign = 'right';
  ct.fillText(hp.hymin, hp.llim-5, hp.cv.height-hp.blim, hp.llim);
  ct.fillText(Math.floor(tymax/0.8), hp.llim-5, hp.tlim+10, hp.llim);
  
  ct.font = '16px Century Gothic';
  ct.textAlign = 'left';
  ct.fillStyle = '#000000';
  
  ct.fillText(hp.title, hp.llim, 15, hp.cv.width-hp.llim);

  // Draw Log Button
  hp.logbtn.Draw(hp.log);
  
  // Draw Unzoom Button
  if(hp.bbin == 0 && hp.ebin == hp.nbin){
    hp.unzbtn.Draw(0);
  }else{
    hp.unzbtn.Draw(1);
  }

  //Draw Number of Bins
  ct.font = '14px Century Gothic';
  ct.fillStyle = '#000000';
  ct.textAlign = 'center';
  ct.fillText('Nbin',  20, hp.tlim+34, 30);
  ct.fillText(hp.nbin, 20, hp.tlim+48, 30);
  ct.strokeRect(3,hp.tlim+20,35,50);
  hp.bnnbtn.Draw(0);
  hp.bnwbtn.Draw(0);

}

function clearHist(hp){
  var ct = hp.cv.getContext("2d");
  ct.clearRect(hp.llim, hp.tlim,
	       hp.cv.width-hp.llim-hp.rlim,
	       hp.cv.height-hp.tlim-hp.blim);

}

function clearCanvas(hp){
  var ct = hp.cv.getContext("2d");
  ct.clearRect(0, 0, hp.cv.width, hp.cv.height);
}


function drawHist(hp){
  // Draw path
  var xzero = hp.llim;
  var yzero = hp.cv.height-hp.blim;
  var ct = hp.cv.getContext("2d");
  var ty, tty;
  
  ct.beginPath();
  ct.moveTo(xzero, yzero);
  ty = hp.cbin[hp.bbin];
  if(hp.log == 1){
    if(ty > 0){
      ty = Math.log(ty)/Math.LN10;
    }
  }
  ct.lineTo(xzero, yzero - ty*hp.ymag);
  for(var i=hp.bbin+1;i<hp.ebin;i++){
    tty = hp.cbin[i-1];
    ty  = hp.cbin[i];
    if(hp.log == 1){
      if(tty > 0){
	tty = Math.log(tty)/Math.LN10;
      }
      if(ty > 0){
	ty = Math.log(ty)/Math.LN10;
      }
    }
    ct.lineTo((i-hp.bbin)*hp.xmag+xzero, yzero - tty*hp.ymag);
    ct.lineTo((i-hp.bbin)*hp.xmag+xzero, yzero - ty*hp.ymag);
  }
  ct.closePath();
  ct.fillStyle = '#ffaa33';
  clearHist(hp);
  ct.fill();
  ct.stroke();
}


function calcXYmag(hp){
  var gwid = hp.cv.width - hp.llim - hp.rlim;
  var ghi  = hp.cv.height - hp.tlim - hp.blim;
  var tymax = Math.max.apply(null, hp.cbin.slice(hp.bbin, hp.ebin));
  
  hp.xmag = gwid / (hp.ebin - hp.bbin);

  if(hp.log == 0){
    hp.ymag = ghi / tymax*0.8;
  }else{
    if(hp.hymax != 0){
      hp.ymag = ghi / (Math.log(tymax) / Math.LN10) * 0.8;
    }else{
      hp.ymag = 1;
    }
  }
}

function calcBinning(hp){
  hp.cbin.length = 0;

  // fill histogram
  var dbin = (hp.hxmax-hp.hxmin)/hp.nbin;

  for(var i=0;i<hp.nbin;i++){
    hp.cbin[i] = 0;
  }

  var hx;
  for(var i=0;i<hp.xar.length;i++){
    hx = Math.floor((hp.xar[i]-hp.hxmin)/dbin);
    hp.cbin[hx] += hp.yar[i];
  }

  hp.hymax = Math.max.apply(null, hp.cbin);

  hp.bbin = 0;
  hp.ebin = hp.nbin;
}


function init(cvname, title, xar, yar){
  var xmin = 0, xmax = 1;
  var ymin = 0, ymax = 100;
  var tlim = 22;
  var blim = 20;
  var llim = 50;
  var rlim = 10;
  var hymax = 100;
  var hxmin = 0;
  var hxmax=4096;
  var cbin = [];
  var ibin = 1;
  var nbin = 64;
  var gwid = 100;
  var ghi  = 100;
  var xmag = 1.;
  var ymag = 1.;

  nbin = nbinlist[ibin];

  var cv = document.getElementById(cvname);

  var logbtn = new cButton(cv, LogBtnAr, tlim);
  var unzbtn = new cButton(cv, UnZBtnAr, tlim);
  var bnnbtn = new cButton(cv, BnNBtnAr, tlim);
  var bnwbtn = new cButton(cv, BnWBtnAr, tlim);
  var hp = new HProp(cv, title, xar, yar,
		     tlim, blim, llim, rlim,
		     hxmin, hxmax, 0, hymax,
		     cbin,ibin,gwid,ghi,xmag,ymag, 0.,
		     logbtn, unzbtn, bnnbtn, bnwbtn);

  
  var xmax = Math.max.apply(null, hp.xar);
  var xmin = Math.min.apply(null, hp.xar);
  hp.hxmin =0;
  hp.hxmax=4096;
  var df = xmax - xmin;

  for(var i=0;i<hxmaxlist.length;i++){
    if(df<hxmaxlist[i]){
      hp.hxmax = hxmaxlist[i];
      df = hp.hxmax;
      break;
    }
  }
  if(df != hp.hxmax){
    hp.hxmax = xmax*1.1;
  }


  calcBinning(hp);
  hp.bbin = 0;
  hp.ebin = hp.nbin;
  calcXYmag(hp);
  drawHist(hp);
  drawFrame(hp);
  drawAxis(hp);

  cv.addEventListener('click', onClick(hp), false);
  cv.addEventListener('mousemove', vBar(hp), false);
  cv.addEventListener('mouseout',  showHist(hp), false);
  cv.addEventListener('mousedown', exStart(hp, false));
  cv.addEventListener('mouseup', exStop(hp, false));
		      
}
