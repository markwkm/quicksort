/*
 * Uncontrolled forking.  Spawn as many processes as needed in order to
 * completely sort an array of ints.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* Prototypes */

void swap(int[], int, int);

/* Declarations */

/*
 * Dump to the stdout the contents of the array to be sorted.
 */
void display(int array[], int length)
{
	int i;
	printf(">");
	for (i = 0; i < length; i++)
		printf(" %d", array[i]);
	printf("\n");
}

/*
 * Parition the data into two parts, elements smaller than the pivot to the
 * 'left' and elements larger to the 'right'.
 */
int partition(int array[], int left, int right, int pivot_index)
{
	int pivot_value = array[pivot_index];
	int store_index = left;
	int i;

	swap(array, pivot_index, right);
	for (i = left; i < right; i++)
		if (array[i] <= pivot_value) {
			swap(array, i, store_index);
			++store_index;
		}
	swap(array, store_index, right);
	return store_index;
}

void quicksort(int array[], int left, int right)
{
	int pivot_index = left;
	int pivot_new_index;

	/*
	 * Use -1 to initialize because fork() uses 0 to identify a process as a
	 * child.
	 */
	int lchild = -1;
	int rchild = -1;

	if (right > left) {
		int status; /* For waitpid() only. */

		pivot_new_index = partition(array, left, right, pivot_index);

		/*
		 * Parallize by processing the left and right partion siultaneously.
		 * Start by spawning the 'left' child.
		 */
		lchild = fork();
		if (lchild < 0) {
			perror("fork");
			exit(1);
		}
		if (lchild == 0) {
			/* The 'left' child starts processing. */
			quicksort(array, left, pivot_new_index - 1);
			exit(0);
		} else {
			/* The parent spawns the 'right' child. */
			rchild = fork();
			if (rchild < 0) {
				perror("fork");
				exit(1);
			}
			if (rchild == 0) {
				/* The 'right' child starts processing. */
				quicksort(array, pivot_new_index + 1, right);
				exit(0);
			}
		}
		/* Parent waits for children to finish. */
		waitpid(lchild, &status, 0);
		waitpid(rchild, &status, 0);
	}
}

/*
 * The actual sorting, swap elements in the array that is being sorted.
 */
void swap(int array[], int left, int right)
{
	int temp;
	temp = array[left];
	array[left] = array[right];
	array[right] = temp;
}

int main(int argc, char *argv[])
{
	int *array = NULL;
	int *shm_array;
	int length = 0;
	FILE *fh;
	int data;
	int shm_id;
	key_t key;
	int i;
	size_t shm_size;

	if (argc != 2) {
		printf("usage: %s <filename>\n", argv[0]);
		return 1;
	}

	/* Initialize data. */
	printf("attempting to sort file: %s\n", argv[1]);

	fh = fopen(argv[1], "r");
	if (fh == NULL) {
		printf("error opening file\n");
		return 0;
	}

	/*
	 * Read the data to be sorted from a file, using realloc() to dynamically
	 * allocated local storage space.  Then use the final count of items read
	 * to determine how large to make the shared memory segment.
	 */
	while (fscanf(fh, "%d", &data) != EOF) {
		++length;
		array = (int *) realloc(array, length * sizeof(int));
		array[length - 1] = data;
/*
		display(array, length);
*/
	}
	fclose(fh);
	printf("%d elements read\n", length);

	display(array, length);

	/* Use this process's pid as the shared memory key identifier. */
	key = IPC_PRIVATE;

	/* Create the shared memory segment. */
	shm_size = length * sizeof(int);
	if ((shm_id = shmget(key, shm_size, IPC_CREAT | 0666)) == -1) {
		perror("shmget");
		exit(1);
	}

	/* Attached to the shared memory segment in order to use it. */
	if ((shm_array = shmat(shm_id, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}

	/*
	 * Copy the data to be sorted from the local memory into the shared memory.
	 */
	for (i = 0; i < length; i++) {
		shm_array[i] = array[i];
	}
	display(shm_array, length);

	printf("now sorting...\n");
	quicksort(shm_array, 0, length - 1);
	printf("done sorting\n");

	display(shm_array, length);

	/* Detach from the shared memory now that we are done using it. */
	if (shmdt(shm_array) == -1) {
		perror("shmdt");
		exit(1);
	}

	/* Delete the shared memory segment. */
	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		exit(1);
	}

	return 0;
}
