OS = linux
#OS = windows
#OS = apple


ifeq ($(OS),linux)
  LDPTHREAD = 
  LBM = -lm
  LBSOCKETS = 
  LBPTHREAD = -lpthread
else ifeq ($(OS),windows)
  # set to pthread folder
  LDPTHREAD = -L prebuilt-dll-2-9-1-release/lib/x86
  LBM = 
  LBSOCKETS = -lws2_32
  #can be -lpthreadGC2 or -lpthread
  LBPTHREAD = -lpthreadGC2
else ifeq ($(OS),apple)
  LDPTHREAD = 
  LBM = -lm
  LBSOCKETS = 
  LBPTHREAD = -lpthread
else
  #ID specific include dirs
  #LD specific library dirs
  LDPTHREAD = 
  #LB specific libraries
  LBM = 
  LBSOCKETS = 
  LBPTHREAD = 
endif


CC = gcc

BDIR = bin
SDIR = src

CCFLAGS_DBG = -ggdb -Wall -O0
CCFLAGS_OPT = -O2 -funroll-loops
CCFLAGS_STD = 

CLFLAGS_STD = $(LBM)
CLFLAGS_NW = $(LBM) $(LBSOCKETS)
CLFLAGS_MT = $(LBM) $(LBPTHREAD) $(LDPTHREAD)
CLFLAGS_MTNW = $(LBM) $(LBPTHREAD) $(LBSOCKETS) $(LDPTHREAD)


UTL_ODIR = $(SDIR)/utl
UTL_OBJ = $(UTL_ODIR)/utl.o
UTL_INC = 
UTL_CCFLAGS = $(CCFLAGS_STD)

BS_ODIR = $(SDIR)/bs
BS_OBJ = $(BS_ODIR)/bs.o
BS_INC = 
BS_CCFLAGS = $(CCFLAGS_STD)

NTQC_ODIR = $(SDIR)/ntqc
NTQC_INC = -I $(UTL_ODIR) -I $(BS_ODIR)
NTQC_OBJ = $(NTQC_ODIR)/ntqc.o
NTQC_CCFLAGS = $(CCFLAGS_DBG)

NTQS_ODIR = $(SDIR)/ntqs
NTQS_INC = -I $(UTL_ODIR) -I $(BS_ODIR)
NTQS_OBJ = $(NTQS_ODIR)/ntqsmain.o $(NTQS_ODIR)/ntqs.o $(NTQS_ODIR)/ntqsmanager.o
NTQS_CCFLAGS = $(CCFLAGS_DBG)

NTQS_BIN = $(BDIR)/ntqs
NTQS_BIN_OBJ = $(UTL_OBJ) $(BS_OBJ) $(NTQS_OBJ)
NTQS_CLFLAGS = $(CLFLAGS_MTNW)



TMWORKER_ODIR = $(SDIR)/tmworker
TMWORKER_INC = -I $(UTL_ODIR) -I $(BS_ODIR) -I $(NTQC_ODIR)
TMWORKER_OBJ = $(TMWORKER_ODIR)/tmworkermain.o $(TMWORKER_ODIR)/tmw_opt.o  $(TMWORKER_ODIR)/tmw_priceshift_models.o $(TMWORKER_ODIR)/tmw_prob_models.o
TMWORKER_CCFLAGS = $(CCFLAGS_OPT)
TMWORKER_BIN = $(BDIR)/tmworker
TMWORKER_BIN_OBJ = $(UTL_OBJ) $(BS_OBJ) $(NTQC_OBJ) $(TMWORKER_OBJ)
TMWORKER_CLFLAGS = $(CLFLAGS_NW)

TMWORKERTEST_ODIR = $(SDIR)/tmworker-test
TMWORKERTEST_INC = -I $(UTL_ODIR) -I $(TMWORKER_ODIR)
TMWORKERTEST_OBJ = $(TMWORKERTEST_ODIR)/testmain.o $(TMWORKER_ODIR)/tmw_opt.o  $(TMWORKER_ODIR)/tmw_priceshift_models.o $(TMWORKER_ODIR)/tmw_prob_models.o
TMWORKERTEST_CCFLAGS = $(CCFLAGS_OPT)
TMWORKERTEST_BIN = $(BDIR)/tmworker-test
TMWORKERTEST_BIN_OBJ = $(UTL_OBJ) $(TMWORKERTEST_OBJ)
TMWORKERTEST_CLFLAGS = $(CLFLAGS_STD)

TMJOBSUPPLIER_ODIR = $(SDIR)/tmjobsupplier
TMJOBSUPPLIER_INC = -I $(UTL_ODIR) -I $(BS_ODIR) -I $(NTQC_ODIR) -I $(TMWORKER_ODIR)
TMJOBSUPPLIER_OBJ = $(TMJOBSUPPLIER_ODIR)/tmjobsuppliermain.o $(TMWORKER_ODIR)/tmw_opt.o  $(TMWORKER_ODIR)/tmw_priceshift_models.o $(TMWORKER_ODIR)/tmw_prob_models.o
TMJOBSUPPLIER_CCFLAGS = $(CCFLAGS_DBG)
TMJOBSUPPLIER_BIN = $(BDIR)/tmjobsupplier
TMJOBSUPPLIER_BIN_OBJ = $(UTL_OBJ) $(BS_OBJ) $(NTQC_OBJ) $(TMJOBSUPPLIER_OBJ)
TMJOBSUPPLIER_CLFLAGS = $(CLFLAGS_NW)

TMRESULTSAVER_ODIR = $(SDIR)/tmresultsaver
TMRESULTSAVER_INC = -I $(UTL_ODIR) -I $(BS_ODIR) -I $(NTQC_ODIR) -I $(TMWORKER_ODIR)
TMRESULTSAVER_OBJ = $(TMRESULTSAVER_ODIR)/tmresultsavermain.o $(TMWORKER_ODIR)/tmw_opt.o  $(TMWORKER_ODIR)/tmw_priceshift_models.o $(TMWORKER_ODIR)/tmw_prob_models.o
TMRESULTSAVER_CCFLAGS = $(CCFLAGS_DBG)
TMRESULTSAVER_BIN = $(BDIR)/tmresultsaver
TMRESULTSAVER_BIN_OBJ = $(UTL_OBJ) $(BS_OBJ) $(NTQC_OBJ) $(TMRESULTSAVER_OBJ)
TMRESULTSAVER_CLFLAGS = $(CLFLAGS_NW)



all: $(NTQS_BIN) $(BDIR)/ntq-client-str $(TMWORKER_BIN) $(TMWORKERTEST_BIN) $(TMJOBSUPPLIER_BIN) $(TMRESULTSAVER_BIN)

# compiling objects
$(UTL_ODIR)/%.o: $(UTL_ODIR)/%.c
	$(CC) $(UTL_CCFLAGS) -c $< -o $@ $(UTL_INC)
	
$(BS_ODIR)/%.o: $(BS_ODIR)/%.c
	$(CC) $(BS_CCFLAGS) -c $< -o $@ $(BS_INC)
	
$(NTQC_ODIR)/%.o: $(NTQC_ODIR)/%.c
	$(CC) $(NTQC_CCFLAGS) -c $< -o $@ $(NTQC_INC)
	
$(NTQS_ODIR)/%.o: $(NTQS_ODIR)/%.c
	$(CC) $(NTQS_CCFLAGS) -c $< -o $@ $(NTQS_INC)

$(SDIR)/ntq-client-str/ntq-client-str.o: $(SDIR)/ntq-client-str/ntq-client-str.c
	$(CC) $(CCFLAGS_DBG) -o $(SDIR)/ntq-client-str/ntq-client-str.o -c $(SDIR)/ntq-client-str/ntq-client-str.c -I $(UTL_ODIR) -I $(BS_ODIR) -I $(NTQC_ODIR)


$(TMWORKER_ODIR)/%.o: $(TMWORKER_ODIR)/%.c
	$(CC) $(TMWORKER_CCFLAGS) -c $< -o $@ $(TMWORKER_INC)

$(TMWORKERTEST_ODIR)/%.o: $(TMWORKERTEST_ODIR)/%.c
	$(CC) $(TMWORKERTEST_CCFLAGS) -c $< -o $@ $(TMWORKERTEST_INC)

$(TMJOBSUPPLIER_ODIR)/%.o: $(TMJOBSUPPLIER_ODIR)/%.c
	$(CC) $(TMJOBSUPPLIER_CCFLAGS) -c $< -o $@ $(TMJOBSUPPLIER_INC)

$(TMRESULTSAVER_ODIR)/%.o: $(TMRESULTSAVER_ODIR)/%.c
	$(CC) $(TMRESULTSAVER_CCFLAGS) -c $< -o $@ $(TMRESULTSAVER_INC)


# linking programs
$(NTQS_BIN): $(NTQS_BIN_OBJ)
	$(CC) -o $(NTQS_BIN) $(NTQS_BIN_OBJ) $(NTQS_CLFLAGS)

$(BDIR)/ntq-client-str: $(SDIR)/ntq-client-str/ntq-client-str.o $(UTL_OBJ) $(BS_OBJ) $(NTQC_OBJ)
	$(CC) -o $(BDIR)/ntq-client-str $(SDIR)/ntq-client-str/ntq-client-str.o $(UTL_OBJ) $(BS_OBJ) $(NTQC_OBJ) $(CLFLAGS_NW)


$(TMWORKER_BIN): $(TMWORKER_BIN_OBJ)
	$(CC) -o $(TMWORKER_BIN) $(TMWORKER_BIN_OBJ) $(TMWORKER_CLFLAGS)

$(TMWORKERTEST_BIN): $(TMWORKERTEST_BIN_OBJ)
	$(CC) -o $(TMWORKERTEST_BIN) $(TMWORKERTEST_BIN_OBJ) $(TMWORKERTEST_CLFLAGS)

$(TMJOBSUPPLIER_BIN): $(TMJOBSUPPLIER_BIN_OBJ)
	$(CC) -o $(TMJOBSUPPLIER_BIN) $(TMJOBSUPPLIER_BIN_OBJ) $(TMJOBSUPPLIER_CLFLAGS)

$(TMRESULTSAVER_BIN): $(TMRESULTSAVER_BIN_OBJ)
	$(CC) -o $(TMRESULTSAVER_BIN) $(TMRESULTSAVER_BIN_OBJ) $(TMRESULTSAVER_CLFLAGS)


clean:
	rm $(NTQS_BIN) $(BDIR)/ntq-client-str $(TMWORKER_BIN) $(TMWORKERTEST_BIN) $(TMJOBSUPPLIER_BIN) $(TMRESULTSAVER_BIN) -f
	rm $(UTL_ODIR)/*.o -f
	rm $(BS_ODIR)/*.o -f
	rm $(NTQC_ODIR)/*.o -f
	rm $(NTQS_ODIR)/*.o -f
	rm $(SDIR)/ntq-client-str/*.o -f
	rm $(TMWORKER_ODIR)/*.o -f
	rm $(TMWORKERTEST_ODIR)/*.o -f
	rm $(TMJOBSUPPLIER_ODIR)/*.o -f
	rm $(TMRESULTSAVER_ODIR)/*.o -f
	rm $(SDIR)/*.o -f

