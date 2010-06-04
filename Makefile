all: quicksort quicksort-parallel quicksort-pool

quicksort: quicksort.c
	gcc -g -Wall $< -o $@

quicksort-parallel: quicksort-parallel.c
	gcc -g -Wall $< -o $@

quicksort-pool: quicksort-pool.c
	gcc -g -Wall $< -o $@

clean:
	rm -f quicksort quicksort-parallel quicksort-pool
