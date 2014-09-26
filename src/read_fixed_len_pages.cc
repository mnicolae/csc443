#include <stdio.h>

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
       printf("Usage: read_fixed_len_pages <page_file> <page_size>\n");
   }

   FILE * pageFile = fopen(argv[1], "r");

   if (pageFile)
   {
     

   return 0;
}

