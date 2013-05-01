CC=mpicc
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/, mpi_kmeans.o main.o)
CFLAGS := -std=c99

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

mpi_kmeans: $(OBJS)
	$(CC) $(CFLAGS) $^ -o mpi_kmeans

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

clean:
	rm -rf obj mpi_kmeans
