#***************************************************************************************
# Copyright (c) 2020-2022 Institute of Computing Technology, Chinese Academy of Sciences
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

NAME = gcpt

WORK_DIR = $(CURDIR)

ifdef O
	BUILD_DIR = $(shell readlink -f $(O))/build
else
	BUILD_DIR = $(WORK_DIR)/build
endif

OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

.DEFAULT_GOAL = app

INC_DIR += include resource/nanopb

# Compilation flags
CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS += -fno-PIE -mcmodel=medany -O2 -MMD -Wall -Werror $(INCLUDES)
CFLAGS += -nostdlib -fno-common -ffreestanding

LDFLAGS =

ifdef GCPT_PAYLOAD_PATH
LDFLAGS += -Xlinker --defsym GCPT_PAYLOAD_PATH=1
CFLAGS += -DGCPT_PAYLOAD_PATH=\"$(GCPT_PAYLOAD_PATH)\"
else
GCPT_PAYLOAD_PATH =
endif

ifdef GCPT_PAYLOAD_POSITION
LDFLAGS += -Xlinker --defsym GCPT_PAYLOAD_POSITION=$(GCPT_PAYLOAD_POSITION)
CFLAGS += -DGCPT_PAYLOAD_POSITION=$(GCPT_PAYLOAD_POSITION)
else
GCPT_PAYLOAD_OFFSET =
endif

ifdef DISPLAY_CPU_N
CFLAGS += -DDISPLAY=$(DISPLAY_CPU_N)
endif

ifdef STOP_CPU_N
CFLAGS += -DSTOP_CPU=$(STOP_CPU_N)
endif

ifdef USING_QEMU_DUAL_CORE_SYSTEM
CFLAGS += -DUSING_QEMU_DUAL_CORE_SYSTEM=$(USING_QEMU_DUAL_CORE_SYSTEM)
endif

ifdef USING_BARE_METAL_WORKLOAD
CFLAGS += -DUSING_BARE_METAL_WORKLOAD=$(USING_BARE_METAL_WORKLOAD)
endif

ifdef ENCODE_DECODE_CHECK
CFLAGS += -DENCODE_DECODE_CHECK
endif

#CSANITIZE := undefined
#
#CFLAGS += $(addprefix -fsanitize=, $(CSANITIZE))

# Files to be compiled
SRCS = $(shell find src/ -name "*.[cS]")
SRCS += resource/nanopb/pb_common.c
SRCS += resource/nanopb/pb_decode.c
SRCS += resource/nanopb/pb_encode.c
SRCS += $(patsubst %.proto,%.pb.c,$(shell find src/ -name "*.proto"))
OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(SRCS))))

.PRECIOUS: %.pb.c %.pb.h
# Protobuf
%.pb.c: %.proto
	@python resource/nanopb/generator/nanopb_generator.py --strip-path $<
	@mv $(basename $<).pb.h include/

# Compilation patterns
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) $(CFLAGS) -march=rv64gc -c -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(CC) $(CFLAGS) -march=rv64gcv -c -o $@ $<

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O0 -nostdlib -T restore.lds $(LDFLAGS) -o $@ $^
	@$(OBJDUMP) -S $@ --start-address=0x0 --stop-address=0x100000 > $@.txt &
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $@ $@.bin

app: $(BINARY)

.PHONY: clean
clean:
	@echo + RM ./build ./src/checkpoint.pb.c ./include/checkpoint.pb.h
	@-rm -rf $(shell find src/ -name "*.pb.[ch]")
	@-rm -rf $(BUILD_DIR)
