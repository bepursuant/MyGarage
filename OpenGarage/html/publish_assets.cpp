// takes an html file input and viewname and creates a
// file with a PROGMEM statement that contains the
// original html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
  // check if we were provided enough arguments, if not gripe
  if(argc<3) {
    printf("Usage: %s inputHtmlFile.html viewname\n", argv[0]);
    return 0;
  }

  // input and output filenames (relative)
  char iname[] = argv[1];
  char oname[] = "../www_assets.h";
  char vname[] = argv[2];

  // open a pointer to the input file
  FILE *fp = fopen(iname, "r");
  // and if we can't, toss a wobbly
  if(!fp) {
    printf("Can't open input file %s\n", iname);
    return 0;
  }

  // open a pointer to the output file
  FILE *op = fopen(oname, "a");
  // and if we can't, toss a different wobbly
  if(!op) {
    printf("Can't open output file %s\n", oname);
    return 0;
  }

  //char in[1000];
  //char out[1000];
  //int size;
  char c;
  int i;

  // start by writing our 'header' for this view to the file
  fprintf(op, "const char WWW_ASSETS[%s] PROGMEM = R\"(", vname);

  //bool in_body = false;

  // loop through the file until the end
  while(!feof(fp)) {

    // read 1000 bytes from the file into the in var
  	fgets(in, sizeof(in), fp);

  	size = strlen(in);
  	//c = in[size-1];

    // if this 1000 bytes starts with <body, dump out
    // of this loop, otherwise flag in_body??? huh
  	//if(!in_body) {
  	//  if(strncmp(in, "<body", 5)){
    //    continue;
    //  } else {
  	//    in_body = true;
  	//  }
  	//}

    // if the last character of the 1000 bytes from the file
    // is a return or newline, consider it worthless. not sure
    // this is necessary?
  	//if(c=='\r' || c=='\n') size--;
  	//c = in[size-1];
  	//if(c=='\r' || c=='\n') size--;

  	//char *outp = out;
    //bool isEmpty = true;

    //write the input file without newlines or returns
  	for(i=0; i<size; i++) {
  	  switch(in[i]) {
    	  //case ' ':
    	  //case '\t':
    	  case '\r':
    	  case '\n':
    	  case '\0':
    	  //  if(!isEmpty)
    	  //    *outp++ = in[i];
    	    break;
    		default:
    		  fprint(op, in[i]);
    		  //isEmpty = false;
    		  break;
    	} 
  	}
  }

  // write the 'footer' for this view
  fprint(op, ")\";\r\n0");
  fclose(op);

  // close the input file
  fclose(fp);
}