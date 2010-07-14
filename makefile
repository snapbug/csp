SRCDIR = source
BINDIR = bin
OBJDIR = bin

LDFLAGS = -lm -fopenmp
NON_RATABLE ?= yes
TIME_EFFECTS ?= no
CORR ?= real

MINUS_D = 
ifeq ($(CORR),abs)
MINUS_D += -DABS_CORR
endif

ifeq ($(NON_RATABLE),yes)
MINUS_D += -DNON_RATABLE
endif

ifeq ($(TIME_EFFECTS),yes)
MINUS_D += -DTIME_EFFECTS
endif

# Normal
CFLAGS = -Wall -Wextra -O3 -pedantic -ansi -Wno-long-long $(MINUS_D) -fopenmp
## Debugging
#CFLAGS = -Wall -Wextra -Werror -O2 -pedantic -ansi -g

CC = g++

PARTS = \
	dataset_netflix.o \
	generator_entropy.o \
	generator_greedy_cheat.o \
	generator_item_avg.o \
	generator_naive_bayes.o \
	generator_popularity.o \
	generator_random.o \
	metric_mae.o \
	metric_nmae.o \
	metric_nrmse.o \
	metric_rmse.o \
	param_block.o \
	predictor_constant.o \
	predictor_global_avg.o \
	predictor_item_avg.o \
	predictor_item_knn.o \
	predictor_korbell.o \
	predictor_random.o \
	predictor_user_avg.o \
	predictor_user_knn.o \
	stats.o

PARTS_W_DIR = $(PARTS:%=$(OBJDIR)/%)

all : $(BINDIR)/csp

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/csp : $(PARTS_W_DIR) $(OBJDIR)/csp.o
	@echo Building $(notdir $@)
	@$(CC) $(LDFLAGS) -o $@ $^

$(PARTS_W_DIR) : makefile makefile.dependencies

.PHONY : clean
clean :
	\rm -f $(OBJDIR)/*.o $(BINDIR)/csp

.PHONY : depend
depend :
	makedepend -Y -f- -pbin/ -w1024 -- $(CFLAGS) -- $(SRCDIR)/*.c | sed "s/bin\/source/bin/" >| makefile.dependencies

include makefile.dependencies
