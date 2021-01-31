/*
 * Copyright (c) 1995 Frank Warmerdam
 *
 * This code is in the public domain.
 *
 */

#include "shapefil.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)

{
  DBFHandle hDBF;
  int *panWidth, i, iRecord;
  char szFormat[32], szField[1024];
  char cTitle[32], nTitle[32];
  int nWidth, nDecimals;
  int cnWidth, cnDecimals;
  DBFHandle cDBF;
  DBFFieldType hType, cType;
  int ci, ciRecord;
  char tfile[160];
  int hflds, j, cflds;
  int verbose = 0;
  int force = 0;
  int mismatch = 0;
  int matches = 0;
  char fld_m[256];
  int shift = 0;
  char type_names[4][15] = {"integer", "string", "double", "double"};

  if (argc < 3) {
    printf("dbfcat [-v] [-f] from_DBFfile to_DBFfile\n");
    exit(1);
  }

  if (strcmp("-v", argv[1]) == 0) {
    shift = 1;
    verbose = 1;
  }
  if (strcmp("-f", argv[1 + shift]) == 0) {
    shift++;
    force = 1;
  }
  if (strcmp("-v", argv[1 + shift]) == 0) {
    shift++;
    verbose = 1;
  }
  strcpy(tfile, argv[1 + shift]);
  strcat(tfile, ".dbf");
  hDBF = DBFOpen(tfile, "rb");
  if (hDBF == NULL) {
    printf("DBFOpen(%s.dbf,\"r\") failed for From_DBF.\n", tfile);
    exit(2);
  }

  strcpy(tfile, argv[2 + shift]);
  strcat(tfile, ".dbf");

  cDBF = DBFOpen(tfile, "rb+");
  if (cDBF == NULL) {
    printf("DBFOpen(%s.dbf,\"rb+\") failed for To_DBF.\n", tfile);
    exit(2);
  }

  if (DBFGetFieldCount(hDBF) == 0) {
    printf("There are no fields in this table!\n");
    exit(3);
  }

  hflds = DBFGetFieldCount(hDBF);
  cflds = DBFGetFieldCount(cDBF);

  matches = 0;
  for (i = 0; i < hflds; i++) {
    char szTitle[18];
    char cname[18];
    int j;
    hType = DBFGetFieldInfo(hDBF, i, szTitle, &nWidth, &nDecimals);
    fld_m[i] = -1;
    for (j = 0; j < cflds; j++) {
      cType = DBFGetFieldInfo(cDBF, j, cname, &cnWidth, &cnDecimals);
      if (strcmp(cname, szTitle) == 0) {
        if (hType != cType) {
          printf("Incompatible fields %s(%s) != %s(%s),\n", type_names[hType],
                 nTitle, type_names[cType], cTitle);
          mismatch = 1;
        }
        fld_m[i] = j;
        if (verbose) {
          printf("%s  %s(%d,%d) <- %s  %s(%d,%d)\n", cname, type_names[cType],
                 cnWidth, cnDecimals, szTitle, type_names[hType], nWidth,
                 nDecimals);
        }
        j = cflds;
        matches = 1;
      }
    }
  }

  if ((matches == 0) && !force) {
    printf("ERROR: No field names match for tables, cannot proceed\n   use -f "
           "to force processing using blank records\n");
    exit(-1);
  }
  if (mismatch && !force) {
    printf("ERROR: field type mismatch cannot proceed\n    use -f to force "
           "processing using attempted conversions\n");
    exit(-1);
  }

  for (iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++) {
    ciRecord = DBFGetRecordCount(cDBF);
    for (i = 0; i < hflds; i++) {
      double cf;
      ci = fld_m[i];
      if (ci != -1) {
        cType = DBFGetFieldInfo(cDBF, ci, cTitle, &cnWidth, &cnDecimals);
        hType = DBFGetFieldInfo(hDBF, i, nTitle, &nWidth, &nDecimals);

        switch (cType) {
        case FTString:
        case FTLogical:
        case FTDate:
          DBFWriteStringAttribute(
              cDBF, ciRecord, ci,
              (char *)DBFReadStringAttribute(hDBF, iRecord, i));
          break;

        case FTInteger:
          DBFWriteIntegerAttribute(
              cDBF, ciRecord, ci,
              (int)DBFReadIntegerAttribute(hDBF, iRecord, i));
          break;

        case FTDouble:
          /*	        cf = DBFReadDoubleAttribute( hDBF, iRecord, i );
                          printf ("%s <-  %s (%f)\n", cTitle, nTitle, cf);
          */
          DBFWriteDoubleAttribute(
              cDBF, ciRecord, ci,
              (double)DBFReadDoubleAttribute(hDBF, iRecord, i));
          break;

        case FTInvalid:
          break;
        }
      }
    } /* fields names match */
  }

  if (verbose) {
    printf(" %d records appended \n\n", iRecord);
  }
  DBFClose(hDBF);
  DBFClose(cDBF);

  return (0);
}
