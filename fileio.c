// Functions to write an index to file and read it back
// Does not actually store the original sequence; that seems pointless

#include "seqindex.h"
#include <stdio.h>
#include <stdlib.h>

void write_index(const fm_index *fmi, FILE *f) {
  // Writes the FM-index to file... well, the parts that take
  // time to actually generate.
  fwrite(&fmi->len, sizeof(fmi->len), 1, f);
  fwrite(fmi->C, sizeof(*fmi->C), 5, f);
  fwrite(&fmi->endloc, sizeof(fmi->endloc), 1, f);
  fwrite(fmi->idxs, sizeof(*fmi->idxs), (1+(fmi->len)/32), f);
  fwrite(fmi->bwt, 1, (fmi->len+3)/4, f);
  // C standard guarantees sizeof(char) to be 1
  return;
}

// Reads the BWT from file and reconstructs the FM-index
// Returns a newly allocated FM-index.
// Doesn't check for running out of memory; expect segfaults if that happens.
// If it returns NULL, reading from file failed
fm_index *read_index(FILE *f) {
  long long sz;
  int err = 0;

  fm_index *fmi = calloc(1, sizeof(fm_index));
  sz = fread(&fmi->len, sizeof(fmi->len), 1, f);
  if (sz != 1) {
    fprintf(stderr, "Error reading index from file\n");
    err = 1;
  }
  sz = fread(fmi->C, sizeof(*fmi->C), 5, f);
  if (sz != 5) {
    fprintf(stderr, "Error reading index from file\n");
    err = 1;
  }
  sz = fread(&fmi->endloc, sizeof(fmi->endloc), 1, f);
  if (sz != 1) {
    fprintf(stderr, "Error reading index from file\n");
    err = 1;
  }
  fmi->idxs = malloc((1+(fmi->len)/32) * sizeof(long long));
  sz = fread(fmi->idxs, sizeof(long long), 1+(fmi->len)/32, f);
  if (sz != 1 + (fmi->len)/32) {
    fprintf(stderr, "Error reading index from file\n");
    err = 1;
  }
  fmi->bwt = malloc((fmi->len+3)/4);
  sz = fread(fmi->bwt, 1, (fmi->len+3)/4, f);
  if (sz != (fmi->len+3)/4) {
    fprintf(stderr, "Error reading index from file\n");
    err = 1;
  }

  if (err) {
    destroy_fmi(fmi);
    return NULL;
  }
  
  fmi->lookup = lookup_table();
  fmi->rank_index = seq_index(fmi->bwt, fmi->len, 16, fmi->lookup);
  return fmi;
}
