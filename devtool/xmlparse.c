/* babirl/devtool/xmlparse
 * last modified : 11/09/26 11:04:15 
 * 
 * Test program for XML parsing
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <bbxml.h>

int main(int argc, char *argv[]){
  FILE *fd;
  char buff[1024*10];
  int idx, c;
  BBXMLEL *xml;

  if(argc != 2){
    printf("xmlparse XMLFILE\n");
    exit(0);
  }

  memset(buff, 0, sizeof(buff));

  if(!(fd = fopen(argv[1], "r"))){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  idx = 0;
  while((c = fgetc(fd)) != EOF){
    buff[idx] = c;
    idx ++;
    if(idx >= sizeof(buff)){
      printf("buffer size exeed > %ld\n", (long int)sizeof(buff));
      break;
    }
  }
  
  fclose(fd);

  xml = bbxml_parsebuff(buff, strlen(buff));
  if(!xml){
    printf("errror in file\n");
  }
  
  printf("getTextByTagName resetcount = %s\n",
	 bbxml_getTextByTagName(xml, "resetcount\0"));

  bbxml_free(xml);

  return 0;
}
