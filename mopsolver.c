/**
 * file: mopsolver.c
 *
 * Find optimal solution for a maze provided
 * as a string of 0's and 1's if possible
 *
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Struct representing cell of grid
 */
struct Node {
    _Bool visit;
    char c;
    int x;
    int y;
    int i;
    struct Node *child[3];
    struct Node *parent;
};

/**
 * Linked list of nodes
 */
struct Queue {
    int i;
    struct Queue *next;
};

/**
 * Print program usage message
 */
void printUsage() {
    printf("USAGE:\n"
		    "mopsolver [-hdsp] [-i INFILE] [-o OUTFILE]\n"
		    "\n"
		    "Options:\n"
		    "        -h      "
		    "Print this helpful message to stdout and exit.\n"
		    "        -d      "
		    "Pretty print (display) the maze after reading.  "
		    "(Default: off)\n"
		    "        -s      "
		    "Print shortest solution steps.        		"
		    "(Default: off)\n"
		    "        -p      "
		    "Print an optimal path.                		"
		    "(Default: off)\n"
		    "        -i INFILE       "
		    "Read maze from INFILE.        		"
		    "(Default: stdin)\n"
		    "        -o OUTFILE      "
		    "Write all output to OUTFILE.  		"
		    "(Default: stdout)\n");
}

/**
 * Process command line arguments
 *
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @param d pointer to char indicating -d flag
 * @param s pointer to char indicating -s flag
 * @param p pointer to char indicating -p flag
 * @param inFile input file pointer, default stdin
 * @param outFile output file pointer, default stdout
 */
void processArgs(int argc, char **argv, _Bool *d, _Bool *s, _Bool *p,
		FILE **inFile, FILE **outFile) {
    _Bool in = 0, out = 0;
    for (int i = 1; i < argc; i++) {
        _Bool flag = 0;
	int j = 0;
        while (argv[i][j]) {
            char c = argv[i][j];
	    if (in) {
		*inFile = fopen(argv[i], "r");
		if (*inFile == NULL) {
		    perror(argv[i]);
		    exit(1);
		}
		in = 0;
		break;
	    }
	    if (out) {
		*outFile = fopen(argv[i], "w");
		if (*outFile == NULL) {
		    perror(argv[i]);
		    exit(1);
		}
		out = 0;
		break;
	    }
            if (c == '-') {
                flag = 1;
            }
            if (flag) {
                if (c == 'h') {
                    printUsage();
                    exit(0);
                }
                if (c == 'd') *d = 1;
                if (c == 's') *s = 1;
                if (c == 'p') *p = 1;
                if (c == 'i') {
                    in = 1;
		    break;
                }
                if (c == 'o') {
                    out = 1;
		    break;
                }
            }
            j++;
	}
    }
}

/**
 * Create maze given input file
 *
 * @param inFile file to read input from
 * @param row row dimension of maze
 * @param col col dimension of maze
 * @param maze array of pointers to Node structs
 */
void createMaze(FILE *inFile, int *row, int *col, struct Node ***maze) {
    _Bool newLine = 1;
    char c;
    int i = 0, len = 25;
    *maze = malloc(len * sizeof(struct Node *));
    assert(*maze);

    while ((c = fgetc(inFile)) != EOF) {
	if (c == '\n') {
	    newLine = 1;
	} else if (c == '0' || c == '1') {
	    if (newLine) {
		(*row)++;
		newLine = !newLine;
	    }
	    if (*row == 1) (*col)++;
            if ((*row - 1) * *col + i + 1 >= len) {
	        len *= 4;
	        *maze = realloc(*maze, len * sizeof(struct Node *));
		assert(*maze);
	    }
	    if (*row != 1) i %= *col;
	    struct Node *node = malloc(sizeof(struct Node));
	    assert(node);
	    node->visit = 0;
	    node->c = c;
	    if (*row == 1) node->x = *col - 1;
	    if (*row != 1) node->x = i;
	    node->y = *row - 1;
	    node->i = 0;
	    node->parent = NULL;
	    node->child[0] = NULL;
	    node->child[1] = NULL;
	    node->child[2] = NULL;
	    (*maze)[(*row - 1) * *col + i] = node;
	    i++;
	}
    }
}

/**
 * Solve the given maze if possible
 *
 * @param row row dimension of maze
 * @param col column dimension of maze
 * @param maze array of Nodes
 * @param head first element in queue
 * @param tail last element in queue
 * @return 0 if maze is not solveable
 *         1 if maze is solveable
 */
_Bool solve(int row, int col, struct Node **maze, struct Queue *head, struct Queue *tail) {
    if (head == NULL) {
	return 0;
    }

    struct Node *curr = maze[head->i];

    if (curr->x == col - 1 && curr->y == row - 1) {
	while (head) {
	    struct Queue *next = head->next;
	    free(head);
	    head = next;
	}
	return 1;
    } else {
	int i = curr->y * col + curr->x;
	struct Node *neighbor;
	if (curr->x - 1 >= 0) {
	    neighbor = maze[i - 1];
	    if (neighbor->c == '0' && curr->parent != neighbor &&
			    !neighbor->visit) {
		curr->child[curr->i++] = neighbor;
		neighbor->parent = curr;
		struct Queue *newTail = malloc(sizeof(struct Queue));
		assert(newTail);
		newTail->i = i - 1;
		newTail->next = NULL;
		tail->next = newTail;
		tail = newTail;
		neighbor->visit = 1;
	    }
	}
	if (curr->y - 1 >= 0) {
            neighbor = maze[i - col];
            if (neighbor->c == '0' && curr->parent != neighbor &&
			    !neighbor->visit) {
                curr->child[curr->i++] = neighbor;
		neighbor->parent = curr;
                struct Queue *newTail = malloc(sizeof(struct Queue));
		assert(newTail);
                newTail->i = i - col;
                newTail->next = NULL;
                tail->next = newTail;
                tail = newTail;
		neighbor->visit = 1;
            }
        }
	if (curr->x + 1 < col) {
            neighbor = maze[i + 1];
            if (neighbor->c == '0' && curr->parent != neighbor &&
			    !neighbor->visit) {
                curr->child[curr->i++] = neighbor;
		neighbor->parent = curr;
		struct Queue *newTail = malloc(sizeof(struct Queue));
		assert(newTail);
                newTail->i = i + 1;
                newTail->next = NULL;
                tail->next = newTail;
                tail = newTail;
		neighbor->visit = 1;
            }
        }
	if (curr->y + 1 < row) {
            neighbor = maze[i + col];
            if (neighbor->c == '0' && curr->parent != neighbor &&
			    !neighbor->visit) {
                curr->child[curr->i++] = neighbor;
		neighbor->parent = curr;
		struct Queue *newTail = malloc(sizeof(struct Queue));
		assert(newTail);
                newTail->i = i + col;
                newTail->next = NULL;
                tail->next = newTail;
                tail = newTail;
		neighbor->visit = 1;
            }
        }
    }
    struct Queue *next = head->next;
    if (tail == head) tail = NULL;
    free(head);
    head = next;
    return solve(row, col, maze, head, tail);
}

/**
 * Pretty print maze
 *
 * @param outFile file to write output to
 * @param row row dimension of maze
 * @param col column dimension of maze
 * @param maze array of nodes
 */
void printMaze(FILE *outFile, int row, int col, struct Node **maze) {
    fprintf(outFile, "|-");
    for (int j = 0; j < col; j++) {
	fprintf(outFile, "--");
    }
    fprintf(outFile, "|\n");

    for (int i = 0; i < row; i++) {
	if (i == 0) fprintf(outFile, "  ");
	if (i != 0) fprintf(outFile, "| ");

	for (int j = 0; j < col; j++) {
	    if (maze[i * col + j]->c == '0') {
		fprintf(outFile, ". ");
	    } else if (maze[i * col + j]->c == '1') {
		fprintf(outFile, "# ");
	    } else {
		fprintf(outFile, "%c ", maze[i * col + j]->c);
	    }
	}

	if (i == row - 1) fprintf(outFile, " ");
	if (i != row - 1) fprintf(outFile, "|");
	fprintf(outFile, "\n");
    }

    fprintf(outFile, "|-");
    for (int j = 0; j < col; j++) {
        fprintf(outFile, "--");
    }
    fprintf(outFile, "|\n");
}

/**
 * Update maze with solution path
 *
 * @param node exit cell of maze to track solution path
 */
int updateMaze(struct Node *node) {
    int i = 0;
    while (node) {
	node->c = '+';
	node = node->parent;
	i++;
    }
    return i;
}

/**
 * Free allocated memory for solution
 *
 * @param size size of maze array
 * @param maze the array of nodes
 */
void freeMem(int size, struct Node **maze) {
    if (maze) {
        for (int i = 0; i < size; i++) {
	    free(maze[i]);
        }
        free(maze);
    }
}

/**
 * Main entry to program mopsolver
 *
 * @param argc number of command line arguments
 * @param argv the command line arguments
 * @return 0 on successful execution
 */
int main(int argc, char *argv[]) {
    _Bool d = 0, s = 0, p = 0;
    int row = 0, col = 0;
    FILE *inFile = stdin, *outFile = stdout;
    struct Node **maze = NULL;
    struct Queue *queue = NULL;
    
    processArgs(argc, argv, &d, &s, &p, &inFile, &outFile);
    createMaze(inFile, &row, &col, &maze);

    if (d) printMaze(outFile, row, col, maze);
    if (s || p) {
	_Bool solveable = 0;
	if (maze[0]->c != '1' && maze[row * col - 1]->c != '1') {
	    queue = malloc(sizeof(struct Queue));
	    assert(queue);
	    queue->i = 0;
	    queue->next = NULL;
	    solveable = solve(row, col, maze, queue, queue);
	}

	if (s && !solveable) fprintf(outFile, "No solution.\n");
	if (s && solveable) fprintf(outFile,
			"Solution in %d steps.\n",
			updateMaze(maze[row * col - 1]));
	if (p) printMaze(outFile, row, col, maze);
    }
    freeMem(row * col, maze);
    fclose(inFile);
    fclose(outFile);

    return 0;
}
