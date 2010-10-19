SRCDIR = source
BINDIR = bin
OBJDIR = bin

MINUS_D = 

ifdef CORR
MINUS_D += -DABS_CORR
endif

ifdef TIME_EFFECTS
MINUS_D += -DTIME_EFFECTS
endif

ifndef NONRATABLE
MINUS_D += -DNON_RATABLE
endif

ifdef MOVIELENS
MINUS_D += -DML
endif

ifdef ENT
MINUS_D += -DENT
endif

ifdef ASC
MINUS_D += -DASC
endif

CFLAGS = -Wall -Wextra -pedantic -ansi -Wno-long-long $(MINUS_D)

ifdef DEBUG
CFLAGS += -g -O2
else
CFLAGS += -fopenmp -O3 -g
endif

LDFLAGS = -lm -fopenmp

CC = g++

PARTS = \
	dataset_netflix.o \
	dataset_movielens.o \
	generator_distance.o \
	generator_entropy.o \
	generator_greedy_cheat.o \
	generator_item_avg.o \
	generator_naive_bayes.o \
	generator_other_greedy.o \
	generator_other_greedy_pers.o \
	generator_popularity.o \
	generator_predictor.o \
	generator_random.o \
	generator_tree.o \
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
	@echo $(basename $(notdir $<))
	@$(CC) $(CFLAGS) -c $< -o $@

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
