CC=mpic++
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/, kmeans.o main.o)
CFLAGS := -std=c99

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

kmeans: $(OBJS)
	$(CC) $(CFLAGS) $^ -o kmeans

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

clean:
	rm -rf obj kmeans
