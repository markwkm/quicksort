#include <stdio.h>
#include <stdlib.h>

/* Prototypes */
void swap(int[], int, int);

/* Declarations */

void display(int array[], int length)
{
	int i;
	printf(">");
	for (i = 0; i < length; i++)
		printf(" %d", array[i]);
	printf("\n");
}

int partition(int array[], int left, int right, int pivot_index)
{
	int pivot_value = array[pivot_index];
	int store_index = left;
	int i;

	printf("%d %d %d\n", left, right, pivot_index);
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

	loop:
	printf("sorting %d to %d\n", left, right);
	if (right > left) {
		pivot_new_index = partition(array, left, right, pivot_index);
		printf("L\n");
		quicksort(array, left, pivot_new_index - 1);
		printf("R\n");
		pivot_index = left = pivot_new_index + 1;
		goto loop;
	}
}

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
	int length = 0;
	FILE *fh;
	int data;

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

	while (fscanf(fh, "%d", &data) != EOF) {
		++length;
		array = (int *) realloc(array, length * sizeof(int));
		array[length - 1] = data;
		display(array, length);
	}
	fclose(fh);
	printf("%d elements read\n", length);

	display(array, length);
	quicksort(array, 0, length - 1);
	display(array, length);

	return 0;
}
