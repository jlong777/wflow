#!/bin/bash

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

client_pid=$1
wdir=$2
html_dir=`echo $wdir | cut -d"/" -f3`

cd $wdir/gsoap

if [ -e $wdir/node_num ]
then
  node_num=`cat $wdir/node_num`
else
  node_num=0
fi

last_node_num=$((node_num - 1))

while (ps -p $client_pid | grep $client_pid > /dev/null)
do 
  echo "<html>"                                             > ../index.temp
  echo "<head>"                                            >> ../index.temp
  echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">" >> ../index.temp
  echo "<meta http-equiv=\"Expires\" content=\"0\">"       >> ../index.temp
  echo "<meta http-equiv=\"refresh\" content=\"20\">"      >> ../index.temp
  echo "</head>"                                           >> ../index.temp
  echo "<body>"                                            >> ../index.temp

  echo "<map name=\"graphmap\">"                           >> ../index.temp
  cat $wdir/map.htm                                        >> ../index.temp
  echo "</map>"                                            >> ../index.temp

  echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">" >> ../index.temp
  echo "<br>"                                              >> ../index.temp
  echo "<pre>"                                             >> ../index.temp
  echo "Copy this <a href=\"output.html\">link</a> to visit this page later" >> ../index.temp
  echo "<br>"                                              >> ../index.temp
  cat outfile                                              >> ../index.temp
  echo "</pre>"                                            >> ../index.temp
  echo "</body>"                                           >> ../index.temp
  echo "</html>"                                           >> ../index.temp
  
  mv ../index.temp ../output.html
  sleep 20
done

# all done, don't refresh anymore

sleep 10

echo "<html>"                                               > ../index.temp
echo "<head>"                                              >> ../index.temp
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">"   >> ../index.temp
echo "<meta http-equiv=\"Expires\" content=\"0\">"         >> ../index.temp
echo "</head>"                                             >> ../index.temp
echo "<body>"                                              >> ../index.temp

echo "<map name=\"graphmap\">"                             >> ../index.temp
cat $wdir/map.htm                                          >> ../index.temp
echo "</map>"                                              >> ../index.temp

echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">" >> ../index.temp
echo "<br>"                                                >> ../index.temp
echo "<pre>"                                               >> ../index.temp
echo "Copy this <a href=\"output.html\">link</a> to visit this page later" >> ../index.temp
echo "<br>"                                                >> ../index.temp
cat outfile                                                >> ../index.temp
echo "<br>"                                                >> ../index.temp
echo "<br>"                                                >> ../index.temp
echo "View <a href=\"gSOAP/compile_file\">compilation output</a><br>" >> ../index.temp
echo "View <a href=\"gSOAP/client.c\">source code</a><br>" >> ../index.temp
echo "</pre>"                                              >> ../index.temp
echo "</body>"                                             >> ../index.temp
echo "</html>"                                             >> ../index.temp
mv ../index.temp ../output.html
