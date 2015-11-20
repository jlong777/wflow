<?php

# do_edit.php - perform the desired edit

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

include("include.php");

# capture input params

$piece = explode("&", $_SERVER['QUERY_STRING']);

$i=0;
while(isset($piece[$i]))
{
  $tok = strtok($piece[$i], "=");
  $param_name[] = $tok;
  $i++;
}

$my_argc = $i;

# first two inputs are always $wflow_wdir, and $wflow_nodename
for($i=0;$i<$my_argc; $i++)
{
  $input[$i] = $_GET[$param_name[$i]]; 
}
$wflow_wdir = $input[0];
$wflow_nodename = $input[1];

# record the input data - a data directory will hold a directory for each node,
# each node directory will have a text file for each parameter

if(!file_exists("${wflow_wdir}/DATA"))
{
  mkdir("$wflow_wdir/DATA", 0755);
}

if(!file_exists("${wflow_wdir}/DATA/$wflow_nodename"))
{
  mkdir("$wflow_wdir/DATA/$wflow_nodename", 0755);
}

for($i=0;$i<$my_argc; $i++)
{
  if(strlen($input[$i]) && !strstr($param_name[$i], "_inedge"))
  {
    $fp = fopen("$wflow_wdir/DATA/$wflow_nodename/$param_name[$i]", "w") or die("file open error...");
    fwrite($fp, $input[$i]);
    fclose($fp);
    # scrub input
    # convert dos and mac lines to unix
    exec("cat $wflow_wdir/DATA/$wflow_nodename/$param_name[$i] | tr '\r' '\n' | tr -s '\n' > $wflow_wdir/scrub");
    # if the input is 0 or 1 lines long, assume it is not a file, 
    # but a parameter, and so remove any \n
    exec("wc -l $wflow_wdir/scrub | tr -s ' ' | cut -d' ' -f2", $output);
    foreach($output as $linecount);
    if($linecount < 2)
      exec("cat $wflow_wdir/scrub | tr -d '\n' > $wflow_wdir/DATA/$wflow_nodename/$param_name[$i]");
    else
    {
      exec("echo '\n' >> $wflow_wdir/scrub");
      exec("cat $wflow_wdir/scrub | tr -s '\n' > $wflow_wdir/DATA/$wflow_nodename/$param_name[$i]");
    }
    unlink("$wflow_wdir/scrub");
  }
  else
  {
    if(!strlen($input[$i]) && file_exists("$wflow_wdir/DATA/$wflow_nodename/$param_name[$i]")) # delete any existing value
      unlink("$wflow_wdir/DATA/$wflow_nodename/$param_name[$i]");
  }
  # redirect this input to another node? if yes, insert new edge
  if(strstr($param_name[$i], "_inedge") && strlen($input[$i]))
  {
    $tok = explode("_inedge", $param_name[$i]);
    exec("$WFLOW_DIR/new_edge.sh $input[$i] $wflow_wdir $wflow_nodename $tok[0]");
  }
}

if(file_exists("${wflow_wdir}/node_num"))
{
  exec("cat $wflow_wdir/node_num", $output);
  foreach($output as $node_num);
}
else
  $node_num = 0;

# wflow_edge param is edge to delete
for($i=0;$i<$my_argc; $i++)
  if(!strcmp($param_name[$i], "wflow_edge") && strlen($input[$i]))
  {
    exec("cat $wflow_wdir/graph.dot | grep -v \"$input[$i]\" > $wflow_wdir/graph.dot.tmp");
    rename("$wflow_wdir/graph.dot.tmp", "$wflow_wdir/graph.dot");
  }

# insert new edge statement, change random numbers, and regenerate map

for($i=0;$i<$my_argc; $i++)
  if(!strcmp($param_name[$i], "wflow_input") && strlen($input[$i]))
    exec("$WFLOW_DIR/new_edge.sh $input[$i] $wflow_wdir $wflow_nodename");
    
# wflow_delete set means delete the node
for($i=0;$i<$my_argc; $i++)
  if(!strcmp($param_name[$i], "wflow_delete") && strlen($input[$i]))
  {
    exec("cat $wflow_wdir/graph.dot | grep -v \"$wflow_nodename\" > $wflow_wdir/graph.dot.tmp");
    rename("$wflow_wdir/graph.dot.tmp", "$wflow_wdir/graph.dot");
  }

# change random numbers in URL strings to prevent caching web page

exec("$WFLOW_DIR/new_url.sh $wflow_wdir");

exec("$dot -Tcmap $wflow_wdir/graph.dot -o $wflow_wdir/map.htm");            # makes clickable co-ords
exec("$dot -Tjpg $wflow_wdir/graph.dot -o $wflow_wdir/map_${node_num}.jpg"); # makes map jpeg

# solve potential local caching problems by changing name of map.jpg each time

$last_node_num = $node_num - 1;
if(file_exists("${wflow_wdir}/map_${last_node_num}.jpg"))
  unlink("$wflow_wdir/map_${last_node_num}.jpg");

$last_node_num = $node_num;
$node_num = $node_num + 1;
exec("echo $node_num > $wflow_wdir/node_num");

# is the graph ready to run (not implemented yet, just give the button)
# exec("$WFLOW_DIR/runable.sh $wflow_wdir");
touch("${wflow_wdir}/runable");

echo "<html>\n";
echo "<head>\n";
echo "<meta http-equiv=\"Expires\" content=\"-1\">\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "</head>\n";
echo "<body>\n";

echo "<map name=\"graphmap\">\n";

$fp = fopen("$wflow_wdir/map.htm", "r") or die("arrrggg");
while(!feof($fp))
{
  $line = fgets($fp, 1024);
  echo $line;
}
fclose($fp);

exec("echo $wflow_wdir | cut -d\"/\" -f3", $output);
foreach($output as $wdir);

echo "</map>\n";
echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$wdir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">\n";

if(file_exists("${wflow_wdir}/runable"))
{
  echo "<form method=\"get\" action=\"/cgi-bin/wflow/run_workflow.sh\">\n";
  echo "<input name=\"wdir\" type=\"hidden\" value=\"$wflow_wdir\">\n";
  echo "<input name=\"gsoap\" type=\"hidden\" value=\"$GSOAP\">\n";
  echo "<input name=\"wflow_dir\" type=\"hidden\" value=\"$WFLOW_DIR\">\n";
  echo "<input name=\"wflow_cgi\" type=\"hidden\" value=\"$WFLOW_CGI\">\n";
  echo "<input type=\"submit\" value=\"Run Workflow\">\n";
  echo "</form>\n";
  unlink("$wflow_wdir/runable");
}

echo "</body>\n";
echo "<head>\n";
echo "<meta http-equiv=\"Expires\" content=\"-1\">\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "</head>\n";
echo "</html>\n";

?>
