<?php

# ver 0.2

# Copyright (C) 2005, University of Alaska Fairbanks
# Biotechnology Computing Research Group
# https://biotech.inbre.alaska.edu
# jlong@alaska.edu
# All rights reserved.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, 
# Boston, MA  02110-1301, USA.

echo "<html>\n";
echo "<head>\n";
echo "<title>WFLOW</title>\n";
echo "<meta http-equiv=\"Expires\" content=\"0\">\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "</head>\n";

include("include.php");
$func = $_GET['func'];
$wdir = $_GET['wdir'];
$wdir = "$SCRATCH/$wdir";
$wsdl = $_GET['wsdl'];

# this script will display a clickable node in the main window
# in the following manner:
# 1) parse the tree_items file to create a node string for use by "dot"
# 2) if there is a dot file, insert the new node string; else create dot file
# 3) generate html for the new dot file

# 1) parse the tree_items file to create a node string for use by "dot"

# append a number onto each node name so we can allow for duplicates
if(file_exists("$wdir/node_num"))
{
  exec("cat $wdir/node_num", $output);
  foreach($output as $node_num);
}
else
  $node_num = 0;

# parse the wsdl to generate the "label" attribute that defines node appearance
# use the php expat routines to parse the wsdl for the operation names
readWsdl("$wdir/wsdl_$wsdl"); # from include.php

# get the start and end of the portType block
foreach($wsdl_index as $in_key=>$in_val)
  if(!strcasecmp($in_key, "porttype"))
  {
    foreach($in_val as $key=>$val)
    {
      if($key == "0") $start = $val;
      if($key == "1") $end   = $val;
    }
    break;
  }
    
# now go to the input tag within the operation block (within 
# the portType block) for this func and get the message
for($i=$start+1; $i<$end; $i++)
  foreach($wsdl_values[$i] as $key=>$val)
    if((!strcasecmp($key, "tag")) && (!strcasecmp($val, "operation")))
      foreach($wsdl_values[$i] as $key=>$val)
        if(!strcasecmp($key, "attributes"))
	  foreach($val as $a_key=>$a_val)
            if(!strcasecmp($a_key, "name") && !strcasecmp($a_val, $func))
	      # $i is in the right operation, so next input is the one we want
	      for($j=$i+1; $j<$end; $j++)
	        foreach($wsdl_values[$j] as $key=>$val)
                  if((!strcasecmp($key, "tag")) && (!strcasecmp($val, "input")))
		    foreach($wsdl_values[$j] as $key=>$val)
                      if(!strcasecmp($key, "attributes"))
                        foreach($val as $a_key=>$a_val)
                          if(!strcasecmp($a_key, "message"))
			  {
			    # now we have the name of the message block
			    # containing the input parameter names
			    # take off namespace id
			    $tok = strtok($a_val, ":");
			    while ($tok)
                            {
                              $last_tok = $tok;
                              $tok = strtok(":");
                            }
			    break 8;
			  }
# now go to the message block and extract the parts (input parameters)
$k = 0;
foreach($wsdl_index as $in_key=>$in_val)
  if(!strcasecmp($in_key, "message"))
  {
    foreach($in_val as $key=>$val)
    {
      foreach($wsdl_values[$val] as $akey=>$aval) # at a message
        if(!strcasecmp($akey, "attributes"))
	  foreach($aval as $bkey=>$bval)
            if(!strcasecmp($bkey, "name") && !strcasecmp($bval, $last_tok))
	      # $val is the right message, so get all "parts" 
	      # up to the closing message tag
	      for($j=$val+1; $j<$val+$MAXPARAMS; $j++) 
	        foreach($wsdl_values[$j] as $ckey=>$cval)
		{
                  if((!strcasecmp($ckey, "tag")) && (!strcasecmp($cval, "part")))
		    foreach($wsdl_values[$j] as $dkey=>$dval)
                      if(!strcasecmp($dkey, "attributes"))
		      {
		        $flag = 0;
                        foreach($dval as $ekey=>$eval)
			{
                          if(!strcasecmp($ekey, "name"))
			  {
			    $input[$k] = $eval;
			    if($flag) $k++;
			    else $flag = 1;
                          }
                          if(!strcasecmp($ekey, "type"))
			  {
			    $intype[$k] = $eval;
			    if($flag) $k++;
			    else $flag = 1;
                          }
		        }
		      }
		  if((!strcasecmp($ckey, "tag")) && (!strcasecmp($cval, "message")))
		    break 6;
                }
    }
    break;
  }
		
# repeat the above for the output parameter
# get the start and end of the portType block
foreach($wsdl_index as $in_key=>$in_val)
  if(!strcasecmp($in_key, "porttype"))
  {
    foreach($in_val as $key=>$val)
    {
      if($key == "0") $start = $val;
      if($key == "1") $end   = $val;
    }
    break;
  }
    
# now go to the output tag within the operation block (within 
# the portType block) for this func and get the message
for($i=$start+1; $i<$end; $i++)
  foreach($wsdl_values[$i] as $key=>$val)
    if((!strcasecmp($key, "tag")) && (!strcasecmp($val, "operation")))
      foreach($wsdl_values[$i] as $key=>$val)
        if(!strcasecmp($key, "attributes"))
	  foreach($val as $a_key=>$a_val)
            if(!strcasecmp($a_key, "name") && !strcasecmp($a_val, $func))
	      # $i is in the right operation, so next output is the one we want
	      for($j=$i+1; $j<$end; $j++)
	        foreach($wsdl_values[$j] as $key=>$val)
                  if((!strcasecmp($key, "tag")) && (!strcasecmp($val, "output")))
		    foreach($wsdl_values[$j] as $key=>$val)
                      if(!strcasecmp($key, "attributes"))
                        foreach($val as $a_key=>$a_val)
                          if(!strcasecmp($a_key, "message"))
			  {
			    # now we have the name of the message block
			    # containing the output parameter name
			    # take off namespace id
			    $tok = strtok($a_val, ":");
			    while ($tok)
                            {
                              $last_tok = $tok;
                              $tok = strtok(":");
                            }
			    break 8;
			  }
# now go to the message block and extract the output part (output parameter)
foreach($wsdl_index as $in_key=>$in_val)
  if(!strcasecmp($in_key, "message"))
  {
    foreach($in_val as $key=>$val)
    {
      foreach($wsdl_values[$val] as $akey=>$aval) # at a message
        if(!strcasecmp($akey, "attributes"))
	  foreach($aval as $bkey=>$bval)
            if(!strcasecmp($bkey, "name") && !strcasecmp($bval, $last_tok))
	      # $val is the right message, so get all "parts" 
	      # up to the closing message tag
	      for($j=$val+1; $j<$val+$MAXPARAMS; $j++) 
	        foreach($wsdl_values[$j] as $ckey=>$cval)
		{
                  if((!strcasecmp($ckey, "tag")) && (!strcasecmp($cval, "part")))
		    foreach($wsdl_values[$j] as $dkey=>$dval)
                      if(!strcasecmp($dkey, "attributes"))
		      {
		        $flag = 0;
                        foreach($dval as $ekey=>$eval)
			{
                          if(!strcasecmp($ekey, "name"))
			  {
			    $output = $eval;
			    if($flag) break 8;
			    else $flag = 1;
                          }
                          if(!strcasecmp($ekey, "type"))
			  {
			    $outtype = $eval;
			    if($flag) break 8;
			    else $flag = 1;
                          }
			}
		      }
		  if((!strcasecmp($ckey, "tag")) && (!strcasecmp($cval, "message")))
		  {
		    $output = "NO_OUTPUT";
		    $outtype = "NULL";
		    break 6;
                  }
		}
    }
    break;
  }
# construct the label;
$func = strtoupper($func);
#$label = "\"\{${func}_$node_num|";
$label = "\"${func}_$node_num|";

for($i=0; $i<$k; $i++)
#  $label = "$label<in_$input[$i]($intype[$i])>$input[$i]|";
  $label = "$label<in_$input[$i]>$input[$i]|";

#$label = "$label<out($outtype)>$output}\"";
$label = "$label<out>$output\"";

# record output type and name for this node, used in declare_vars.c
if(!file_exists("${wdir}/OUTTYPES"))
{
  mkdir("$wdir/OUTTYPES", 0755);
}
exec("echo $outtype > $wdir/OUTTYPES/${func}_$node_num");
exec("echo $output >> $wdir/OUTTYPES/${func}_$node_num");

# 2) if there is a dot file, insert the new node string; else create dot file

exec("$WFLOW_DIR/node.sh $wdir $wsdl $label $func $node_num $WFLOW_SERVER $dot");

# solve potential local caching problems by changing name of map.jpg each time

$last_node_num = $node_num - 1;

if(file_exists("$wdir/map_${last_node_num}.jpg"))
{
  unlink("$wdir/map_${last_node_num}.jpg");
}

$node_num = $node_num + 1;
exec("echo $node_num > $wdir/node_num");
$last_node_num = $node_num - 1;

echo "<body>\n";
echo "<map name=\"graphmap\">\n";

$fp = fopen("$wdir/map.htm", "r") or die("arrrggg");
while(!feof($fp))
{
  $line = fgets($fp, 1024);
  echo $line;
}
fclose($fp);

exec("echo $wdir | cut -d\"/\" -f3", $output);
foreach($output as $wdir);

echo "</map>\n";
echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$wdir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">\n";
echo "</body>\n";
echo "<head>\n";
echo "<meta http-equiv=\"Expires\" content=\"0\">\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "</head>\n";
echo "</html>\n";

?>
