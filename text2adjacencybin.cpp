// This create a binary representation of the adjacency log from the 
//  tuple representation of temporal edges.



#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned int uint;

struct infolog {
	uint nodes;
	uint changes;
	uint maxtime;
	uint size;
};

void create( FILE *out, FILE *f, struct infolog *infolog) {
	uint nodes;
	uint changes;
	uint maxtime;

	uint size_read;

	uint from, to, time, op;
	uint last_time, last_from;

	uint nodes_read=-1;
	int t;
	uint changes_read=0;

	fscanf(f, "%u %u %u", &nodes, &changes, &maxtime);
	printf("nodes: %u\nchanges: %u\nmaxtime: %u\n", nodes, changes, maxtime);

	if (maxtime + nodes >= 2147483647) {
		printf("graph too big!!! you should resize the time\n");
		exit(-1);
	}


	last_time = UINT_MAX;
	last_from = UINT_MAX;
	
	size_read = 0;
	
	while ( 4 == fscanf(f, "%u %u %u %u", &from, &to, &time, &op)) {
		
		if ( from != last_from) {

			// if next change belongs from the previous node
			if ( last_from +1 == from) {
				fwrite(&nodes_read, sizeof(int), 1, out);
				nodes_read--;
				size_read++;
				//*p++ = nodes + from;
			}
			else {
				while (++last_from <= from) {
					//*p++ = nodes + last_from;
					fwrite(&nodes_read, sizeof(int), 1, out);
					nodes_read--;
					size_read++;
				}
			}

			last_from = from;
			last_time = UINT_MAX;
		}

		if ( time != last_time) {
			//*p++ = nodes + time;
			t = nodes + time;
			fwrite(&t, sizeof(int), 1, out);
			size_read++;
		}

		//*p++ = to;
		fwrite(&to, sizeof(int), 1, out);
		
		last_time = time;
		
		changes_read++;
		size_read++;
		
		
		if (changes_read%1000000==0) {
			fprintf(stderr, "Processing: %0.2f%%\r", 100*(float)changes_read/changes);
		}
	}


	while (++last_from < nodes) {
		//*p++ = nodes + last_from;
		fwrite(&nodes_read, sizeof(int), 1, out);
		nodes_read--;
		size_read++;
	}


	

	nodes_read = -1*nodes_read - 1;


	

	printf("nodes_read: %d\n", nodes_read);
	printf("changes_read: %u\n", changes_read);
	printf("size_read: %u\n", size_read);
	
	assert(nodes == nodes_read);
	assert(changes_read == changes);
	
	infolog->nodes = nodes;
	infolog->changes = changes;
	infolog->maxtime = maxtime;
	infolog->size = size_read;

}


int main( int argc, char *argv[]) {
	FILE *in, *out;
	char filename[1000];
	struct infolog infolog;
	
	sprintf(filename, "%s.bin", argv[1]);
	
	in = fopen(argv[1], "r");
	out = fopen(filename, "w");
	
	
	fwrite(&infolog, sizeof(struct infolog), 1, out);
	create( out, in, &infolog);
	
	fseek(out, 0L, SEEK_SET);
	fwrite(&infolog, sizeof(struct infolog), 1, out);
	
	fclose(in);
	fclose(out);
	
	return 0;
}