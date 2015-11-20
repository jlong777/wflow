#!/bin/bash

# ver 0.2

# new_edge.sh - insert new edge statement

input=$1
wdir=$2
nodename=$3

target_node=`echo $input | cut -f1 -d:`
target_param=`echo $input | cut -f2 -d:`
line_num=`cat $wdir/graph.dot | grep -n "^$target_node" | head -1 | cut -d: -f1`
#new_url=`cat $wdir/graph.dot | grep "label=" | grep $target_node | \
#         sed "s/<${target_param}>a*[0-9]*.\{0,1\}[0-9]*|/<in_a>a|/" | \
#         sed "s/<${target_param}>b*[0-9]*.\{0,1\}[0-9]*}/<in_b>b}/"`
#echo "$new_url" > $wdir/urlfile
#cat $wdir/graph.dot | sed "${line_num}c\\
#$new_url \\
#" > $wdir/graph.dot.tmp
#mv $wdir/graph.dot.tmp $wdir/graph.dot
line_num=`cat $wdir/graph.dot | grep -n "^}" | head -1 | cut -d: -f1`
line_num=$((line_num - 1))


if [ ! -z $4 ]            # redirect an input
then
cat $wdir/graph.dot | sed "${line_num}a\\
${nodename}:in_$4\ ->\ ${input}; \\
" > $wdir/graph.dot.tmp
else                      # redirect an output
cat $wdir/graph.dot | sed "${line_num}a\\
${nodename}:out\ ->\ ${input}; \\
" > $wdir/graph.dot.tmp
fi

mv $wdir/graph.dot.tmp $wdir/graph.dot

# Copyright (C) 2005, University of Alaska Fairbanks
# Biotechnology Computing Research Group
# http://iab-devel.iab.alaska.edu
# jim.long@iab.alaska.edu
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
