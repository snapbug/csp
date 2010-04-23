SRCDIR = source
BINDIR = bin
OBJDIR = bin

LDFLAGS = -lm

# Normal
CFLAGS = -Wall -Wextra -O3 -pedantic -ansi
## Debugging
#CFLAGS = -Wall -Wextra -Werror -O2 -pedantic -ansi -g

CC = g++

PARTS = \
	dataset_netflix.o \
	generator_entropy.o \
	generator_item_avg.o \
	generator_naive_bayes.o \
	generator_popularity.o \
	generator_random.o \
	metric_factory.o \
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
	@echo $(notdir $<)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/csp : $(PARTS_W_DIR) $(OBJDIR)/csp.o
	@echo Building $(notdir $@)
	@$(CC) $(LDFLAGS) -o $@ $^

$(PARTS_W_DIR) : makefile

.PHONY : clean
clean :
	\rm $(OBJDIR)/*.o $(BINDIR)/csp

.PHONY : depend
depend :
	makedepend -Y -f- -pbin/ -w1024 -- $(CFLAGS) -- $(SRCDIR)/*.c | sed "s/bin\/source/bin/" >| makefile.dependencies

include makefile.dependencies
