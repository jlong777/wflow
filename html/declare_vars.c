/* declare_vars.c                            02/2005  jlong@alaska.edu
 *
 * ver 0.2
 *
 * declare, initialize, and set function params for a node, 
 * invoked once per node
 *
 * usage: 
 * declare_vars <working dir> <node_name> <wsdl> <service location>
 *
 * Copyright (C) 2005, University of Alaska Fairbanks
 * Biotechnology Computing Research Group
 * https://biotech.inbre.alaska.edu
 * jlong@alaska.edu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this 
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 *   Neither the name of the University of Alaska Fairbanks nor the names of its 
 *   contributors may be used to endorse or promote products derived from this 
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_LEN       1024
#define NUM_NODE_VARS   50
#define PARAM_LEN      256
#define NUM_USCR        50

char * source_name(char *, char *, char *);
char * find_root_source(char * wdir, char * source);

int main(int argc, char * argv[])
{
  int i, j, k, num_func_pieces;
  int type; /* type 1=char, 2=double, 3=float, 4=int, 5=long */
  char in_vars[NUM_NODE_VARS][PARAM_LEN], out_var[PARAM_LEN];
  char buffer[BUF_LEN], buffer2[BUF_LEN], var_buf[PARAM_LEN], *p, *q, *r;
  char func_name[PARAM_LEN], piece[NUM_USCR][PARAM_LEN], last_piece[PARAM_LEN]; 
  struct stat sbuf;
  FILE * callfile, * infile, * outfile, * tmpfile, * typefile;
  
  buffer[0] = 0x0;
  strcat(buffer, argv[1]); 
  strcat(buffer, "/gSOAP/callfile"); /* service function calls */
  callfile = fopen(buffer, "a");
  if(callfile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return;
  }
  
  buffer[0] = 0x0;
  strcat(buffer, argv[1]); 
  strcat(buffer, "/gSOAP/soapClient.c");
  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return;
  }
  
  buffer[0] = 0x0;
  strcat(buffer, argv[1]);
  strcat(buffer, "/gSOAP/client.c");
  outfile = fopen(buffer, "a");
  if(outfile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return;
  }
  
  buffer[0] = 0x0;
  strcat(buffer, argv[1]);
  strcat(buffer, "/gSOAP/tempfile"); /* parameter initializations */
  tmpfile = fopen(buffer, "a");
  if(tmpfile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return;
  }
  
  /* get function name, everything before the last _ */
  i = j = 0;
  buffer[0] = 0x0;
  strcat(buffer, argv[2]);
  while(buffer[i])
  {
    func_name[i] = buffer[i];
    if(buffer[i] == '_')
      j++;                     /* the number of _'s */
    i++;
  }
  func_name[i] = 0x0;
  i = 0;
  while(func_name[i])
  {
    if(func_name[i] == '_') j--;
    if(!j) break;
    i++;
  }
  func_name[i] = 0x0;
  
  /* now break func name into pieces separated by _ */
  i = j = k = 0;
  while(func_name[i])
  {
    piece[j][k] = func_name[i];
    if(func_name[i] == '_')
    {
      piece[j][k] = 0x0;
      i++; j++; k = 0;
      continue;
    }
    i++; k++;
  }
  piece[j][k] = 0x0;
  
  num_func_pieces = j+1;
  for(j=j+1; j<NUM_USCR; j++)
    piece[j][0] = 0x0;
  
  /* parse soapClient.c for pieces and "soap_call", declare vars */
parselabel:
  while(fgets(buffer, BUF_LEN, infile))
  {
    i = 0;
    while(buffer[i]) buffer2[i] = toupper(buffer[i++]);
    if(!(q=strstr(buffer, "soap_call_"))) continue;

    for(i=0; i<num_func_pieces; i++)
    {
      if(!strstr(buffer2, piece[i]))
        goto parselabel;
      /*printf("piece = %d  %s\n", i, piece[i]);*/
    }

    /* we made it this far, so we're at the function definition */
    
    fprintf(callfile, "\n  outfile = fopen(\"outfile\", \"a\");\n");
    fprintf(callfile, "  fprintf(outfile, \"Invoking %s... \\n\");\n", argv[2]);
    fprintf(callfile, "  fflush(NULL);\n");
    
    /* set the namespace table */
    fprintf(callfile, "  soap_set_namespaces(&soap, %s);\n", argv[3]);
    fprintf(callfile, "  ");
    
    i = 0;
    while(q[i] != '(') fprintf(callfile, "%c", q[i++]);
    fprintf(callfile, "(&soap, %s, \"\", \n", argv[4]);
    
    /* q is at the start of the param declarations */
    p = strstr(q, ","); p++; /* at end of 1st declaration, struct soap *soap */
    q = strstr(p, ","); q++; /* at end of 2nd declaration, const char *URL */
    p = strstr(q, ","); p++; /* at end of 3rd declaration, const char *action */

    q = p;
    i = j = 0;
    /* break up the declarations into pieces & output */
    while(strstr(p, ","))
    {
      i = j = 0; 
      while(*p == ' ') p++;

      while(*p != ',')
      {
        if(*p==' ')
        {
          piece[i][j] = 0x0;
          i++; j = 0;
	  while(*p == ' ') p++;
	  continue;
        }
        if(*p=='*')
        {
          piece[i][j++] = *p++;
	  piece[i][j] = 0x0;
	  i++; j = 0;
	  while(*p == ' ') p++;
	  continue;
        }
        piece[i][j++] = *p++;
      }
      piece[i][j] = 0x0;
    
      fprintf(outfile, "  ");
      for(k=0; k<i; k++)
        fprintf(outfile, "%s ", piece[k]);
      /* remove "USCORE" string if present */
      j = k = 0;
      while(piece[i][j])
      {
        last_piece[k++] = piece[i][j];
        if(piece[i][j] == '_')
	  j += 7;
	else
	  j++;
      }
      last_piece[k] = 0x0;
      fprintf(outfile, "%s_%s;\n", argv[2], last_piece);
    
      /* set the variable in one of three ways:
       * 1) the variable has a value set in a file, or
       * 2) the variable is the sink of an edge, or
       * 3)  if 1) or 2) don't apply, set to NULL
       */
      
      /* 1) does a file exist containing the value? */
      buffer[0] = 0x0;
      strcat(buffer, "../DATA/");
      strcat(buffer, argv[2]);
      strcat(buffer, "/");
      strcat(buffer, last_piece);
      if(stat(buffer, &sbuf) == 0) /* file exists */
      {
        type = 0;
        for(k=0; k<i; k++)
        {
          if(strstr(piece[k], "char"))        type = 1; /* assume char * for now */
          else if(strstr(piece[k], "double")) type = 2;
          else if(strstr(piece[k], "float"))  type = 3;
          else if(strstr(piece[k], "int"))    type = 4;
          else if(strstr(piece[k], "boolean"))type = 5;
	  else if(strstr(piece[k], "long"))   type = 6;
        }
        fprintf(callfile, "    %s_%s,\n", argv[2], last_piece);
        fprintf(tmpfile,    "  %s_%s = ", argv[2], last_piece);
        switch(type)
        {
          case 1: fprintf(tmpfile, "get_data(\"%s\", \"%s\");\n", argv[2], last_piece);
                  break;
          case 2:
          case 3: fprintf(tmpfile, "atof(get_data(\"%s\", \"%s\"));\n", argv[2], last_piece);
                  break;
          case 4:
	  case 5: fprintf(tmpfile, "atoi(get_data(\"%s\", \"%s\"));\n", argv[2], last_piece);
                  break;
          case 6: fprintf(tmpfile, "atol(get_data(\"%s\", \"%s\"));\n", argv[2], last_piece);
                  break;
          default: fprintf(stderr, "declare_vars: error, no type detected, assuming string\n");
                   break;
        }
      }
      else /* file does not exist */
      {
        /* is the param the sink of an edge? */
	if(q=source_name(argv[1], argv[2], last_piece))
	{
	  /* if q is an output struct, append name of struct output member */
	  if(r=strstr(q, "_out"))
	  {
	    if(!r[4])
	    {
	      buffer[0] = 0x0;
              strcat(buffer, argv[1]);
              strcat(buffer, "/OUTTYPES/");
              strcat(buffer, q);
	      k = strlen(buffer);
	      buffer[k-4] = 0x0;
              typefile = fopen(buffer, "r");
              if(typefile == NULL)
              {
                fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
                fflush(NULL);
              }
              fgets(buffer, BUF_LEN, typefile); /* output struct member type */
	      fgets(buffer, BUF_LEN, typefile); /* output struct member name */
              fclose(typefile);
              k = 0;
              while(buffer[k] != '\n') k++;
              buffer[k] = 0x0;
	      fprintf(callfile, "    %s.%s,\n", q, buffer);
            }
	    else
	      fprintf(callfile, "    %s,\n", q);
	  }
	  else
	    fprintf(callfile, "    %s,\n", q);
	}
	else
	  fprintf(callfile, "    %s,\n", "NULL");
      }
      p++; /* go to next parameter */
    }
    p++;
    /* p is positioned at the last parameter, the output struct */
    i = j = 0; 
    while(*p == ' ') p++;
    while(*p != ')')
    {
      if(*p==' ')
      {
        piece[i][j] = 0x0;
        i++; j = 0;
	while(*p == ' ') p++;
	continue;
      }
      if(*p=='*') /* assumes the pointer is the last piece */
      {
        piece[i][j++] = *p++;
	piece[i][j] = 0x0;
	break;
      }
      piece[i][j++] = *p++;
    }
    piece[i][j] = 0x0;
    
    fprintf(outfile, "  ");
    for(k=0; k<i; k++)
      fprintf(outfile, "%s ", piece[k]);
    fprintf(outfile, "%s_out;\n", argv[2]);
    
    fprintf(callfile, "    &%s_out);\n", argv[2]);
      
    /* output type and name stored in OUTTYPES/node_name */
    buffer[0] = 0x0;
    strcat(buffer, argv[1]);
    strcat(buffer, "/OUTTYPES/");
    strcat(buffer, argv[2]);
    typefile = fopen(buffer, "r");
    if(typefile == NULL)
    {
      fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
      fflush(NULL);
    }
    fgets(buffer, BUF_LEN, typefile); /* output struct member type */
    
    fprintf(callfile, "  fprintf(outfile, \"Output of %s invocation: \\n\");\n", argv[2]);
    fprintf(callfile, "  fflush(NULL);\n");
    fprintf(callfile, "  if(soap.error)\n");
    fprintf(callfile, "    soap_print_fault(&soap, outfile);\n");
    fprintf(callfile, "  else\n");
    
    type = 0;
    if(p=strstr(buffer, "xsd:string"))  type = 1;
    if(p=strstr(buffer, "xsd:double"))  type = 2;
    if(p=strstr(buffer, "xsd:float"))   type = 3;
    if(p=strstr(buffer, "xsd:integer")) type = 4;
    if(p=strstr(buffer, "xsd:long"))    type = 5;
    
    fgets(buffer, BUF_LEN, typefile); /* output struct member name */
    fclose(typefile);
    k = 0;
    while(buffer[k] != '\n') k++;
    buffer[k] = 0x0;
      
    switch(type) /* also print the output of the service invocation */
    {
      case 0: fprintf(callfile, "    fprintf(outfile, \"This node produces no output.\\n\\n\");\n");
              break;
      case 1: fprintf(callfile, "    fprintf(outfile, \"%%s\\n\\n\",  %s_out.%s);\n", argv[2], buffer);
              break;
      case 2: fprintf(callfile, "    fprintf(outfile, \"%%lf\\n\\n\", %s_out.%s);\n", argv[2], buffer);
              break;
      case 3: fprintf(callfile, "    fprintf(outfile, \"%%f\\n\\n\",  %s_out.%s);\n", argv[2], buffer);
              break;
      case 4: fprintf(callfile, "    fprintf(outfile, \"%%d\\n\\n\",  %s_out.%s);\n", argv[2], buffer);
              break;
      case 5: fprintf(callfile, "    fprintf(outfile, \"%%ld\\n\\n\", %s_out.%s);\n", argv[2], buffer);
              break;
      default: fprintf(stderr, "declare_vars: error, non-standard type %s\n", p);
              break;
    }
    fprintf(callfile, "  fflush(NULL);\n");
    fprintf(callfile, "  fclose(outfile);\n");
    break;
  }
}

/* if a param is a sink of an edge, return the name of the source
 * if the source itself is a sink, recursively return the source of the sink,
 * else return NULL
 */
char * source_name(char * wdir, char * node, char * param)
{
  int i, j;
  char buffer[BUF_LEN], source[PARAM_LEN], *p, *q, *ret_pt;
  FILE * infile;
  
  buffer[0] = 0x0;
  strcat(buffer, wdir); 
  strcat(buffer, "/graph.dot");
  
  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return NULL;
  }
  
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "->")) /* an edge */
    {
      if(q=strstr(p, node)) /* at the node name of the sink */
      {
        while(*q != ':') q++;
        q+=4; /* at the param name */
	if(strstr(q, param)) /* we are a sink, get the source name */
	{
	  i = j = 0;
	  while(buffer+i != p-1)
	  {
	    if(!strncmp(buffer+i, ":out", 4))
	    {
	      source[j++] = '_';
	      i++;
	    }
	    else if(!strncmp(buffer+i, ":in_", 4))
	    {
	      source[j++] = '_';
	      i += 4;
	    }
	    else
	      source[j++] = buffer[i++];
	  }
	  buffer[i] = source[j] = 0x0;

          /* find out if the source is itself a sink */
	  ret_pt = find_root_source(wdir, buffer);
	  if(ret_pt)
	    return ret_pt;
	  else
	  {
	    ret_pt = (char *) malloc(j+1);
            i = 0;
            while(source[i] != 0x0)
              ret_pt[i] = source[i++];
            ret_pt[i] = 0x0;
            return ret_pt;
	  }
	}
      }
    }
  }
  return (char *) 0x0; /* NULL */
}


char * find_root_source(char * wdir, char * source)
{
  /* recursively determine root source */
  int i, j;
  char buffer[BUF_LEN], root[PARAM_LEN], *p, *q, *ret_pt;
  FILE * infile;
  
  buffer[0] = 0x0;
  strcat(buffer, wdir); 
  strcat(buffer, "/graph.dot");
  
  infile = fopen(buffer, "r");
  if(infile == NULL)
  {
    fprintf(stderr, "declare_vars: file open error, %s\n", buffer); 
    fflush(NULL);
    return NULL;
  }
  
  while(fgets(buffer, BUF_LEN, infile))
  {
    if(p=strstr(buffer, "->")) /* an edge */
    {
      if(q=strstr(p, source)) /* our source is a sink */
      {
        i = j = 0;
	while(buffer+i != p-1)
	{
	  if(!strncmp(buffer+i, ":out", 4))
	  {
	    root[j++] = '_';
	    i++;
	  }
	  else if(!strncmp(buffer+i, ":in_", 4))
	  {
	    root[j++] = '_';
	    i += 4;
	  }
	  else
	    root[j++] = buffer[i++];
	}
	buffer[i] = root[j] = 0x0;
	ret_pt = find_root_source(wdir, buffer);
	if(ret_pt)
	    return ret_pt;
	else
	{
	  ret_pt = (char *) malloc(j+1);
          i = 0;
          while(root[i] != 0x0)
            ret_pt[i] = root[i++];
          ret_pt[i] = 0x0;
          return ret_pt;
	}
      }
    }
  }
  return (char *) 0x0; /* NULL */
}
