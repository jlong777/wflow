#!/bin/bash

# ver 0.2

# if there is a dot file, insert the new node string; else create dot file

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

#echo $1  > $1/blah
#echo $2 >> $1/blah
#echo $3 >> $1/blah
#echo $4 >> $1/blah
#echo $5 >> $1/blah
#echo $6 >> $1/blah
#echo $7 >> $1/blah

wdir=$1
wsdl=$2
label="label=\"$3\""
func=$4
node_num=$5
server=$6
dot=$7
#echo $label >> $1/blah

# the URL string points to a page that allows the user to set the parameters
# for a node in a workflow
URL="URL=\"${server}/wflow/edit.php?func=$func&node=$node_num&wdir=$wdir&wsdl=$wsdl&random_num=$RANDOM\""

if [ -e $wdir/graph.dot ]
then
  line_num=`cat $wdir/graph.dot | grep -n "^}" | head -1 | cut -d: -f1`
  line_num=$((line_num - 1))
  cat $wdir/graph.dot | sed "${line_num}a\\
${func}_$node_num\ [$URL, $label]; \\
  " > $wdir/graph.dot.temp
  mv $wdir/graph.dot.temp $wdir/graph.dot
else
  echo "digraph G"                                             > $wdir/graph.dot
  echo "{"                                                    >> $wdir/graph.dot
  echo "node [shape=\"Mrecord\", fontname=\"Times-Bold\", fontsize=\"9\", style=\"bold\"];" >> $wdir/graph.dot
  echo "rankdir=LR;"                                          >> $wdir/graph.dot
  echo "${func}_$node_num"" [$URL, $label];"                  >> $wdir/graph.dot
  echo "}"                                                    >> $wdir/graph.dot
fi

# 3) generate html for the new dot file

$dot -Tcmap $wdir/graph.dot -o $wdir/map.htm            # makes clickable co-ords
$dot -Tjpg $wdir/graph.dot -o $wdir/map_${node_num}.jpg # makes map jpeg
