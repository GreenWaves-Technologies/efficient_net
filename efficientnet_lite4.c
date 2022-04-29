
/*
 * Copyright (C) 2017 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */


/* Autotiler includes. */
#include "efficientnet_lite4.h"
#include "efficientnet_lite4Kernels.h"
#include "gaplib/ImgIO.h"

#ifdef __EMUL__
#define pmsis_exit(n) exit(n)
#endif

#define __XSTR(__s) __STR(__s)
#define __STR(__s) #__s
#ifndef STACK_SIZE
#define STACK_SIZE      1024
#endif

AT_HYPERFLASH_FS_EXT_ADDR_TYPE efficientnet_lite4_L3_Flash = 0;

char *ImageName;
/* Inputs */
// L2_MEM signed char Input_1[150528];
/* Outputs */
L2_MEM short int Output_1[1000];
int outclass = 0, MaxPrediction = 0;

static void cluster()
{
    #ifdef PERF
    printf("Start timer\n");
    gap_cl_starttimer();
    gap_cl_resethwtimer();
    #endif

    GPIO_HIGH();
    efficientnet_lite4CNN(Output_1);
    GPIO_LOW();
    printf("Runner completed\n");

    //Check Results
    outclass = 0, MaxPrediction = 0;
    for(int i=0; i<1000; i++){
        if (Output_1[i] > MaxPrediction){
            outclass = i;
            MaxPrediction = Output_1[i];
        }
    }
    printf("Predicted class:\t%d\n", outclass);
    printf("With confidence:\t%d\n", MaxPrediction);
}

int test_efficientnet_lite4(void)
{
    printf("Entering main controller\n");
    /* ---------------->
     * Put here Your input settings
     * <---------------
     */


#ifndef __EMUL__
    OPEN_GPIO_MEAS();
    /* Configure And open cluster. */
    struct pi_device cluster_dev;
    struct pi_cluster_conf cl_conf;
    pi_cluster_conf_init(&cl_conf);
    cl_conf.id = 0;
    cl_conf.cc_stack_size = STACK_SIZE;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-4);
    }
    int cur_fc_freq = pi_freq_set(PI_FREQ_DOMAIN_FC, 250000000);
    if (cur_fc_freq == -1)
    {
        printf("Error changing frequency !\nTest failed...\n");
        pmsis_exit(-4);
    }

    int cur_cl_freq = pi_freq_set(PI_FREQ_DOMAIN_CL, 175000000);
    if (cur_cl_freq == -1)
    {
        printf("Error changing frequency !\nTest failed...\n");
        pmsis_exit(-5);
    }
#ifdef __GAP9__
    pi_freq_set(PI_FREQ_DOMAIN_PERIPH, 250000000);
#endif
#endif
    // IMPORTANT - MUST BE CALLED AFTER THE CLUSTER IS SWITCHED ON!!!!
    printf("Constructor\n");
    int ConstructorErr = efficientnet_lite4CNN_Construct();
    if (ConstructorErr)
    {
        printf("Graph constructor exited with error: %d\n(check the generated file efficientnet_lite4Kernels.c to see which memory have failed to be allocated)\n", ConstructorErr);
        pmsis_exit(-6);
    }

    printf("Reading image from %s\n",ImageName);

    //Reading Image from Bridge
    img_io_out_t type = IMGIO_OUTPUT_CHAR;
    if (ReadImageFromFile(ImageName, 224, 224, 3, Input_1, 224*224*3*sizeof(char), type, TRANSPOSE_2CHW)) {
        printf("Failed to load image %s\n", ImageName);
        pmsis_exit(-1);
    }
    printf("Finished reading image %s\n", ImageName);
    for (int i=0; i<224*224*3; i++) Input_1[i] -= 128;

    printf("Call cluster\n");
#ifndef __EMUL__
    struct pi_cluster_task task;
    pi_cluster_task(&task, cluster, NULL);
    pi_cluster_task_stacks(&task, NULL, SLAVE_STACK_SIZE);
    pi_cluster_send_task_to_cl(&cluster_dev, &task);
#else
    cluster();
#endif

    efficientnet_lite4CNN_Destruct();

#ifdef PERF
    {
      unsigned int TotalCycles = 0, TotalOper = 0;
      printf("\n");
      for (unsigned int i=0; i<(sizeof(AT_GraphPerf)/sizeof(unsigned int)); i++) {
        TotalCycles += AT_GraphPerf[i]; TotalOper += AT_GraphOperInfosNames[i];
      }
      for (unsigned int i=0; i<(sizeof(AT_GraphPerf)/sizeof(unsigned int)); i++) {
        printf("%45s: Cycles: %10u (%%: %5.2f%%), Operations: %10u (%%: %5.2f%%), Operations/Cycle: %f\n", AT_GraphNodeNames[i], AT_GraphPerf[i], 100*((float) (AT_GraphPerf[i]) / TotalCycles), AT_GraphOperInfosNames[i], 100*((float) (AT_GraphOperInfosNames[i]) / TotalOper), ((float) AT_GraphOperInfosNames[i])/ AT_GraphPerf[i]);
      }
      printf("\n");
      printf("%45s: Cycles: %10u (%%:100.00%%), Operations: %10u (%%:100.00%%), Operations/Cycle: %f\n", "Total", TotalCycles, TotalOper, ((float) TotalOper)/ TotalCycles);
      printf("\n");
    }
#endif

    if(outclass==42 && MaxPrediction>15000) printf("Test successful!\n");
    else {
        printf("Wrong results!\n");
        pmsis_exit(-1);
    }

    printf("Ended\n");
    pmsis_exit(0);
    return 0;
}

int main(int argc, char *argv[])
{
    printf("\n\n\t *** NNTOOL efficientnet_lite4 Example ***\n\n");
    #ifdef __EMUL__
    if (argc < 2)
    {
      PRINTF("Usage: squeezenet [image_file]\n");
      exit(-1);
    }
    ImageName = argv[1];
    test_efficientnet_lite4();
    #else
    ImageName = __XSTR(AT_IMAGE);
    return pmsis_kickoff((void *) test_efficientnet_lite4);
    #endif
    return 0;
}
