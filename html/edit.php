<?php

# display an edit page for a node, either a custom pise type page or a 
# plain page generated from the param names (edit_node)
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
echo "<meta http-equiv=\"Expires\" content=\"-1\">\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "<title>\n";

include("include.php");
$func = $_GET['func'];
$node = $_GET['node'];
$wdir = $_GET['wdir'];
$wsdl = $_GET['wsdl'];

##### need code here to check for pise-type page


# this is the plain page generated just from the param names
exec("$WFLOW_DIR/edit_node $wdir ${func}_$node");

if(file_exists("$wdir/edit_num"))
{
  exec("cat $wdir/edit_num", $output);
  foreach($output as $edit_num);
}
else
  $edit_num = 0;

echo "WFLOW: edit number $edit_num\n";
echo "</title>\n";
echo "</head>\n";

echo "<body>\n";
srand(time());
$random = (rand());
echo "<form method=\"get\" action=\"/wflow/do_edit.php?rand_num=$random\">\n";
echo "<input name=\"wflow_wdir\" type=\"hidden\" value=\"$wdir\">\n";
echo "<input name=\"wflow_nodename\" type=\"hidden\" value=\"${func}_$node\">\n";

$fp = fopen("$wdir/node.html", "r") or die("file open error...");
while(!feof($fp))
{
  $line = fgets($fp, 1024);
  echo $line;
}
fclose($fp);

echo "<br>\n";

echo "<input type=\"submit\" value=\"Update\">\n";
echo "</form></body>\n";
echo "<head>\n";
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
echo "<meta http-equiv=\"Expires\" content=\"-1\">\n";
echo "</head></html>\n";

$edit_num = $edit_num + 1;
exec("echo $edit_num > $wdir/edit_num");

?>
