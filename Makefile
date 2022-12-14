# Copyright (C) 2022 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

ifndef GAP_SDK_HOME
	$(error Source sourceme in gap_sdk first)
endif

ifneq '$(TARGET_CHIP_FAMILY)' 'GAP9'
	$(error This Project is only for GAP9)
endif

include common.mk
include $(RULES_DIR)/at_common_decl.mk

io=stdout

ifeq '$(MODEL_TYPE)' 'tflite'
	ifeq ($(MODEL_SIZE), 0)
		IMAGE_SIZE=224
		IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_224.ppm
	endif
	ifeq ($(MODEL_SIZE), 1)
		IMAGE_SIZE=240
		IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_240.ppm
	endif
	ifeq ($(MODEL_SIZE), 2)
		IMAGE_SIZE=260
		IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_260.ppm
	endif
	ifeq ($(MODEL_SIZE), 3)
		IMAGE_SIZE=280
		IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_280.ppm
	endif
	ifeq ($(MODEL_SIZE), 4)
		IMAGE_SIZE=300
		IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_300.ppm
	endif
else
	IMAGE_SIZE=224
	IMAGE = $(CURDIR)/images/ILSVRC2012_val_00011158_224.ppm
endif
FLASH_TYPE ?= DEFAULT
RAM_TYPE   ?= DEFAULT

ifeq '$(FLASH_TYPE)' 'HYPER'
	MODEL_L3_FLASH=AT_MEM_L3_HFLASH
else ifeq '$(FLASH_TYPE)' 'MRAM'
	MODEL_L3_FLASH=AT_MEM_L3_MRAMFLASH
	READFS_FLASH = target/chip/soc/mram
else ifeq '$(FLASH_TYPE)' 'QSPI'
	MODEL_L3_FLASH=AT_MEM_L3_QSPIFLASH
	READFS_FLASH = target/board/devices/spiflash
else ifeq '$(FLASH_TYPE)' 'OSPI'
	MODEL_L3_FLASH=AT_MEM_L3_OSPIFLASH
	#READFS_FLASH = target/board/devices/ospiflash
else ifeq '$(FLASH_TYPE)' 'DEFAULT'
	MODEL_L3_FLASH=AT_MEM_L3_DEFAULTFLASH
endif

ifeq '$(RAM_TYPE)' 'HYPER'
	MODEL_L3_RAM=AT_MEM_L3_HRAM
else ifeq '$(RAM_TYPE)' 'QSPI'
	MODEL_L3_RAM=AT_MEM_L3_QSPIRAM
else ifeq '$(RAM_TYPE)' 'OSPI'
	MODEL_L3_RAM=AT_MEM_L3_OSPIRAM
else ifeq '$(RAM_TYPE)' 'DEFAULT'
	MODEL_L3_RAM=AT_MEM_L3_DEFAULTRAM
endif

USE_PRIVILEGED_FLASH?=0
ifeq ($(USE_PRIVILEGED_FLASH), 1)
MODEL_SEC_L3_FLASH=AT_MEM_L3_MRAMFLASH
else
MODEL_SEC_L3_FLASH=
endif

$(info Building NNTOOL model)
NNTOOL_EXTRA_FLAGS ?= 

include common/model_decl.mk

# pulpChip = GAP
# PULP_APP = $(MODEL_PREFIX)

APP = $(MODEL_PREFIX)
APP_SRCS += $(MODEL_PREFIX).c $(MODEL_GEN_C) $(MODEL_COMMON_SRCS) $(CNN_LIB)

APP_CFLAGS += -g -O3 -mno-memcpy -fno-tree-loop-distribute-patterns
APP_CFLAGS += -I. -I$(GAP_SDK_HOME)/utils/power_meas_utils -I$(MODEL_COMMON_INC) -I$(TILER_EMU_INC) -I$(TILER_INC) $(CNN_LIB_INCLUDE) -I$(MODEL_BUILD)
APP_CFLAGS += -DPERF -DAT_MODEL_PREFIX=$(MODEL_PREFIX) $(MODEL_SIZE_CFLAGS)
APP_CFLAGS += -DSTACK_SIZE=$(CLUSTER_STACK_SIZE) -DSLAVE_STACK_SIZE=$(CLUSTER_SLAVE_STACK_SIZE)
APP_CFLAGS += -DAT_IMAGE=$(IMAGE) -DTRANSPOSE_2CHW=$(TRANSPOSE_2CHW) -DIMAGE_SIZE=$(IMAGE_SIZE)
APP_CFLAGS += -DFREQ_FC=$(FREQ_FC) -DFREQ_CL=$(FREQ_CL) -DFREQ_PE=$(FREQ_PE) -DMODEL_NAME=$(TRAINED_MODEL)
ifneq '$(platform)' 'gvsoc'
ifdef GPIO_MEAS
APP_CFLAGS += -DGPIO_MEAS
endif
VOLTAGE?=800
ifeq '$(PMSIS_OS)' 'pulpos'
	APP_CFLAGS += -DVOLTAGE=$(VOLTAGE)
endif
endif

READFS_FILES=$(abspath $(MODEL_TENSORS))
ifneq ($(MODEL_SEC_L3_FLASH), )
	runner_args += --flash-property=$(CURDIR)/$(MODEL_SEC_TENSORS)@mram:readfs:files
endif

build:: model

clean:: clean_model

DATASET_PATH=
test_accuracy_nntool: $(MODEL_STATE)
	python models/test_accuracy_tflite.py $(MODEL_STATE) $(DATASET_PATH)

include common/model_rules.mk
$(info APP_SRCS... $(APP_SRCS))
$(info APP_CFLAGS... $(APP_CFLAGS))
include $(RULES_DIR)/pmsis_rules.mk

