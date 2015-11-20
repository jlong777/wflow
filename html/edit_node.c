/* edit_node.c                                  02/2005 jlong@alaska.edu
 *
 * ver 0.2
 *
 * generate html to allow a user to edit a node in a workflow graph
 *
 * usage edit_node <working dir> <node name>
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
#include <sys/stat.h>
#include <sys/types.h>

#define BUF_LEN       1024
#define NUM_NODES      100
#define NUM_NODE_VARS   50
#define PARAM_LEN      256

char * get_value(char *, char *, char *);

void remove_upstream_inputs(char *, char (*)[PARAM_LEN], char *,
			    int (*)[NUM_NODE_VARS]);

int main(int argc, char * argv[])
{
  int i, in_i=0, j, node_i=0, m, n;
  int edit_var[NUM_NODES][NUM_NODE_VARS];
  char in_vars[NUM_NODES][NUM_NODE_VARS][PARAM_LEN],
    node_names[NUM_NODES][PARAM_LEN]; /* these are static for now */
  char out_var[NUM_NODES][PARAM_LEN];
  char buffer[BUF_LEN], var_buf[PARAM_LEN], var_buf2[PARAM_LEN], *p, *q, *r;
  FILE * infile, * outfile;
  
  /* first parse for all node parameters */
  buffer[0] = 0x0;
  strcat(buffer, argv[1]); 
  strcat(buffer, "/graph.dot");
  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "file open error, %s\n", buffer); 
    fflush(NULL);
  }

  /* initialize */
  for(i=0; i<NUM_NODES; i++)
  {
    node_names[i][0] = 0x0;
    for(j=0; j<NUM_NODE_VARS; j++)
      edit_var[i][j] = 0;
  }
  /* parse */
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "label=")) /* at a node def */
    {
      /* record node name */
      i = j = 0;
      while(buffer[i] == ' ') i++; /* skip any leading whitespace */
      while(buffer[i] != ' ') node_names[node_i][j++] = buffer[i++];
      node_names[node_i][j] = 0x0;

      /* get params */
      while(q=strpbrk(p, "<"))
      {
        j = 0;
        if(*(q+1)=='i') /* an input param */
	{
	  q+=4;
	  while(*q != '>')
	  {
	    /* record input parameter name */
	    in_vars[node_i][in_i][j++] = *q;
            q++;
	  }
	  in_vars[node_i][in_i][j] = 0x0;
	  edit_var[node_i][in_i] = 3; /* 0=doesn't exist, 1=upstream, 2=sink, 3=can edit */
	  in_i++; p = q; continue;
	}
	if(*(q+1)=='o') /* an output param */
	{
	  q+=5;
	  while(*q != '"')
	  {
	    
	    /* record output parameter name */
	    out_var[node_i][j++] = *q;
            q++;
	  }
	  out_var[node_i][j] = 0x0;
	  break;
	}
      }
      in_i = 0; node_i++; 
      if(node_i >= NUM_NODES) fprintf(stderr, "NUM_NODES exceeded\n");
    }
  }
  rewind(infile);
  
  /* now parse for edges, removing input params that are already sinks */
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "->")) /* an edge */
    {
      for(i=0; i<NUM_NODES; i++)
        if(q=strstr(p, node_names[i])) /* at the node name of the sink */
	  break;

      while(*q != ':') q++;
      q++; /* at the field name */
      j = 0;
      while(*q != ';') var_buf[j++] = *q++; /* the param name of the sink */
      var_buf[j] = 0x0;

      /* delete the input param from being editable */
      for(j=0; j<NUM_NODE_VARS; j++)
	if(!strcmp(in_vars[i][j], var_buf+3))
	  /*in_vars[i][j][0] = 0x0;*/
	  edit_var[i][j] = 2;
    }
  }
  rewind(infile);
  
  /* also remove from consideration the inputs of any upstream nodes */
  buffer[0] = 0x0;
  strcat(buffer, argv[1]);
  strcat(buffer, "/graph.dot");
  remove_upstream_inputs(argv[2], node_names, buffer, edit_var);
  
  /* output the html */
  buffer[0] = 0x0;
  strcat(buffer, argv[1]);
  strcat(buffer, "/node.html");
  outfile = fopen(buffer, "w");
  if(outfile == NULL) {fprintf(stderr, "file open error, %s\n", buffer); fflush(NULL);}
  
  fprintf(outfile, "<h3>%s</h3>\n", argv[2]);
  
  for(i=0; i<NUM_NODES; i++)
    if(!strcmp(node_names[i], argv[2]))
      for(j=0; j<NUM_NODE_VARS; j++)
      {
	if(edit_var[i][j] == 3)
	{
	  if(p=get_value(argv[1], argv[2], in_vars[i][j]))
            fprintf(outfile, 
	          /*"<input type=\"text\" name=\"%s\" value=\"%s\" selected> <b>%s</b><br>\n",
                    in_vars[i][j], p, in_vars[i][j]);*/ 
                  "<b>%s</b><br><textarea name=\"%s\" rows=1 cols=60>%s</textarea><br>\n",
                    in_vars[i][j], in_vars[i][j], p);
          else
	    fprintf(outfile, 
	          /*"<input type=\"text\" name=\"%s\"> <b>%s</b><br>\n",*/ 
                  "<b>%s</b><br><textarea name=\"%s\" rows=1 cols=60></textarea><br>\n",
                    in_vars[i][j], in_vars[i][j]);
	}  
	/* send this input to another node param? */
	if(edit_var[i][j])
	{
	  fprintf(outfile, "<select name=\"%s_inedge\"><br>\n", in_vars[i][j]);
	  fprintf(outfile, "<option value=\"\"><br>\n");
	  for(m=0; m<NUM_NODES; m++)
            for(n=0; n<NUM_NODE_VARS; n++)
	      if(edit_var[m][n]>1 && strcmp(node_names[m], argv[2]))
                fprintf(outfile, "<option value=\"%s:in_%s\"> %s:%s<br>\n", 
	                node_names[m], in_vars[m][n], node_names[m], in_vars[m][n]);
          fprintf(outfile, "</select> Optional: Choose a node:param to receive <b>%s</b> input<br>\n", in_vars[i][j]);
	  fprintf(outfile, "<hr><br>\n");
	}
      }
	
	
  fprintf(outfile, "<br>\n<select name=\"wflow_input\"><br>\n");
  fprintf(outfile, "<option value=\"\"><br>\n");
  
  for(i=0; i<NUM_NODES; i++)
    if(!strcmp(node_names[i], argv[2]))
      break;

  if(strcmp(out_var[i], "NO_OUTPUT"))
    for(i=0; i<NUM_NODES; i++)
      for(j=0; j<NUM_NODE_VARS; j++)
        if(edit_var[i][j]>1 && strcmp(node_names[i], argv[2]))
          fprintf(outfile, "<option value=\"%s:in_%s\"> %s:%s<br>\n", 
	          node_names[i], in_vars[i][j], node_names[i], in_vars[i][j]);

  fprintf(outfile, "</select> Optional: Choose a node:param to receive the output<br>\n");

  /* list of delete-able edges */
  
  fprintf(outfile, "<br>\n<select name=\"wflow_edge\"><br>\n");
  fprintf(outfile, "<option value=\"\"><br>\n");
  
  while(fgets(buffer, BUF_LEN, infile))
  {
    /* is our node the source of an edge? */
    if((p=strstr(buffer, "->")) && !(q=strstr(p, argv[2])) && (r=strstr(buffer, argv[2])))
    {
      i = j = 0;
      /* make a nice display string */
      while(buffer[i] != ';')
      {
        if((buffer[i]=='i') && (buffer[i+1]=='n') && (buffer[i+2]=='_')) i+=3;
	if((buffer[i]==':') && (buffer[i+1]=='o') && (buffer[i+2]=='u') && (buffer[i+3]=='t'))
	{
	  i+=4;
          var_buf2[j++] = ':';
	  for(m=0; m<NUM_NODES; m++)
	    if(!strcmp(node_names[m], argv[2]))
	      break;
	  n = 0;
	  while(out_var[m][n] != 0x0) var_buf2[j++] = out_var[m][n++];
	}
        var_buf2[j++] = buffer[i]; /* the display string */
	i++;
      }
      var_buf2[j] = 0x0;
      
      i = 0;
      while(*p != ' ') p++;
      p++; /* at the sink name */
      while(*p != ';') var_buf[i++] = *p++;
      var_buf[i] = 0x0;
      /*fprintf(outfile, "<option value=\"%s\"> %s<br>\n", var_buf, var_buf2);*/
      fprintf(outfile, "<option value=\"%s\"> %s<br>\n", buffer, var_buf2);
    }
  }
  fprintf(outfile, "</select> Optional: Choose an edge to delete<br><br>\n");
  fprintf(outfile, "<input type=\"checkbox\" name=\"wflow_delete\"> <b>Delete this node</b><br>\n");
  fclose(infile);
  fclose(outfile);
  
  return 0;
}

void remove_upstream_inputs(char * node_name, char (*node_names)[PARAM_LEN],
                            char * graph_file,
			    int (*edit_var)[NUM_NODE_VARS])
{
  int i, j;
  char buffer[BUF_LEN], parent_name[PARAM_LEN], *p, *q;
  FILE * infile;

  infile = fopen(graph_file, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "file open error, %s\n", graph_file); 
    fflush(NULL);
  }
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "->")) /* an edge */
    {
      if((q=strstr(p, node_name))) /* node_name has a parent here */
      {
        i = 0;
        while(buffer[i] != ':') parent_name[i++] = buffer[i];
	parent_name[i] = 0x0;
	/* remove parent's inputs from consideration */
	for(i=0; i<NUM_NODES; i++) 
          for(j=0; j<NUM_NODE_VARS; j++)
            if(!strcmp(node_names[i], parent_name) && edit_var[i][j])
	      /*in_vars[i][j][0] = 0x0;*/
	      edit_var[i][j] = 1;
        /* recursively remove the other upstream parents */
	remove_upstream_inputs(parent_name, node_names, graph_file, edit_var);
      }
    }
  }
  fclose(infile);
  return;
}

char * get_value(char * wdir, char * node, char * param)
{
  char buffer[1024], * ret_pt;
  long size;
  struct stat sbuf;
  FILE * infile;  
  
  buffer[0] = 0x0;
  strcat(buffer, wdir);
  strcat(buffer, "/DATA/");
  strcat(buffer, node);
  strcat(buffer, "/");
  strcat(buffer, param);
  if(stat(buffer, &sbuf) == -1)
  {
/*    fprintf(stderr, "edit_node: warning, no file stats for %s, probably not set yet, returning NULL...\n", buffer);
    fflush(NULL); */
    return NULL;
  }

  ret_pt = (char *) malloc(sbuf.st_size + 1);

  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "edit_node: file open error, %s\n", buffer);
    fflush(NULL);
    return NULL;
  }

  size = fread(ret_pt, 1, sbuf.st_size, infile);
  if(size != sbuf.st_size)
  {
    fprintf(stderr, "edit_node: error reading file %s, returning...\n", buffer);
    fflush(NULL);
    return NULL;
  }
  fclose(infile);
  
  return ret_pt;
}












































