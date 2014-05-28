#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __linux__
typedef char mincore_element_type;
#else
typedef unsigned char mincore_element_type;
#endif

int main(int argc, char** argv)
{
	if (argc < 2) { fprintf(stderr, "usage: %s file_to_examine\n", argv[0]); return 1; }
	for (int i = 1; i < argc; i++) {
		char const* const file_name = argv[i];

		int const fd = open(file_name, O_RDONLY);
		if (fd == -1) { fprintf(stderr, "%s - ", argv[i]); perror("open() failed"); continue; }

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
		for (int p = 0; p < num_pages; p++) if (page_vector[p] & 1) total_in_mem++;

		printf("%s (file size=%lld, page size=%d", file_name, (long long) stat_buffer.st_size, page_size);
		printf(", %d/%d = %d%% pages cached): ", total_in_mem, num_pages, (100*total_in_mem) / num_pages);

		for (int p = 0; p < num_pages; p++) {
			bool const in_mem = page_vector[p] & 1;
			printf(in_mem ? "1" : "0");
		}
		printf("\n");
		free(page_vector);
	}

	return 0;
}
