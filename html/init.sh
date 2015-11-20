#!/bin/bash                                           
#  ver 0.2                                     05/2013  jlong@alaska.edu

# init.sh - create a unique directory to hold the workflow
# the name is in the format D12-15-04_06:13:08_1075
# i.e., 12/15/04 at 06:13:08 a.m. followed by the PID of this shell

workdir=`date +D%m-%d-%y_%H:%M:%S_$$`
mkdir ${1}/$workdir

cd ${1}/$workdir
chmod 775 ./

# copy needed files to workdir

cp -R ${2}/icons ./
cp ${2}/tree.js ./
cp ${2}/tree.html ./
cp ${2}/tree_tpl.js ./

# sed is not needed yet, am anticipating the ability to upload a workflow
cat ${2}/main.html | sed "s/value=\"wdir\"/value=\"$workdir\"/" > main.html
    
echo "$workdir"

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
