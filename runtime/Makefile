# Disable all implicit Makefile rules
MAKEFLAGS += --no-builtin-rules
.SUFFIXES: ;

CXX ?= g++
PROTOC ?= protoc

VERBOSE ?= 1

DEPDIR ?= .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$(@:.o=.d)
$(shell mkdir -p $(DEPDIR) >/dev/null)
$(shell mkdir -p $(DEPDIR)/bessport >/dev/null)
$(shell mkdir -p $(DEPDIR)/bessport/utils >/dev/null)
$(shell mkdir -p $(DEPDIR)/bessport/hooks >/dev/null)
$(shell mkdir -p $(DEPDIR)/samples > /dev/null)
$(shell mkdir -p $(DEPDIR)/rpc > /dev/null)
$(shell mkdir -p $(DEPDIR)/actor > /dev/null)

# e.g., 4.9.3 -> 40903
CXXVERSION := $(shell $(CXX) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')

CXXVERSION_4_8_OR_HIGHER := $(shell expr $(CXXVERSION) \>= 40800)
ifeq "$(CXXVERSION_4_8_OR_HIGHER)" "0"
  $(error g++ 4.8 or higher is required)
endif

RTE_SDK ?= $(abspath ../deps/bess/deps/dpdk-16.07)
RTE_TARGET ?= $(shell uname -m)-native-linuxapp-gcc
DPDK_LIB ?= dpdk

ifneq ($(wildcard $(RTE_SDK)/$(RTE_TARGET)/*),)
    DPDK_INC_DIR = $(RTE_SDK)/$(RTE_TARGET)/include
    DPDK_LIB_DIR = $(RTE_SDK)/$(RTE_TARGET)/lib
else ifneq ($(wildcard $(RTE_SDK)/build/*),)
    # if the user didn't do "make install" for DPDK
    DPDK_INC_DIR = $(RTE_SDK)/build/include
    DPDK_LIB_DIR = $(RTE_SDK)/build/lib
else
    ifeq ($(words $(MAKECMDGOALS)),1)
        ifneq ($(MAKECMDGOALS),clean)
            $(error DPDK is not available. \
                    Make sure $(abspath $(RTE_SDK)) is available and built)
        endif
    endif
endif

PROTOCXXFLAGS = -std=gnu++11 -O0

CXXFLAGS += -std=gnu++11 -g3 -ggdb3 -Ofast -march=native \
	    -isystem $(DPDK_INC_DIR) -D_GNU_SOURCE \
	    -Wall -Wextra -Wcast-align

# -Wshadow should not be used for g++ 4.x, as it has too many false positives
CXXVERSION_5_OR_HIGHER := $(shell expr $(CXXVERSION) \>= 50000)
ifeq "$(CXXVERSION_5_OR_HIGHER)" "1"
  CXXFLAGS += -Wshadow
endif

LDFLAGS += -rdynamic -L$(DPDK_LIB_DIR) -Wl,-rpath=$(DPDK_LIB_DIR) -pthread \
	   -static-libstdc++

LIBS += -Wl,-non_shared \
	-Wl,--whole-archive -l$(DPDK_LIB) -Wl,--no-whole-archive \
	-lglog -lgflags -lprotobuf -lgrpc++ -lgrpc \
	-lssl -lcrypto -lunwind -llzma -lpcap -lz \
	-Wl,-call_shared \
	-ldl

ifdef SANITIZE
    CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address -static-libasan
endif

ifdef COVERAGE
    # override -Ofast with -Og for better results
    CXXFLAGS += --coverage -Og
    LDFLAGS += --coverage
endif

LIBS += -libverbs

PROTO_DIR = bessport
PROTOS = $(wildcard $(PROTO_DIR)/*.proto)
PROTO_SRCS = $(patsubst %.proto,%.pb.cc, $(PROTOS))
PROTO_SRCS += $(patsubst %.proto,%.grpc.pb.cc, $(PROTOS))
PROTO_HEADERS = $(patsubst %.cc,%.h, $(PROTO_SRCS))
PROTOCFLAGS += --proto_path=$(PROTO_DIR) --proto_path=protobuf --cpp_out=$(PROTO_DIR) --grpc_out=$(PROTO_DIR) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`

ALL_SRCS = $(wildcard bessport/*.cc bessport/hooks/*.cc bessport/utils/*.cc samples/*.cc rpc/*.cc) gtest_main.cc actor/actor.cc actor/execution_context.cc actor/nf_ec_timer.cc
HEADERS = $(wildcard bessport/*.h bessport/hooks/*.h bessport/utils/*.h rpc/*.h) actor/actor.h actor/atom_definition.h actor/execution_context.h actor/nf_ec_timer.h actor/network_function.h actor/network_function_hub.h actor/flow_state.h actor/service_chain_state.h

TEST_SRCS = $(filter %_test.cc gtest_main.cc, $(ALL_SRCS))
TEST_OBJS = $(TEST_SRCS:.cc=.o)
TEST_ALL_EXEC = all_test

SRCS = $(filter-out $(TEST_SRCS), $(ALL_SRCS)) $(PROTO_SRCS)
OBJS = $(SRCS:.cc=.o)

GTEST_DIR = /usr/src/gtest

# The following variables are defined for sample examples.
PROCESS_PACKET_EXEC = process_packets
PROCESS_PACKET_OBJS = $(filter-out main.o samples/rpc_client.o rpc/nfa_rpc_server.o,$(OBJS))

RPC_TEST_CLIENT_EXEC = rpc_client
RPC_TEST_CLIENT_OBJS = $(filter-out main.o samples/process_packets.o rpc/nfa_rpc_server.o,$(OBJS))

RPC_TEST_SERVER_EXEC = nfa_rpc_server
RPC_TEST_SERVER_OBJS = $(filter-out main.o samples/%.o,$(OBJS))


.PHONY: all clean tests protobuf

all: $(OBJS) $(PROCESS_PACKET_EXEC) $(RPC_TEST_CLIENT_EXEC) $(RPC_TEST_SERVER_EXEC)

clean:
	rm -rf $(EXEC) .deps/*.d .deps/*/*.d *_test */*_test *_bench */*_bench \
		*.a *.pb.* *.o */*.o *.gcda *.gcno */*.gcda */*.gcno \
		coverage.info coverage_html */*.pb.cc */*.pb.h
		
tests: $(TEST_OBJS) $(TEST_ALL_EXEC)
		
protobuf: $(PROTO_SRCS)

# This build wrapper takes 4 parameters:
# $(1): build type (CXX, LD, ...)
# $(2): Make target
# $(3): Make prerequisites
# $(4): command
define BUILD
$(2): $(3)
	$$(eval _TYPE = $$(strip $(1)))
	$$(eval _CMD = $$(strip $(4)))
	@if [ $$(VERBOSE) -eq 0 ]; then \
		printf "%-11s %s\n" "[$$(_TYPE)]" "$$@"; \
	else \
		printf "%-11s %s\n" "[$$(_TYPE)]" "$$(_CMD)"; \
	fi
	@if ! $$(_CMD); then \
		echo "Error: \033[0;31m$$@"; \
		echo "\033[0;33m$$(_CMD)\033[0m"; \
		false; \
	fi
endef

$(eval $(call BUILD, \
	PROTOC, \
	%.pb.cc %.pb.h %.grpc.pb.cc %.grpc.pb.h, \
	%.proto, \
	$$(PROTOC) $$< $$(PROTOCFLAGS)))
	
$(eval $(call BUILD, \
	CXX, \
	%.pb.o, \
	%.pb.cc $(DEPDIR)/$$@.d, \
	$$(CXX) -o $$@ -c $$< $$(PROTOCXXFLAGS) $$(DEPFLAGS)))

$(eval $(call BUILD, \
	CXX, \
	%.o, \
	%.cc $(PROTO_HEADERS) $(DEPDIR)/$$@.d, \
	$$(CXX) -o $$@ -c $$< $$(CXXFLAGS) $$(DEPFLAGS)))
	
$(eval $(call BUILD, \
	TEST_CXX, \
	%_test.o, \
	%_test.cc $(DEPDIR)/$$@.d, \
	$$(CXX) -o $$@ -c $$< $$(CXXFLAGS) $$(DEPFLAGS)))

$(eval $(call BUILD, \
	TEST_LD, \
	$$(TEST_ALL_EXEC), \
	$$(TEST_OBJS) gtest-all.o bess.a, \
	$$(CXX) -o $$@ $$^ $$(LDFLAGS) $(LIBS)))

$(eval $(call BUILD, \
	TEST_CXX, \
	gtest-all.o, \
	$$(GTEST_DIR)/src/gtest-all.cc, \
	$$(CXX) -o $$@ -c $$< -I$$(GTEST_DIR) $$(CXXFLAGS) $$(DEPFLAGS)))

CORE_OBJS = $(OBJS)

$(eval $(call BUILD, \
	AR, \
	bess.a, \
	$(CORE_OBJS), \
	$$(AR) rcs $$@ $$^))

# The rule for linking samples are added below here.
$(eval $(call BUILD, \
	LD, \
	$$(PROCESS_PACKET_EXEC), \
	$$(PROCESS_PACKET_OBJS), \
	$$(CXX) -o $$@ $$^ $$(LDFLAGS) $$(LIBS)))
	
$(eval $(call BUILD, \
	LD, \
	$$(RPC_TEST_CLIENT_EXEC), \
	$$(RPC_TEST_CLIENT_OBJS), \
	$$(CXX) -o $$@ $$^ $$(LDFLAGS) $$(LIBS)))
	
$(eval $(call BUILD, \
	LD, \
	$$(RPC_TEST_SERVER_EXEC), \
	$$(RPC_TEST_SERVER_OBJS), \
	$$(CXX) -o $$@ $$^ $$(LDFLAGS) $$(LIBS)))

%.d: ;

.PRECIOUS: %.d $(PROTO_HEADERS)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(ALL_SRCS)))