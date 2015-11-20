#!/bin/bash

# ver 0.2

# wrapper to run a workflow

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

# use the cgi bashlib
. /usr/local/lib/bashlib

wdir=`param wdir`
GSOAP=`param gsoap`
WFLOW_DIR=`param wflow_dir`
WFLOW_CGI=`param wflow_cgi`
html_dir=`echo $wdir | cut -d"/" -f3`

cd $wdir

if [ -e $wdir/node_num ]
then
  node_num=`cat $wdir/node_num`
else
  node_num=0
fi

last_node_num=$((node_num - 1))

echo "<html>"                                             > output.html
echo "<head>"                                            >> output.html
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">" >> output.html
echo "<meta http-equiv=\"Expires\" content=\"0\">"       >> output.html
echo "<meta http-equiv=\"refresh\" content=\"20\">"      >> output.html
echo "</head>"                                           >> output.html
echo "<body>"                                            >> output.html

echo "<map name=\"graphmap\">"                           >> output.html
cat $wdir/map.htm                                        >> output.html
echo "</map>"                                            >> output.html

echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">" >> output.html
echo "<br>"                                              >> output.html
echo "<pre>"                                             >> output.html

echo "</pre>"                                            >> output.html
echo "</body>"                                           >> output.html
echo "</html>"                                           >> output.html


echo "Content-type: text/html"
echo ""

echo "<html>"
echo "<head>"
echo "<meta http-equiv=\"refresh\" content=\"5;url=$WFLOW_SERVER/wflow/gsoap/$html_dir/output.html\">"
echo "</head>"
echo "<body>"
echo "<map name=\"graphmap\">"
cat $wdir/map.htm
echo "</map>"

echo "<img src=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/map_${last_node_num}.jpg\" usemap=\"#graphmap\" border=\"0\">"
echo "<br>"
echo "<pre>"
echo "Running wsdl2h and gSOAP compiler...<br>"


# parse the workflow graph, and output the order of nodes to process

if [ -e $wdir/node_order ]
then
  rm $wdir/node_order
fi
touch $wdir/node_order

$WFLOW_DIR/output_order $wdir

# run wsdl2h on each wsdl file, concatenating the resulting .h files into one .h

if [ -e $wdir/gSOAP ]
then
  rm -rf $wdir/gSOAP
fi

mkdir $wdir/gSOAP
chmod 755 $wdir/gSOAP

wsdl_num=0
while [ -e $wdir/wsdl_$wsdl_num ]
do
  $GSOAP/wsdl2h -c $wdir/wsdl_$wsdl_num -o $wdir/gSOAP/n${wsdl_num}wflow.h -n n${wsdl_num}wflow
  cat $wdir/gSOAP/n${wsdl_num}wflow.h >> $wdir/gSOAP/wflow_head.h
  wsdl_num=$((wsdl_num + 1))
done

# run the gSOAP compiler
$GSOAP/soapcpp2 -c -I$GSOAP -d $wdir/gSOAP $wdir/gSOAP/wflow_head.h

# for each node to process, determine parameter names
# each node output has a unique name that may serve as the input name to a child
# create a C sourcefile with the proper order and names, compile, and run

cd $wdir/gSOAP

if [ -r callfile ]
then
  rm callfile
fi

if [ -r tempfile ]
then
  rm tempfile
fi

echo "Creating C source code of gSOAP calls to run graph...<br>"

echo "#include \"soapH.h\""                              > client.c
echo "#include <stdio.h>"                               >> client.c
echo "#include <stdlib.h>"                              >> client.c
echo "#include <string.h>"                              >> client.c
echo "#include <sys/stat.h>"                            >> client.c
echo "#include <sys/types.h>"                           >> client.c
echo "#include <unistd.h>"                              >> client.c
echo ""                                                 >> client.c

# declare the namespace tables
for i in `ls ../wsdl_*`
do
  j=`echo $i | sed "s/\.\.\///"`
  i=$j
  echo "SOAP_NMAC struct Namespace ${i}[] = "           >> client.c
#  cat $i | grep -v "#include" | grep -v "namespaces\[\]" | \
#  grep "[^$]"                                           >> client.c
##  grep "[^$]" | sed "s/^/  /"                           >> client.c
##### next line is a hack
  nsmap=`cat ../$i | grep "definitions name=" | sed "s/.*name=//" |\
         sed "s/\"//g"`
  cat ${nsmap}.nsmap | grep -v "#include" | grep -v "namespaces\[\]" | \
  grep "[^$]"                                           >> client.c
done

echo ""                                                 >> client.c
echo "struct Namespace *namespaces;"                    >> client.c
echo ""                                                 >> client.c
echo "char * get_data(char *, char *);"                 >> client.c
echo ""                                                 >> client.c
echo "int main(int argc, char **argv)"                  >> client.c
echo "{"                                                >> client.c
echo "  FILE * outfile;"                                >> client.c
echo "  pid_t pid;"                                     >> client.c
echo "  char buffer[1026];"                             >> client.c
echo "  char num[64];"                                  >> client.c

# declare and set variables & print function calls
echo "  struct soap soap;"                              >> client.c
for i in `cat $wdir/node_order`
do
  # which wsdl does this service use?
  wsdl=`cat $wdir/graph.dot | grep $i | grep wsdl | cut -d" " -f2 |\
        sed "s/.*wsdl=/wsdl_/" | sed "s/&.*//"`
  ##### location hack for now
  location=`tail -50 $wdir/$wsdl | grep location= | sed "s/\/>//" | \
            sed "s/.*location=//"`
  $WFLOW_DIR/declare_vars $wdir $i $wsdl $location
done

rm $wdir/node_order

echo ""                                                 >> client.c
echo "  soap_init(&soap);"                              >> client.c
echo ""                                                 >> client.c
echo "  buffer[0] = num[0] = 0x0;"                      >> client.c
echo "  pid = getpid();"                                >> client.c
echo "  sprintf(num, \" %d \", pid);"                   >> client.c
echo "  strcat(buffer, \"$WFLOW_CGI/monitor_me.sh \");" >> client.c
echo "  strcat(buffer, num);"                           >> client.c
echo "  strcat(buffer, argv[1]);"                       >> client.c
echo "  strcat(buffer, \" &\");"                        >> client.c
echo "  system(buffer);"                                >> client.c

# parameter initializations are in "tempfile"
cat tempfile                                            >> client.c

# function calls are in "callfile"
cat callfile                                            >> client.c

echo ""                                                 >> client.c
echo "  outfile = fopen(\"outfile\", \"a\");"           >> client.c
echo "  fprintf(outfile, \"\n\nWFLOW done.\n\");"       >> client.c
echo "  fflush(NULL);"                                  >> client.c
echo "  fclose(outfile);"                               >> client.c
echo "  return 0;"                                      >> client.c
echo "}"                                                >> client.c

echo ""                                                 >> client.c
echo "char * get_data(char * node, char * param)"       >> client.c
echo "{"                                                >> client.c
echo "  char buffer[1024], * ret_pt;"                   >> client.c
echo "  long size;"                                     >> client.c
echo "  struct stat sbuf;"                              >> client.c
echo "  FILE * infile;"                                 >> client.c
echo ""                                                 >> client.c
echo "  buffer[0] = 0x0;"                               >> client.c
echo "  strcat(buffer, \"../DATA/\");"                  >> client.c
echo "  strcat(buffer, node);"                          >> client.c
echo "  strcat(buffer, \"/\");"                         >> client.c
echo "  strcat(buffer, param);"                         >> client.c
echo "  if(stat(buffer, &sbuf) == -1)"                  >> client.c
echo "  {"                                              >> client.c
echo "    fprintf(stderr, \"client: error getting file stats for %s, returning...\n\", buffer);" >> client.c
echo "    fflush(NULL);"                                >> client.c
echo "    return NULL;"                                 >> client.c
echo "  }"                                              >> client.c
echo ""                                                 >> client.c
echo "  ret_pt = (char *) malloc(sbuf.st_size + 1);"    >> client.c
echo ""                                                 >> client.c
echo "  infile = fopen(buffer, \"r\");"                 >> client.c
echo "  if(infile == NULL)"                             >> client.c
echo "  {"                                              >> client.c
echo "    fprintf(stderr, \"client: file open error, %s\n\", buffer); " >> client.c
echo "    fflush(NULL);"                                >> client.c
echo "    return NULL;"                                 >> client.c
echo "  }"                                              >> client.c
echo ""                                                 >> client.c
echo "  size = fread(ret_pt, 1, sbuf.st_size, infile);" >> client.c
echo "  if(size != sbuf.st_size)"                       >> client.c
echo "  {"                                              >> client.c
echo "    fprintf(stderr, \"client: error reading file %s, returning...\n\", buffer);" >> client.c
echo "    fflush(NULL);"                                >> client.c
echo "    return NULL;"                                 >> client.c
echo "  }"                                              >> client.c
echo ""                                                 >> client.c
echo "  ret_pt[sbuf.st_size] = 0x0;"                    >> client.c
echo "  return ret_pt;"                                 >> client.c
echo "}"                                                >> client.c

# compile and run

echo "Compiling code...<br>"

cp $GSOAP/stdsoap2.c $wdir/gSOAP
cp $GSOAP/stdsoap2.h $wdir/gSOAP

echo "Output from compilation:"                    > compile_file
echo ""                                           >> compile_file
gcc -c -DWITH_NONAMESPACES -Wall -I. soapC.c      >> compile_file 2>&1
gcc -c -DWITH_NONAMESPACES -Wall -I. soapClient.c >> compile_file 2>&1
gcc -c -DWITH_NONAMESPACES -Wall -I. stdsoap2.c   >> compile_file 2>&1
gcc    -DWITH_NONAMESPACES -Wall -I. -o client client.c *.o >> compile_file 2>&1

echo "Done, beginning to run workflow...<br>"

echo "<br>"
echo "<br>"
echo "View <a href=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/gSOAP/compile_file\" target=\"_blank\">compilation output</a><br>"
echo "View <a href=\"$WFLOW_SERVER/wflow/gsoap/$html_dir/gSOAP/client.c\" target=\"_blank\">source code</a><br>"
echo "</pre>"
echo "</body>"
echo "</html>"

# detach child (redirect all streams so kill won't hang)
./client $wdir < /dev >& /dev/null &
kill -9 $$
