<!-- ver 0.2                                       05/2013  jlong@alaska.edu -->
<html><head>
<title>WFLOW</title>
</head>
<frameset rows="*" cols="150,*">

<?php

# this page is called from a form that provides wsdl locations

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

include("include.php");

# initialize the environment
exec("$WFLOW_DIR/init.sh $SCRATCH $WFLOW_DIR", $output);
foreach($output as $workdir);
$sed_dir = $workdir;
$workdir = "$SCRATCH/$workdir";

# deal with the uploaded wsdl file
foreach($_FILES as $fname=>$farray)
{
  if(is_uploaded_file($farray['tmp_name']))
    move_uploaded_file($farray['tmp_name'], "$workdir/$farray[name]");
}

# generate tree_items from wsdls
$tree_items = fopen("$workdir/tree_items.js", "w");
fwrite($tree_items, "var TREE_ITEMS = [\n");
fwrite($tree_items, "  ['WSDL', 'null',\n");

# get each wsdl location
$wsdl_num = 0;
if($fp = fopen("$workdir/$farray[name]", "r"))
  do
  {
    $wsdl = fgets($fp, 1024);
    $wsdl = trim($wsdl);
    if(strlen($wsdl))
    {
      exec("$curl $wsdl > $workdir/wsdl_$wsdl_num");
      
      # get just the name of the wsdl
      $tok = strtok($wsdl, "/");
      while ($tok)
      {
        $last_tok = $tok;
        $tok = strtok("/");
      }
      fwrite($tree_items, "    ['$last_tok', '$wsdl',\n");

      # use the php expat routines to parse the wsdl for the operation names
      readWsdl("$workdir/wsdl_$wsdl_num"); # from include.php

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
    
      # now go to the operation tags within the portType block and get the names
      for($i=$start+1; $i<$end; $i++)
        foreach($wsdl_values[$i] as $key=>$val)
	  if((!strcasecmp($key, "tag")) && (!strcasecmp($val, "operation")))
	    foreach($wsdl_values[$i] as $key=>$val)
	      if(!strcasecmp($key, "attributes"))
	        foreach($val as $a_key=>$a_val)
		  if(!strcasecmp($a_key, "name"))
                    fwrite($tree_items, "      ['$a_val','/wflow/node.php?func=$a_val&wsdl=$wsdl_num'],\n");
      $wsdl_num = $wsdl_num + 1;
      fwrite($tree_items, "    ]");
  
      if(!feof($fp))
        fwrite($tree_items, ",\n");
      else
        fwrite($tree_items, "\n");
    }
  } 
  while(!feof($fp));

fwrite($tree_items, "  ]\n");
fwrite($tree_items, "];\n");
fclose($tree_items);
fclose($fp);

# add workdir info to tree_items.js
$string = "s/\?/\?wdir=${sed_dir}\&/g";
$string = "sed \"'$string'\"";
$string = "cat $workdir/tree_items.js \| $string \> $workdir/tree_items.temp";
exec("echo '#!/bin/bash' > $workdir/exec_sed");
exec("echo $string >> $workdir/exec_sed");
exec("chmod 755 $workdir/exec_sed");
exec("$workdir/exec_sed");
unlink("$workdir/exec_sed");
rename("$workdir/tree_items.temp", "$workdir/tree_items.js");

exec("echo $workdir | cut -d\"/\" -f3", $output);
foreach($output as $wdir);

echo "<frame src=\"$WFLOW_SERVER/wflow/gsoap/$wdir/tree.html\" name=\"menuFrame\" scrolling=\"AUTO\" noresize>\n";
echo "<frame src=\"$WFLOW_SERVER/wflow/gsoap/$wdir/main.html\" name=\"mainFrame\" scrolling=\"AUTO\">\n";

?>

</frameset>
<noframes>
<body>

</body>
</noframes>
</html>
