STM_PATH=../../stm/
include parallelDefs

# required files
GLOBAL_REQUIRE = utils.h parallel.h sequence.h graph.h
OBJS = ST.o ../../stm/txn.o ../../stm/txn2phase.o

include ../common/timeRequiredFiles
include MakeBench
include $(STM_PATH)/MakeSTM
