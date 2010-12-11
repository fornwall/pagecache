#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
typedef char mincore_element_type;
#else
typedef unsigned char mincore_element_type;
#endif

int main(int argc, char** argv)
{
	if (argc != 2) { fprintf(stderr, "usage: %s file_to_examine\n", argv[0]); return 1; }
	char const* const file_name = argv[1];

	int const fd = open(file_name, O_RDONLY);
	if (fd == -1) { perror("open() failed"); return 1; }

	struct stat stat_buffer;
	if (fstat(fd, &stat_buffer) == -1) { perror("fstat() failed"); return 1; }
	if (stat_buffer.st_size == 0) { fprintf(stderr, "%s: File is empty\n", file_name); return 1; }

	void* mapped_mem = mmap(NULL, stat_buffer.st_size, PROT_NONE, MAP_FILE | MAP_PRIVATE, fd, 0);
	if (mapped_mem == MAP_FAILED) { perror("mmap() failed"); return 1; }

	int const page_size = getpagesize();
	int const num_pages = stat_buffer.st_size / page_size + (stat_buffer.st_size % page_size == 0 ? 0 : 1);
	mincore_element_type* page_vector = (mincore_element_type*) malloc(num_pages);
	if (page_vector == NULL) { fprintf(stderr, "malloc() for page_vector failed\n"); return 1; }
	if (mincore(mapped_mem, stat_buffer.st_size, page_vector) == -1) { perror("mincore() failed"); return 1; }

	int total_in_mem = 0;
	printf("Page cache for '%s' (file size=%lld, page size=%d):\n", file_name, stat_buffer.st_size, page_size);
	for (int i = 0; i < num_pages; i++) {
		bool const in_mem = page_vector[i] & 1;
		if (in_mem) total_in_mem++;
		printf(in_mem ? "1" : "0");
	}
	printf("\nSummary: %d/%d (%d%%) pages cached\n", total_in_mem, num_pages, (100*total_in_mem) / num_pages);

	return 0;
}
