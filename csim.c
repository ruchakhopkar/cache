#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Name: Rucha Khopkar
// Andrew ID: rkhopkar
/*Implemented a cache using a structure for each block containing 4 variables to
store tag, count , valid bit and dirty bit.
Written codes for hit, miss, initial conditions and evictions using round robin
technique for both load and store.*/

typedef struct { // creating a struct describing each block in the cache. Every
                 // block consists of the tag, valid bit, dirty bit and count.
    long tag;
    int count;
    int valid_bit;
    int dirty_bit;
} blocks;

int main(int argc, char *argv[]) {

    csim_stats_t a = {.hits = 0,
                      .misses = 0,
                      .evictions = 0,
                      .dirty_bytes = 0,
                      .dirty_evictions = 0}; // initializing values

    int replacement;
    char *fname = NULL;
    int c;
    int s = 0, S = 0, b = 0, E = 0, B = 0;
    // taking input arguments
    while ((c = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (c) {
        case 's':
            s = atoi(optarg);
            S = 1 << s;
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            B = 1 << b;
            break;
        case 't':
            fname = optarg;
            break;
        default:
            return 0;
        }
    }

    FILE *fptr;
    int size = S * E;
    blocks *cache =
        malloc(size * sizeof(*cache)); // initializing cache using malloc
    if (cache == NULL)                 // checking if cache is NULL
    {
        free(cache);
        return 0;
    }
    long set_index;
    long tag;
    long t = 64 - b - s; // defining number of bits allocated for t based on the
                         // values of b and s
    int bytes;
    long address;
    char method;
    // initializing all values of struct as 0
    int i, j;
    for (i = 0; i < S; i++) {
        for (j = 0; j < E; j++) {
            cache[i * E + j].count = 0;
            cache[i * E + j].dirty_bit = 0;
            cache[i * E + j].tag = 0;
            cache[i * E + j].valid_bit = 0;
        }
    }

    if (fname != NULL) // getting the total number of elements in the file
    {
        fptr = fopen(fname, "r");
        int line_count = 0;
        if (fptr == NULL) // checking if file pointer is NULL
        {
            free(cache);
            return 0;
        }
        while (!feof(fptr)) {
            fscanf(fptr, "%c%*c%lx%*c%d%*c", &method, &address, &bytes);
            line_count++;
        }
        fclose(fptr);

        fptr = fopen(fname, "r"); // parsing all the inputs in the file based on
                                  // the value in line_count
        if (fptr == NULL)         // checking if fptr is NULL
        {
            fclose(fptr);
            free(cache);
            return 0;
        }
        int real_count = 0;
        while (!feof(fptr)) {
            real_count++;
            if (real_count ==
                line_count) // checking to see if all the inputs have reached
            {
                break;
            }
            fscanf(fptr, "%c%*c%lx%*c%d%*c", &method, &address,
                   &bytes); // scanning to get the method 'L' or 'S', the
                            // address and the bytes
            set_index = ((unsigned long)(address << (t - 1) << 1)) >>
                        (b + t); // getting the set index
            tag = ((unsigned long)(address)) >> (s + b); // getting the tag

            // loading
            if (method == 'L') {
                int flag = 0;
                replacement = 0;
                int i;
                for (i = 0; i < E; i++) {
                    if (cache[(set_index * E) + i].tag == tag &&
                        cache[(set_index * E) + i].valid_bit ==
                            1) // checking to see if hit has occured
                    {
                        a.hits++;
                        int round_robin =
                            cache[(set_index * E) + i]
                                .count; // storing the current value of count
                        for (int j = 0; j < E; j++) {
                            if (cache[(set_index * E) + j].valid_bit == 1 &&
                                cache[(set_index * E) + j].count <=
                                    round_robin &&
                                j != i) // checking to see if the count value
                                        // has to be updated for the blocks
                            {
                                cache[(set_index * E) + j].count += 1;
                            }
                        }
                        cache[(set_index * E) + i].count = 0;
                        flag = 1;
                        break;
                    } else {
                        continue;
                    }
                }
                if (!flag) // if hit has not occured, then its a miss
                {
                    int valid_flag = 0;
                    int i;
                    a.misses++;
                    for (i = 0; i < E; i++) {
                        if (cache[(set_index * E) + i].valid_bit ==
                            0) // checking to see if this is the initial
                               // condition when the entire cache is empty
                        {
                            valid_flag = 1;
                            cache[(set_index * E) + i].valid_bit = 1;
                            int round_robin = cache[(set_index * E) + i].count;
                            cache[(set_index * E) + i].count = 0;
                            cache[(set_index * E) + i].tag = tag;
                            cache[(set_index * E) + i].dirty_bit = 0;

                            for (int j = 0; j < E;
                                 j++) // Least Recently Used for initial
                                      // condition with round robin algorithm
                            {
                                if (cache[(set_index * E) + j].valid_bit == 1 &&
                                    cache[(set_index * E) + j].count <=
                                        round_robin &&
                                    j != i) {
                                    cache[(set_index * E) + j].count += 1;
                                }
                            }
                            break;
                        }
                    }
                    if (!valid_flag) // if not an initial condition(no blocks
                                     // are empty)
                    {
                        int final_count = cache[(set_index * E)].count;
                        int j;
                        for (j = 0; j < E; j++) {
                            if (cache[(set_index * E) + j].count >=
                                    final_count &&
                                cache[(set_index * E) + j].valid_bit ==
                                    1) // finding the element to be removed by
                                       // finding the least recently used
                                       // element
                            {
                                final_count = cache[(set_index * E) + j].count;
                                replacement = j;
                            }
                        }
                        cache[(set_index * E) + replacement].tag = tag;
                        if (cache[(set_index * E) + replacement].dirty_bit ==
                            1) // checking to see if the dirty bit for the
                               // evicted block was set, and if so, incrementing
                               // the dirty evictions
                        {
                            a.dirty_evictions += B;
                        }
                        cache[(set_index * E) + replacement].dirty_bit =
                            0; // replacing blocks
                        int round_robin =
                            cache[(set_index * E) + replacement].count;
                        cache[(set_index * E) + replacement].count = 0;
                        a.evictions++;

                        for (int k = 0; k < E;
                             k++) // least recently used algorithm
                        {
                            if (cache[(set_index * E) + k].valid_bit == 1 &&
                                cache[(set_index * E) + k].count <=
                                    round_robin &&
                                k != replacement) {
                                cache[(set_index * E) + k].count += 1;
                            }
                        }
                    }
                }
            }
            // store
            if (method == 'S') {
                int flag = 0;
                int replacement = 0;
                int i;
                for (i = 0; i < E; i++) {
                    if (cache[(set_index * E) + i].tag == tag &&
                        cache[(set_index * E) + i].valid_bit ==
                            1) // checking to see if hit has happened
                    {
                        a.hits++;
                        int round_robin = cache[(set_index * E) + i].count;
                        for (int j = 0; j < E;
                             j++) // least recently used algorithm using round
                                  // robin
                        {
                            if (cache[(set_index * E) + j].valid_bit == 1 &&
                                cache[(set_index * E) + j].count <=
                                    round_robin &&
                                j != i) {
                                cache[(set_index * E) + j].count += 1;
                            }
                        }
                        cache[(set_index * E) + i].count = 0;
                        flag = 1;
                        cache[(set_index * E) + i].dirty_bit =
                            1; // setting the dirty bit because store has been
                               // called
                        break;
                    } else {
                        continue;
                    }
                }
                if (!flag) // if not hit, there is a miss
                {
                    int valid_flag = 0;
                    a.misses++;
                    int i;
                    for (i = 0; i < E; i++) {

                        if (cache[(set_index * E) + i].valid_bit ==
                            0) // checking for initial condition
                        {
                            valid_flag = 1;
                            cache[(set_index * E) + i].valid_bit = 1;
                            int round_robin =
                                cache[(set_index * E) + i]
                                    .count; // storing current count
                            for (int j = 0; j < E;
                                 j++) // round robin algorithm for finding least
                                      // recently used
                            {
                                if (cache[(set_index * E) + j].valid_bit == 1 &&
                                    cache[(set_index * E) + j].count <=
                                        round_robin &&
                                    j != i) {
                                    cache[(set_index * E) + j].count += 1;
                                }
                            }
                            cache[(set_index * E) + i].count =
                                0; // updating count to be 0
                            cache[(set_index * E) + i].tag = tag;
                            cache[(set_index * E) + i].dirty_bit =
                                1; // setting dirty bit because store has been
                                   // called
                            break;
                        }
                    }
                    if (!valid_flag) // when all the blocks are filled,
                                     // evictions need to take place
                    {
                        int final_count = cache[(set_index * E)].count;
                        int j;
                        for (j = 0; j < E; j++) {
                            if (cache[(set_index * E) + j].count >
                                    final_count &&
                                cache[(set_index * E) + j].valid_bit ==
                                    1) // finding the least recently used block
                            {
                                final_count = cache[(set_index * E) + j].count;
                                replacement = j;
                            }
                        }
                        cache[(set_index * E) + replacement].tag = tag;
                        if (cache[(set_index * E) + replacement].dirty_bit)
                            a.dirty_evictions +=
                                B; // since store has been called increment
                                   // dirty evictions
                        cache[(set_index * E) + replacement].dirty_bit =
                            1; // set dirty bit because method='S'
                        int round_robin =
                            cache[(set_index * E) + replacement].count;
                        for (int k = 0; k < E;
                             k++) // least recently used algorithm using round
                                  // robin
                        {
                            if (cache[(set_index * E) + k].valid_bit == 1 &&
                                cache[(set_index * E) + k].count <=
                                    round_robin &&
                                k != replacement) {
                                cache[(set_index * E) + k].count += 1;
                            }
                        }
                        cache[(set_index * E) + replacement].count = 0;
                        a.evictions++; // dirty evictions are also evictions
                                       // hence incrementing
                    }
                }
            }
        }
        fclose(fptr);
    }
    for (i = 0; i < S; i++) // finding total number of dirty bytes by traversing
                            // through the cache
    {
        for (j = 0; j < E; j++) {
            if (cache[i * E + j].dirty_bit == 1) {
                a.dirty_bytes += B;
            }
        }
    }
    printSummary(&a);
    free(cache);
    return 0;
}