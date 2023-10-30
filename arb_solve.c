// gcc -o arb_solve arb_solve.c -std=c99 -pedantic -Wall
// -I/c/Users/username/AppData/Local/miniconda3/envs/envname/Library/include
// -I/c/Users/username/AppData/Local/miniconda3/envs/envname/Library/include/flint
// -Lc/Users/username/AppData/Local/miniconda3/envs/envname/Library/lib
// -lflint


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arb.h"
#include "acb.h"
#include "arb_mat.h"
#include "acb_mat.h"
#define PATHBUFSIZE 1000


void read_mat(acb_mat_t a, const char* path, slong prec) {
    // Reads a complex matrix column after column from a matrix-market format file written by
    // scipy.io.mmwrite('a', a, field='complex', precision=18, symmetry='general')
    char buff[255];
    int rows;
    int cols;
    FILE* fp = fopen(path, "r");
    // skip the two header lines
    fgets(buff, 255, (FILE*) fp);
    fgets(buff, 255, (FILE*) fp);
    // 3rd line contains dimension info
    fgets(buff, 255, (FILE*) fp);
    rows = atoi(strtok(buff, " "));
    cols = atoi(strtok(NULL, "\n"));
    // init matrix
    acb_mat_init(a, rows, cols);
    // read lines with complex entries
    for(int j = 0; j < cols; j++) {
        for(int i = 0; i < rows; i++) {
            fgets(buff, 255, (FILE*) fp);
            arb_set_str(acb_realref(acb_mat_entry(a, i, j)), strtok(buff, " "), prec);
            arb_set_str(acb_imagref(acb_mat_entry(a, i, j)), strtok(NULL, "\n"), prec);
        }
    }
    fclose(fp);
}


void test_read_mat() {
    acb_mat_t a;
    read_mat(a, "a.mtx", 106);
    printf("[\n");
    for(int i = 0; i < acb_mat_nrows(a); i++) {
        printf("[\n");
        for(int j = 0; j < acb_mat_ncols(a); j++) {
            printf("%s %s\n",
                   arb_get_str(acb_realref(acb_mat_entry(a, i, j)), 19, ARB_STR_NO_RADIUS),
                   arb_get_str(acb_imagref(acb_mat_entry(a, i, j)), 19, ARB_STR_NO_RADIUS));
        }
        printf("]\n");
    }
    printf("]\n");
    acb_mat_clear(a);
}


void read_vec(acb_mat_t b, const char* path, slong prec) {
    // Reads a complex vector written by numpy.savetxt('b.txt', b, fmt='%.18e %.18e', header=str(b.shape[0])).
    char buff[255];
    int rows;
    FILE* fp = fopen(path, "r");
    // 1st line contains dimension info
    fgets(buff, 255, (FILE*) fp);
    rows = atoi(strtok(buff, "# \n"));
    // init matrix
    acb_mat_init(b, rows, 1);
    // read lines with complex entries
    for(int i = 0; i < rows; i++) {
        fgets(buff, 255, (FILE*) fp);
        arb_set_str(acb_realref(acb_mat_entry(b, i, 0)), strtok(buff, " "), prec);
        arb_set_str(acb_imagref(acb_mat_entry(b, i, 0)), strtok(NULL, "\n"), prec);
    }
    fclose(fp);
}


void test_read_vec() {
    acb_mat_t b;
    read_vec(b, "b.txt", 106);
    printf("[\n");
    for(int i = 0; i < acb_mat_nrows(b); i++) {
        printf("%s %s\n",
               arb_get_str(acb_realref(acb_mat_entry(b, i, 0)), 19, ARB_STR_NO_RADIUS),
               arb_get_str(acb_imagref(acb_mat_entry(b, i, 0)), 19, ARB_STR_NO_RADIUS));
    }
    printf("]\n");
    acb_mat_clear(b);
}


void write_vec(acb_mat_t x, const char* path, slong digits) {
    // Writes the complex solution vector x of ax=b to a file to be read via x = numpy.loadtxt('x.txt')
    // followed by x = x[:, 0] + 1j * x[:, 1]
    FILE* fp = fopen(path, "w");
    int rows = acb_mat_nrows(x);
    // There is no need to start the file with a header commenting the dimension, but let's just do it.
    fprintf(fp, "# %d\n", rows);
    for(int i = 0; i < rows; i++) {
        fprintf(fp, "%s %s\n",
                arb_get_str(acb_realref(acb_mat_entry(x, i, 0)), digits, ARB_STR_NO_RADIUS),
                arb_get_str(acb_imagref(acb_mat_entry(x, i, 0)), digits, ARB_STR_NO_RADIUS));
//                arb_get_str(acb_realref(acb_mat_entry(x, i, 0)), digits, 0),
//                arb_get_str(acb_imagref(acb_mat_entry(x, i, 0)), digits, 0),
//                (int) arb_rel_accuracy_bits(acb_realref(acb_mat_entry(x, i, 0))),
//                (int) arb_rel_accuracy_bits(acb_imagref(acb_mat_entry(x, i, 0))));
    }
    fclose(fp);
}


void test_write_vec() {
    acb_mat_t x;
    acb_mat_init(x, 2, 1);
    arb_set_str(acb_realref(acb_mat_entry(x, 0, 0)), "2.3e-11", 106);
    arb_set_str(acb_imagref(acb_mat_entry(x, 0, 0)), "-5.1e2", 106);
    arb_set_str(acb_realref(acb_mat_entry(x, 1, 0)), "0.0", 106);
    arb_set_str(acb_imagref(acb_mat_entry(x, 1, 0)), "1", 106);
    write_vec(x, "x.txt", 19);
    acb_mat_clear(x);
}


int main()
{
    char dir[PATHBUFSIZE];
    const char *envvar = "SOLVE_DIR";
    char path[PATHBUFSIZE];

    // Make sure envvar actually exists
    if(!getenv(envvar)){
        fprintf(stderr, "The environment variable %s was not found.\n", envvar);
        exit(1);
    }

    // Make sure the buffer is large enough to hold the environment variable value.
    if(snprintf(dir, PATHBUFSIZE, "%s", getenv(envvar)) >= PATHBUFSIZE){
        fprintf(stderr, "PATHBUFSIZE of %d was too small. Aborting\n", PATHBUFSIZE);
        exit(1);
    }

    slong prec = 212;
    acb_mat_t a;
    acb_mat_t b;
    acb_mat_t x;

    sprintf(path, "%s/a.mtx", dir);
    read_mat(a, path, prec);
    sprintf(path, "%s/b.txt", dir);
    read_vec(b, path, prec);
    acb_mat_init(x, acb_mat_ncols(a), 1);

    flint_printf("Matrices read\n");
    printf("%i\n", acb_mat_solve(x, a, b, prec));
    sprintf(path, "%s/x.txt", dir);
    write_vec(x, path, 19);

    acb_mat_clear(a);
    acb_mat_clear(b);
    acb_mat_clear(x);
}
