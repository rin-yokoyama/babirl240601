(function(){

  //  var color_modified = '#ffcc00';
  var color_modified = "rgb(255, 204, 0)";
  var reset_when_reloaded = 1;
  var modflag = new Array(500);

  init();
  
  function init() {
    var fs = document.forms;
    
    for(var j=0;j<500;j++){
      modflag[j] = 0;
    }

    //console.log("initialize");
    for (var i = 0; i < fs.length; i++) {
      if (reset_when_reloaded) {
	fs[i].reset();
      }
      var es = fs[i].elements;
      add_event(fs[i], ['reset'],
		mkhandler(fs[i].elements,
			  function(e) { set_color(e, false); }));
      for (var j = 0; j < es.length; j++) {
	var prop = get_property_watched(es[j]);
	es[j].saved_default = es[j][prop];
	es[j].orig_color = es[j].style.backgroundColor;
	
	if (es[j].type == 'radio') { /* XXX: inefficient */
	  es[j].orig_checked = es[j].checked;
	  add_event(es[j], ['keyup', 'change', 'click'],
		    mkhandler([es[j]], watch_property));
	} else {
	  add_event(es[j], ['keyup', 'change', 'click'],
		    mkhandler([es[j]], watch_property));
	}
      }
    }
    
    var ls = document.getElementsByTagName('label');
    for (var i = 0; i < ls.length; i++) {
      var labeled_elm;
      if (ls[i].htmlFor
	  && (labeled_elm = document.getElementById(ls[i].htmlFor))) {
	if (!labeled_elm.assoc_labels) {
	  labeled_elm.assoc_labels = [];
	}
	labeled_elm.assoc_labels.push(ls[i]);
	/* XXX: implicitly labeled controls not considered */
	ls[i].orig_color = ls[i].style.backgroundColor;
      }
    }
    function mkhandler(es, func) {
      return function() {
	for (var i = 0; i < es.length; i++) {
	  func(es[i]);
	}
      };
    }
  }
  
  function add_event(elm, evtypes, func) {
    for (var i = 0; i < evtypes.length; i++) {
      if (elm.attachEvent) {
	elm.attachEvent('on' + evtypes[i], func);
      } else if (elm.addEventListener) {
	elm.addEventListener(evtypes[i], func, false);
      } else {
	elm['on' + evtypes[i]] = func;
      }
    }
  }
  
  function get_property_watched(elm) {
    var property_watched = { checkbox: 'checked', radio: 'checked',
                             _default: 'value' };
    return property_watched[elm.type] || property_watched['_default'];
  }
  
  function watch_property(elm) {
    var prop = get_property_watched(elm);
    if(elm.name != 'macb0'){
	set_color(elm, elm[prop] != elm.saved_default);
    }else{
	set_mac0_color();
    }
  }
  
  function set_color(elm, hilight) {
      if(elm.type === 'submit'){
	  return;
      }
      //console.log(elm.name, elm.type);
      if(elm.name.match(/=/)){
	  var idx = elm.name.indexOf("=") + 1;
	  var len = elm.name.length;
	  elmid = elm.name.substring(idx, len);
	  //console.log('id=' + elmid);
      }else{
	  elmid = elm.name.replace(/[a-zA-Z]/g,'');
      }

    hdname = "mod"+elmid;
    idelm = document.forms[0].elements[hdname];

    if(hilight){
      if(elm.type == 'radio'){
	if(elm.org_checked != elm.checked){
  	  modflag[elmid] ++;
	  //console.log('radio modified', elm.name, modflag[elmid], elm.orig_checked, elm.checked);
	  if(modflag[elmid] > 2) modflag[elmid] = 2;
        }
      }else 	  
      if(elm.style.backgroundColor != color_modified){
	elm.style.backgroundColor = color_modified;
	modflag[elmid] ++;
      }
    }else{
      if(elm.type == 'radio'){
        if(elm.orig_checked == elm.checked){
          modflag[elmid] --;
	  //console.log('radio minus', elm.name, modflag[elmid], elm.orig_checked, elm.checked);
	  if(modflag[elmid] < 0) modflag[elmid] = 0;
        }
      }else
      if(elm.style.backgroundColor == color_modified){
	elm.style.backgroundColor = elm.orig_color;
	modflag[elmid] --;
      }
    }

    idelm.value = modflag[elmid];

    var tj = 0;
    for(tj=0;tj<500;tj++){
      if(modflag[tj]){
	document.getElementById('modtable').style.backgroundColor = '#ff6633';
        break;
      }
    }
    if(tj==500){
      document.getElementById('modtable').style.backgroundColor = '#eeeeee';
    }
    


    if (!elm.assoc_labels) {
      return;
    }
    for (var i = 0; i < elm.assoc_labels.length; i++) {
      //console.log('assclabels ',i);
      set_color(elm.assoc_labels[i], hilight);
    }
  }

  function set_mac0_color() {
    elm = document.forms[0].MAC0;
    elmid = 0;
    hdname = "mod"+elmid;
    idelm = document.forms[0].elements[hdname];

    if(elm.style.backgroundColor != color_modified){
	elm.style.backgroundColor = color_modified;
	modflag[elmid] ++;
      }
      
    idelm.value = modflag[elmid];

    if (!elm.assoc_labels) {
      return;
    }
    for (var i = 0; i < elm.assoc_labels.length; i++) {
      set_color(elm.assoc_labels[i], true);
    }
  }

})();


function set_mac0(str, pxe, grub){
    document.forms[0].MAC0.value = str;
    if(pxe == 1){
	document.forms[0].PXE0.checked = true;
    }else{
	document.forms[0].PXE0.checked = false;
    }
    if(grub == 1){
	document.forms[0].GRUB0.checked = true;
    }else{
	document.forms[0].GRUB0.checked = false;
    }
    document.forms[0].DHCP0.checked = true;
    document.forms[0].Use0.checked = true;
    
}


function ConnectedSelect(selIdList){
    var defval = new Array();
    for(var i=0;selIdList[i];i++) {
        var obj = document.getElementById(selIdList[i]);
        defval[i] = obj.options[obj.selectedIndex].value;
	obj.ConnectedSelectDefault = obj.options[obj.selectedIndex].value;

        var CS = new Object();
        if(i){
            CS.node=document.createElement('select');
            var GR = obj.getElementsByTagName('optgroup');
            while(GR[0]) {
                CS.node.appendChild(GR[0].cloneNode(true));
                obj.removeChild(GR[0]);
            }
        }
        if(selIdList[i+1]) {
            CS.nextSelect = document.getElementById(selIdList[i+1]);
            obj.onchange = function(){ConnectedSelectEnabledSelect(this)};
        } else {
            CS.nextSelect = false;
        }
        obj.ConnectedSelect = CS;
    }
  for(var i=0;selIdList[i];i++) {
    var obj = document.getElementById(selIdList[i]);
    if(selIdList[i+1]) {
      obj.onchange();
    }

    for(var j=0;j<obj.options.length; j++) {
      if(obj.options[j].value == defval[i]) {
        obj.selectedIndex = j;
        ConnectedSelectEnabledSelect(obj)
        break;
      }
    }
  }
}

function ConnectedSelectEnabledSelect(oSel){
    var oVal = oSel.options[oSel.selectedIndex].value;
    if(oVal) {
        if(oSel.ConnectedSelect.nextSelect.options != undefined) {
        
            while(oSel.ConnectedSelect.nextSelect.options[1]) {
              oSel.ConnectedSelect.nextSelect.remove(1);
            }
            var eF = false;
            for(var OG=oSel.ConnectedSelect.nextSelect.ConnectedSelect.node.firstChild;OG;OG=OG.nextSibling) {
                if(OG.label == oVal) {
                    eF = true;
                    for(var OP=OG.firstChild;OP;OP=OP.nextSibling)
                        oSel.ConnectedSelect.nextSelect.appendChild(OP.cloneNode(true));
                    break;
                }
            }
            oSel.ConnectedSelect.nextSelect.disabled = !eF;
        } else {
            oSel.ConnectedSelect.nextSelect.selectedIndex = 0;
            oSel.ConnectedSelect.nextSelect.disabled = true;
        }
    }
    if(oSel.ConnectedSelect.nextSelect.onchange)oSel.ConnectedSelect.nextSelect.onchange();
}


function ConnectedSelectDefault(selIdList){
    for(var i=0;selIdList[i];i++) {
	if(i){
	    var obj = document.getElementById(selIdList[i]);
	    obj.disabled = true;
	    var oSel = document.getElementById(selIdList[i-1])
		var oVal = oSel.ConnectedSelectDefault;
	    for(var OG=obj.ConnectedSelect.node.firstChild;OG;OG=OG.nextSibling) {
		if(OG.label != oVal) { continue; }
		for(var OP=OG.firstChild;OP;OP=OP.nextSibling)
		    obj.appendChild(OP.cloneNode(true));
		obj.disabled = false;
		break;
	    }
	}
    }
}
