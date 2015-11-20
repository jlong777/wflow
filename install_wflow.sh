#!/bin/bash

# ver 0.2

# edit these for your site
HTML_ROOT=/var/www   # html, php pages root dir
CGI_ROOT=/usr/lib/cgi-bin  # cgi-bin root dir
DOT=/usr/bin/dot           # location of "dot" from graphviz
CURL=/usr/bin/curl         # location of curl
SCRATCH=/tmp               # location of scratch dir with 777 permissions
GSOAP=/usr/bin             # location of soapcpp2, and wsdl2h
GSOAPch=/usr/include       # location of stdsoap2.[ch]
WFLOW_SERVER=http://127.0.0.1
WWW_USER=www-data          # files will be owned & run by this user
CC=gcc

# prepare include.php
HTML_ROOT=`echo $HTML_ROOT | sed 's/\//\\\\\//g'`
CGI_ROOT=`echo $CGI_ROOT | sed 's/\//\\\\\//g'`
DOT=`echo $DOT | sed 's/\//\\\\\//g'`
CURL=`echo $CURL | sed 's/\//\\\\\//g'`
SCRATCH=`echo $SCRATCH | sed 's/\//\\\\\//g'`
WFLOW_SERVER=`echo $WFLOW_SERVER | sed 's/\//\\\\\//g'`
cat html/include.in | sed "s/^HTML_ROOT/\$HTML_ROOT = \"$HTML_ROOT\";/"\
                    | sed "s/^CGI_ROOT/\$CGI_ROOT = \"$CGI_ROOT\";/"\
                    | sed "s/^SCRATCH/\$SCRATCH = \"$SCRATCH\";/"\
                    | sed "s/^WFLOW_SERVER/\$WFLOW_SERVER = \"$WFLOW_SERVER\";/"\
                    | sed "s/^curl/\$curl = \"$CURL\";/"\
                    | sed "s/^dot/\$dot = \"$DOT\";/"\
		    > html/include.php

HTML_ROOT=`echo $HTML_ROOT | sed 's/\\\//g'`
CGI_ROOT=`echo $CGI_ROOT | sed 's/\\\//g'`
DOT=`echo $DOT | sed 's/\\\//g'`
CURL=`echo $CURL | sed 's/\\\//g'`
SCRATCH=`echo $SCRATCH | sed 's/\\\//g'`
WFLOW_SERVER=`echo $WFLOW_SERVER | sed 's/\\\//g'`

if [ ! -d $HTML_ROOT/wflow ]
then
  mkdir $HTML_ROOT/wflow
fi
if [ ! -d $HTML_ROOT/wflow/icons ]
then
  mkdir $HTML_ROOT/wflow/icons
fi
if [ ! -d $SCRATCH ]
then
  mkdir $SCRATCH
fi
chmod 777 $SCRATCH

cp html/* $HTML_ROOT/wflow
rm html/include.php
cp icons/* $HTML_ROOT/wflow/icons
rm $HTML_ROOT/wflow/include.in
ln -s $SCRATCH $HTML_ROOT/wflow/gsoap

if [ ! -d $CGI_ROOT/wflow ]
then
  mkdir $CGI_ROOT/wflow
fi
cp cgi/* $CGI_ROOT/wflow

if [ ! -d /usr/local/lib ]
then
  mkdir -p /usr/local/lib
fi
cp bashlib /usr/local/lib

if [ ! -d $SCRATCH/gSOAP ]
then
  mkdir $SCRATCH/gSOAP
fi
cp $GSOAP/soapcpp2    $SCRATCH/gSOAP
cp $GSOAP/wsdl2h      $SCRATCH/gSOAP
cp $GSOAPch/stdsoap2* $SCRATCH/gSOAP

cd $HTML_ROOT/wflow

for i in `ls *.c`
do
  name=`echo $i | sed "s/.c$//"`
  $CC -O2 -o $name $i
done

chown -R $WWW_USER:$WWW_USER $CGI_ROOT/wflow
chown -R $WWW_USER:$WWW_USER $HTML_ROOT/wflow
chown -R $WWW_USER:$WWW_USER $SCRATCH/gSOAP
