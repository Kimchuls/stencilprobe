#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../SZ/sz/include/sz.h"
#include "../SZ/sz/include/rw.h"

#define MAX_LINE_LENGTH 256

typedef struct Fault_Injector_ConfigsSt
{
    
    // inject faults in which position.
    int x;
    int y;
    int z;

    // inject faults in which iterations.
    //     In CoMD, the injected iteration should
    //     be divisible by 10!!!!!!!
    int iters; 
    
    // Descriptions about lossy compressor:
    //     compressor = 1 -> SZ
    //     compressor = 2 -> ZFP
    //     compressor = 3 -> ?
    int compressor; // select a lossy compressor method.

    // Descriptions about error_type:
    //     error_type = "ABS" -> error type is absolute error bound
    //     error_type = "REL" -> error type is relative error bound
    char error_type[10]; // error type of lossy compressor

    // Descriptions about error_bound, there are two formats:
    //     format 1: error_bound = 1E-2
    //     format 2: error_bound = 0.01
    double error_bound; // error bound of lossy compressor, where error_bound should be related to its error_type

    // Char version of error_bound, only used for overwrite_sz_config().
    char error_bound_ch[32];

    // More details about error type and error bound could be found at sz.config
} Fault_Injector_Configs;

static double mytimer(void);
static double *SZ_fault_generator_1D(int length, double *ori_data);
static Fault_Injector_Configs read_fault_injector_config();
static void overwrite_sz_config(Fault_Injector_Configs config);

/** ************************************************************************
 * @brief Record current timestamp. So that SZ lossy compression time can be 
 * recorded via mytimer() - mytimer() (i.e. timestamp1-timestamp2).
 *
 * @return Current timestamp, double type.
 ************************************************************************ */
double mytimer(void)
{
   struct rusage ruse;
   getrusage(RUSAGE_SELF, &ruse);
   return( (double)(ruse.ru_utime.tv_sec+ruse.ru_utime.tv_usec / 1000000.0) );
}


/** ************************************************************************
 * @brief	Compress double type vector data by SZ lossy compression.
 * 
 * @param length	target vector data length.
 * @param ori_data	target vector data to be compressed.
 *
 * @return	Compressed double type vector data.
 ************************************************************************ */
double * SZ_fault_generator_1D(int length, double * ori_data)
{
    const char *SZcfgFilePath;
    double start_time1 = 0.0,
           start_time2 = 0.0,
           compression_time   = 0.0,
           decompression_time = 0.0;

    SZcfgFilePath = "./exconfig/sz.config";
    int status = SZ_Init(SZcfgFilePath);

    if(status == SZ_NSCS)
    {
        printf("Can not read sz.config file!\n");
        exit(0);
    }

    double *tar_data = (double*)malloc(length*sizeof(double));
    size_t SZ_compressed_size;
    size_t r1 = length, r2 = 0, r3 = 0, r4 = 0, r5 = 0;

    // SZ lossy compress data, and record time.
    start_time1=mytimer(); 
	unsigned char *bytes = SZ_compress(SZ_DOUBLE, ori_data, &SZ_compressed_size, r5, r4, r3, r2, r1); 
	compression_time+=mytimer()-start_time1;
    
    // SZ lossy decompress data, and record time.
    start_time2=mytimer(); 
	size_t SZ_decompressed_size = SZ_decompress_args(SZ_DOUBLE, bytes, SZ_compressed_size, tar_data, r5, r4, r3, r2, r1); 
	decompression_time+=mytimer()-start_time2;
	
	SZ_Finalize();

	return tar_data;
}


/** ************************************************************************
 * @brief   Retrieve all information in fault_injector.config.
 * 
 * @return	A Fault_Injector_Configs type var contains all useful information.
 ************************************************************************ */
Fault_Injector_Configs read_fault_injector_config()
{
    Fault_Injector_Configs config;
    
    const char *FIcfgFilePath;
    FIcfgFilePath = "./fault_injector.config";
    
    FILE *fp = fopen(FIcfgFilePath, "r");
    if(fp == NULL)
    {
        printf("No fault_injector.config found!\n");
        exit(1);
    }

    char chunk[MAX_LINE_LENGTH];

    // Read fault_injector.config line by line.
    while(fgets(chunk, MAX_LINE_LENGTH, fp) != NULL)
    {
        if(chunk[0] != '\n' && chunk[0] != '#')
        {
            // Trim chunk.
            int count_non_space_length = 0;
            for(int i=0; i<strlen(chunk); i++)
            {
                if(chunk[i] != ' ' && chunk[i] != '\n')
                    count_non_space_length++;
            }
            char *temp1 = (char*)malloc(MAX_LINE_LENGTH*sizeof(char));
            int index = 0;
            for(int i=0; i<strlen(chunk); i++)
            {
                if(chunk[i] == ' ' || chunk[i] == '\n')
                    continue;
                temp1[index] = chunk[i];
                index++;
            }
            temp1[index] = '\0';    // why it is out of bound but still can be accessed????? 
                                    // Ok I see. I forgot '\0' in C, sorry~
            

	        // Get config.sim_atoms_r from given .config file.
            if(strstr(temp1, "x="))
            {
	            char *temp2 = strstr(temp1, "=");
		        temp2++;
		        config.x = atoi(temp2);
            }

	        // Get config.sim_atoms_p from given .config file.
	        if(strstr(temp1, "y="))
	        {
		        char *temp2 = strstr(temp1, "=");
		        temp2++;
		        config.y = atoi(temp2);
	        }

	        // Get config.sim_atoms_f from given .config file.
	        if(strstr(temp1, "z="))
	        {
		        char *temp2 = strstr(temp1, "=");
		        temp2++;
		        config.z = atoi(temp2);
	        }

            // Get config.iters from given .config file.
            if(strstr(temp1, "injected_iteration="))
            {
                char *temp2 = strstr(temp1, "=");
                temp2++;
                config.iters = atoi(temp2);
            }

            // Get config.compressor from given .config file.
            if(strstr(temp1, "compressor="))
            {
                char *temp2 = strstr(temp1, "=");
                temp2++;
                config.compressor = atoi(temp2);
            }

            // Get config.error_type from given .config file.
            if(strstr(temp1, "SZerrorBoundMode="))
            {
                char *temp2 = strstr(temp1, "=");
                temp2++;
                strcpy(config.error_type, temp2);
            }
            
            // Get config.error_bound from given .config file (if it is ABS).
            if(strstr(config.error_type, "ABS") && strstr(temp1, "SZabsErrorBound="))
            {
                char *temp2 = strstr(temp1, "=");
                temp2++;
                config.error_bound = atof(temp2);
                strcpy(config.error_bound_ch, temp2);
            }

            // Get config.error_bound from given .config file (if it is REL).
            if(strstr(config.error_type, "REL") && strstr(temp1, "SZrelBoundRatio="))
            {
                char *temp2 = strstr(temp1, "=");
                temp2++;
                config.error_bound = atof(temp2);
                strcpy(config.error_bound_ch, temp2);
            }
        
            free(temp1);
        }
    }
    
    fclose(fp);

    return config;
}


/** ************************************************************************
 * @brief   Modify sz.config by given config (Fault_Injector_Configs)
 * 
 * @param   config containing all useful information by function 
 *                 read_fault_injector_config()
 ************************************************************************ */
void overwrite_sz_config(Fault_Injector_Configs config)
{
    FILE *fPtr;
    FILE *fTemp;
    const char *SZFIcfgFilePath;
    SZFIcfgFilePath = "./exconfig/sz.config";
    char buffer[MAX_LINE_LENGTH];

    // Open all required files
    fPtr  = fopen(SZFIcfgFilePath, "r");
    fTemp = fopen("./exconfig/replace.tmp", "w");

    // Check if target files exist.
    if(fPtr == NULL || fTemp == NULL)
    {
        printf("Unable t open file.\n");
        exit(1);
    }

    // Overwrite information from config to sz.config
    while(fgets(buffer, MAX_LINE_LENGTH, fPtr) != NULL)
    {
        // Overwrite errorBoundMode
        if(strstr(buffer, "errorBoundMode = ") && buffer[0] != '#')
        {
            if(strstr(config.error_type, "ABS"))
            {   
                fputs("errorBoundMode = ABS\n", fTemp);
            }
            else if(strstr(config.error_type, "REL"))
            {
                fputs("errorBoundMode = REL\n", fTemp);
            }
        }
        // Overwrite absErrBound when config.error_type == "ABS"
        else if(strstr(buffer, "absErrBound = ") && buffer[0] != '#')
        {
            if(strstr(config.error_type, "ABS"))
            {
                char temp_buffer[40] = "absErrBound = ";
                strcat(temp_buffer, config.error_bound_ch);
                temp_buffer[strlen(temp_buffer)] = '\n';
                fputs(temp_buffer, fTemp);
            }
            else
            {
                fputs(buffer, fTemp);
            } 
        }
        // Overwrite relBoundRatio when config.error_type == "REL"
        else if(strstr(buffer, "relBoundRatio = ") && buffer[0] != '#' && buffer[0] != 'p')
        {
            if(strstr(config.error_type, "REL"))
            {
                char temp_buffer[40] = "relBoundRatio = ";
                strcat(temp_buffer, config.error_bound_ch);
                temp_buffer[strlen(temp_buffer)] = '\n';
                fputs(temp_buffer, fTemp);
            }
            else
            {
                fputs(buffer, fTemp);
            } 
        }
        else
        {
            fputs(buffer, fTemp);
        }
    }

    fclose(fPtr);
    fclose(fTemp);

    // Delete original source file.
    remove(SZFIcfgFilePath);

    // Rename temorary file as new file.
    rename("./exconfig/replace.tmp", SZFIcfgFilePath);
}
