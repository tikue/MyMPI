CC=mpicc
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/, kmeans.o main.o)

$(OBJDIR)/%.o: %.c
	$(CC) $< -c -o $@

kmeans: $(OBJS)
	$(CC) $^ -o kmeans

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

clean:
	rm -rf obj kmeans
