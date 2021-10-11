#include <assert.h>
#include <stdio.h>

unsigned long long _off;
#define MAX_SEQ_LEN 4096
    
int _getc(FILE *f) {
  int c = fgetc(f);
  if (c == EOF)
    return EOF;
  _off++;
  return c;
}


int readSeqHdr(FILE *fbin) {
  int c, len;

  if ((c = _getc(fbin)) == EOF)
    return EOF;
  // Sequence
  if (c != 0x30)
    return 0;
  if ((c = _getc(fbin)) == EOF)
    return EOF;
  if (c != 0x82)
    return 0;
  // 2-byte encoded ASN.1 length
  if ((c = _getc(fbin)) == EOF)
    return EOF;
  len = c << 8;
  if ((c = _getc(fbin)) == EOF)
    return EOF;
  len |= c;
  if (len <= 0xFF || len > MAX_SEQ_LEN)
    // invalid encoding
    return 0;
  return len;
}

void writeSeqHdr(FILE *fout, int len) {
  assert(len > 0 && len < MAX_SEQ_LEN);

  fputc(0x30, fout);
  fputc(0x82, fout);
  fputc(len >> 8, fout);
  fputc(len & 0xFF, fout);
}

int writeOut(FILE *fbin, int len1, int len2, int index) {
  int i, c, rdsz = len1 - 4;
  char name[16];
  FILE *fout;
  
  sprintf(name, "asn%d.der", index);
  printf("Writing to asn%d.der starting at offset 0x%llx, %d bytes\n", index, _off , len1 + 4);
  fout = fopen(name, "wb");
  if (!fout) {
    perror("failed to open out file\n");
    return EOF;
  }
  
  writeSeqHdr(fout, len1);
  writeSeqHdr(fout, len2);

  for (i = 0; i < rdsz; ++i) {
    if ((c = _getc(fbin)) == EOF) {
      perror("unexpected EOF\n");
      fclose(fout);
      return EOF;
    }
    fputc(c, fout);
  }
  fclose(fout);
  return 0;
}

void fetch(FILE *fbin) {
  int index = 0;

  _off = 0;
  while (1) {
    int len1 = readSeqHdr(fbin);
    if (len1 == EOF)
      break;
    if (len1) {
      int len2 = readSeqHdr(fbin);
      if (len2 == EOF)
        break;
      if (len2 && len2 < len1)
        if (writeOut(fbin, len1, len2, index++) == EOF)
          break;      
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *fbin;
  if (argc != 2) {
    printf("usage: %s <file>\n", argv[0]);
    return 1;
  }

  fbin = fopen(argv[1], "rb");
  if (!fbin) {
    perror("Failed to open file");
    return 1;
  }

  fetch(fbin);
  fclose(fbin);
  return 0;
}
