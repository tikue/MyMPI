CC=g++
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/, data_points.o DNA_strands.o seq_main.o)

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -c -o $@

seq_kmeans: $(OBJS)
	$(CC) $(CFLAGS) $^ -o seq_kmeans

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

clean:
	rm -rf obj seq_kmeans
