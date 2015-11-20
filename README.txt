WFLOW - Remote WSDL Workflows
=============================
Workflows may be built using web services as building blocks. WFLOW is a
browser-based workflow editor that uses Tigra Tree Menu, Graphviz, gSOAP, and
bashlib to build, display, and invoke web services workflows that run entirely
on a remote server.

Tigra Tree Menu

Tigra Tree Menu (http://www.softcomplex.com/products/tigra_tree_menu/), an
open-source JavaScript menu generator, is used to display the services
published at a site in its WSDL (web services description language) document.
Uploading a simple text file containing the URLs of desired WSDLs initializes
the editor and menu. Nodes are added to a workflow by clicking on menu items.

Graphviz

Graphviz (http://www.graphviz.org) is an open-source graph visualization package
that includes an option to produce clickable bitmaps for web pages. This feature
is utilized by WFLOW to build a workflow where each node of the graph represents
a web service that can be configured by clicking on it. Configuration options
are displayed, along with drop-down menus that list potential edge sinks on
downstream nodes. Future work could incorporate an ontology for our WSDL to
further constrain graph semantics.

gSOAP

gSOAP (http://www.cs.fsu.edu/~engelen/soap.html) is an open-source web services
development toolkit that offers the ability to invoke web services in C/C++.
Pressing the "Run Workflow" button creates a C source file from the graph using
gSOAP calls, which is then compiled and run. Once invoked, workflows do not
require the client running the browser to stay online; status and results of the
job are output to a web page that can be viewed at any time. The C source code
(client.c) can be saved and rerun by hand or incorporated into a larger script
(see NOTES below).

bashlib

bashlib (http://bashlib.sourceforge.net) "is a shell script that makes CGI
programming in the bash shell easier, or at least more tolerable."

NOTES
=====
Directory structure used for data by client.c is as follows:
1) a DATA directory in the same directory as client.c
2) in the DATA directory, a node directory for each node whose name is that
   displayed at the top of the node's image in the jpg of the workflow.
3) inside each node directory, a file for each input parameter required by the 
   web service, the name of the file the same name as the parameter, the
   contents of the file holding the value of the parameter.

Example:
# ls -l DATA
total 0
drwxr-xr-x    6 www  admin  204 Dec  5 10:29 .
drwxrwxr-x   20 www  admin  680 Dec  5 10:44 ..
drwxr-xr-x    8 www  admin  272 Dec  5 10:29 BLASTALL_SIMPLE_2
drwxr-xr-x    5 www  admin  170 Dec  5 10:29 GET_FILE_3
drwxr-xr-x    7 www  admin  238 Dec  5 10:26 GET_TICKET_0
drwxr-xr-x    9 www  admin  306 Dec  5 10:40 SEQRET_FULL_1
# ls -l DATA/BLASTALL_SIMPLE_2
total 48
drwxr-xr-x   8 www  admin  272 Dec  5 10:29 .
drwxr-xr-x   6 www  admin  204 Dec  5 10:29 ..
-rw-r--r--   1 www  admin    1 Dec  5 10:40 BLOCKING
-rw-r--r--   1 www  admin    6 Dec  5 10:40 blastall
-rw-r--r--   1 www  admin    2 Dec  5 10:40 nucleotid_db
-rw-r--r--   1 www  admin    2 Dec  5 10:40 protein_db
# more DATA/BLASTALL_SIMPLE_2/BLOCKING
1
# more DATA/BLASTALL_SIMPLE_2/blastall
blastn
# more DATA/BLASTALL_SIMPLE_2/nucleotid_db
nt
# more DATA/BLASTALL_SIMPLE_2/protein_db
nr
#

