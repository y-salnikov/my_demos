#include <stdio.h>
#include <bzlib.h>
#include <string.h>
#include <stdlib.h>

FILE *input_file,*output_file;
char *src_buf, *dst_buf;
long int input_file_length;
unsigned int compressed_length;

const int col=12;

void usage(void)
{
printf("(c)Jarik\n");
printf("Usage: bin2c [-c] input_file output_file\n");
printf("-c - compress data with bzip2\n");
}

void convert_files(void)
{
    long int i=0;
    int tmp;
    fprintf(output_file,"/* Created by bin2c (c)Jarik */\n");
    fprintf(output_file,"char data[]={ \n");    
    while(1)
    {
    tmp=fgetc(input_file);
    if(tmp!=EOF)
    {
	fprintf(output_file,"%d,",(tmp & 0xFF));
	if((i%col)==col-1) fprintf(output_file,"\n");
	i++;
    } else
    {
    fprintf(output_file,"0};\n");    
    fprintf(output_file,"long int data_length=%d;\n",i);        
    return;
    }
    }
}

void get_length(void)
{
    fseek(input_file,0,SEEK_END);
    input_file_length=ftell(input_file);
    fseek(input_file,0,SEEK_SET);
}

void init_buffers(void)
{
    src_buf=malloc(input_file_length);
    compressed_length=(1.01*(double)input_file_length)+600;
    dst_buf=malloc(compressed_length);
}

void free_buffers(void)
{
    free(src_buf);
    free(dst_buf);
}

void open_files(char **argv)
{
    input_file=fopen(argv[2],"rb");
    output_file=fopen(argv[3],"w");
}



void read_file_to_buffer(void)
{
    fread(src_buf,1,input_file_length,input_file);
}


void write_file_from_buff(void)
{
    long int i=0;
    int tmp;
    fprintf(output_file,"/* Created by bin2c (c)Jarik */\n");
    fprintf(output_file,"char compressed_data[]={ \n");    
    for(i=0;i<compressed_length;i++)
    {
        tmp=dst_buf[i];
        fprintf(output_file,"%d,",(tmp & 0xFF));
	    if((i%col)==col-1) fprintf(output_file,"\n");
    }
    fprintf(output_file,"0};\n");    
    fprintf(output_file,"long int compressed_data_length=%d;\n",compressed_length);
    fprintf(output_file,"long int data_length=%d;\n",input_file_length);        
    return;

    
}


void compress_file(char **args)
{
    open_files(args);
    get_length();
    init_buffers();
    read_file_to_buffer();
    if(BZ2_bzBuffToBuffCompress(dst_buf,&compressed_length,src_buf,input_file_length,9,0,0)==BZ_OK)
    {
        printf("compressed\n");
    }else
    {
        printf("compression error\n");
    }
       
    write_file_from_buff();
    free_buffers();
}


main (int argc, char *argv[])
{
    if(argc<3) 
    {
     usage();
     return 0;
    }
    if(strcmp(argv[1],"-c"))
    {
        input_file=fopen(argv[1],"rb");
        output_file=fopen(argv[2],"w");
        convert_files();
        fclose(input_file);
        fclose(output_file);
    } else
    compress_file(argv);    
    return 0;
}
