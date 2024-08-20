(function(){

  //  var color_modified = '#ffcc00';
  var color_modified = "rgb(255, 204, 0)";
  var reset_when_reloaded = 1;
  var modflag = new Array(100);

  init();
  
  function init() {
    var fs = document.forms;
    
    for(var j=0;j<100;j++){
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
	  add_event(es[j], ['keyup', 'change', 'click'],
		    mkhandler(es, watch_property));
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
    set_color(elm, elm[prop] != elm.saved_default);
  }
  
  function set_color(elm, hilight) {
    elmid = elm.name.replace(/[a-zA-Z]/g,'');
    hdname = "mod"+elmid;
    idelm = document.forms[0].elements[hdname];
    if(hilight){
      if(elm.style.backgroundColor != color_modified){
	elm.style.backgroundColor = color_modified;
	modflag[elmid] ++;
      }
    }else{
      if(elm.style.backgroundColor == color_modified){
	elm.style.backgroundColor = elm.orig_color;
	modflag[elmid] --;
      }
    }
    idelm.value = modflag[elmid];

    if (!elm.assoc_labels) {
      return;
    }
    for (var i = 0; i < elm.assoc_labels.length; i++) {
      //console.log('assclabels ',i);
      set_color(elm.assoc_labels[i], hilight);
    }
  }
  
})();
