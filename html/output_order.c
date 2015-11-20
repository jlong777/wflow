/* output_order.c                              5/2013  jlong@alaska.edu
 *
 * ver 0.2
 *
 * parse a workflow graph, and output the order of nodes to process
 * assumes a directed, acyclic graph (DAG)
 *
 * usage: output_order <directory containing graph.dot file>
 *
 * Copyright (C) 2005, University of Alaska Fairbanks
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA.
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN       1024
#define NUM_NODES      100
#define PARAM_LEN      256

void print_parentless_nodes(char *);

int main(int argc, char *argv[])
{
  char buffer[BUF_LEN], newfile[BUF_LEN];
  FILE * infile, * outfile;
  
  /* first parse for all node names */
  buffer[0] = newfile[0] = 0x0;
  strcat(buffer, argv[1]); 
  strcat(buffer, "/graph.dot");
  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "output_order: error opening %s, exiting...\n", buffer); 
    exit(1);
  }

  strcpy(newfile, buffer);
  strcat(newfile, "_temp");
  outfile = fopen(newfile, "w");
  if(outfile == NULL)
  {
    fprintf(stderr, "output_order: error opening %s, exiting...\n", newfile); 
    exit(1);
  }
  
  while(fgets(buffer, BUF_LEN, infile))
    fprintf(outfile, "%s", buffer);
  
  fclose(infile);
  fclose(outfile);
  
  print_parentless_nodes(argv[1]);
  
  strcpy(buffer, "rm ");
  strcat(buffer, newfile);
  system(buffer);
}

void print_parentless_nodes(char *path)
{
  /* the algorithm is as follows:
   * 1) open graph file, and make a list of all nodes
   * 2) if there aren't any nodes, exit
   * 3) eliminate all nodes that are sinks from the list
   * 4) print out what is left in the list, i.e. ouput nodes with no parents
   * 5) output a new graph file with the nodes in the list removed
   * 6) go to step 1
   */
  int nodes_exist=0;
  int i, j, line_num=0;
  char (*graph)[BUF_LEN];
  char buffer[BUF_LEN], graphfile[BUF_LEN], newfile[BUF_LEN], *p, *q;
  char node_names[NUM_NODES][PARAM_LEN], node_name[PARAM_LEN];
  FILE * infile, * outfile;

  graphfile[0] = newfile[0] = 0x0;
  strcat(graphfile, path);
  strcat(graphfile, "/graph.dot_temp");
  strcat(newfile, path);
  strcat(newfile, "/node_order");

  /* first parse for all node names */
  infile = fopen(graphfile, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "output_order: error opening %s, exiting...\n", graphfile); 
    exit(1);
  }

  /* initialize */
  for(i=0; i<NUM_NODES; i++) node_names[i][0] = 0x0;
  
  /* parse */
  i = 0;
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "label=")) /* at a node def */
    {
      nodes_exist = 1; /* there are > 0 nodes defined in the file */
      
      /* record node name */
      j = 0;
      while(buffer[j] != ' ') node_names[i][j] = buffer[j++];
      node_names[i][j] = 0x0; 
      i++;
      
      if(i >= NUM_NODES)
      {
        fprintf(stderr, "output_order: NUM_NODES = %s exceeded, exiting...\n", NUM_NODES);
        exit(1);
      }
    }
    line_num++;
  }
  rewind(infile);
  
  if(nodes_exist)
  {
    /* create space for contents of new graph file */
    graph = (char (*)[BUF_LEN]) malloc(line_num*BUF_LEN);
    line_num = 0;
  
    /* now parse for edges, removing from list nodes that are sinks */
    while(fgets(buffer, BUF_LEN, infile))
    {
      /* build the graph file in memory */
      i = 0;
      while(buffer[i]) graph[line_num][i] = buffer[i++];
      graph[line_num][i] = 0x0;
    
      if(p=strstr(buffer, "->")) /* an edge */
      {
        for(i=0; i<NUM_NODES; i++)
          if(node_names[i][0] && (q=strstr(p, node_names[i]))) /* at the node name of the sink */
          {
	    node_names[i][0] = 0x0; /* removal */
	    break;
	  }
      }
      line_num++;
    }
    fclose(infile);
  
    /* output the nodes with no parents, and remove from graph */
    outfile = fopen(newfile, "a");
    if(outfile == NULL)
    {
      fprintf(stderr, "output_order: error opening %s, exiting...\n", newfile); 
      exit(1);
    }
    for(i=0; i<NUM_NODES; i++)
      if(node_names[i][0])
      {
        fprintf(outfile, "%s\n", node_names[i]);
        for(j=0; j<line_num; j++)
          if(node_names[i][0] && (p=strstr(graph[j], node_names[i])))
            graph[j][0] = 0x0; /* removal */
      }
    fclose(outfile);
    
    outfile = fopen(graphfile, "w");
    if(outfile == NULL)
    {
      fprintf(stderr, "output_order: error opening %s, exiting...\n", graphfile); 
      exit(1);
    }
  
    for(i=0; i<line_num; i++)
      if(graph[i][0])
        fprintf(outfile, "%s", graph[i]);
    fclose(outfile);
    print_parentless_nodes(path);
  }
}
