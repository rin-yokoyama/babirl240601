#!/bin/perl

$file = 'segidlist.h';

open(IN, "< $file");
@data = <IN>;
close(IN);

$devmode = 0;
$fpmode = 0;
$detmode = 0;
$modmode = 0;
@dev;
@devid;
@dev_com;
@fp;
@fpid;
@fp_com;
@det;
@detid;
@det_com;
@mod;
@modid;
@mod_com;
foreach $line (@data){
    if($line =~ /^\/\/\*-Device/){
	$devmode = 1;
    }
    if($line =~ /^\/\/-\*Device/){
	$devmode = 0;
    }
    if($line =~ /^\/\/\*-Focal/){
	$fpmode = 1;
    }
    if($line =~ /^\/\/-\*Focal/){
	$fpmode = 0;
    }
    if($line =~ /^\/\/\*-Detector/){
	$detmode = 1;
    }
    if($line =~ /^\/\/-\*Detector/){
	$detmode = 0;
    }
    if($line =~ /^\/\/\*-Module/){
	$modmode = 1;
    }
    if($line =~ /^\/\/-\*Module/){
	$modmode = 0;
    }

    if($line =~ /^#define/){
	$line =~ s/\n//;
	$line =~ s/\r//;
	$line =~ s/#define//;
	$line =~ s/ +/ /g;
	$line =~ s/^ +//;
	($def, $c) = split(/\/\//, $line);
	($d, $id) = split(/ /, $def);
	$d =~ s/ +//g;
	$id =~ s/ +//g;
	if($devmode){
	    push(@dev, $d);
	    push(@devid, $id);
	    push(@dev_com, $c);
	}
	if($fpmode){
	    push(@fp, $d);
	    push(@fpid, $id);
	    push(@fp_com, $c);
	}
	if($detmode){
	    push(@det, $d);
	    push(@detid, $id);
	    push(@det_com, $c);
	}
	if($modmode){
	    push(@mod, $d);
	    push(@modid, $id);
	    push(@mod_com, $c);
	}
    }
}

$i = 0;
print "<?php\n";
print "  class CSegIDList {\n";
print "    public \$Device = array(\n";
for($i=0;$i<$#dev;$i++){
    print "      ".$devid[$i]." => '".$dev[$i]."',\n";
}
print "      ".$devid[$i]." => '".$dev[$i]."');\n";
print "    public \$Focal = array(\n";
for($i=0;$i<$#fp;$i++){
    print "      ".$fpid[$i]." => '".$fp[$i]."',\n";
}
print "      ".$fpid[$i]." => '".$fp[$i]."');\n";
print "    public \$Detector = array(\n";
for($i=0;$i<$#det;$i++){
    print "      ".$detid[$i]." => '".$det[$i]."',\n";
}
print "      ".$detid[$i]." => '".$det[$i]."');\n";
print "    public \$Module = array(\n";
for($i=0;$i<$#mod;$i++){
    print "      ".$modid[$i]." => '".$mod[$i]."',\n";
}
print "      ".$modid[$i]." => '".$mod[$i]."');\n";


print "  }\n";
print "?>\n";

