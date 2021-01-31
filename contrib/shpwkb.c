/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * this code is based in part on the earlier work of Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * shpwkb.c  - test WKB binary Input / Output
 */

#include "shapefil.h"
#include "shpgeo.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("shpwkb shp_file wkb_file\n");
    exit(1);
  }

  SHPHandle old_SHP = SHPOpen(argv[1], "rb");
  if (old_SHP == NULL) {
    printf("Unable to open old shp file for:%s\n", argv[1]);
    exit(1);
  }

  DBFHandle old_DBF = DBFOpen(argv[1], "rb");
  if (old_DBF == NULL) {
    SHPClose(old_SHP);
    printf("Unable to open old dbf file for:%s\n", argv[1]);
    exit(1);
  }

  FILE *wkb_file = fopen(argv[2], "wb");
  WKBStreamObj *wkbObj = calloc(3, sizeof(int));

  int nEntities;
  int nShapeType;
  SHPGetInfo(old_SHP, &nEntities, &nShapeType, NULL, NULL);

  const int byRing = 0; // TODO(schwehr): Bug?  Always false.

  for (int i = 0; i < nEntities; i++) {
    SHPObject *psCShape = SHPReadObject(old_SHP, i);

    if (byRing == 1) {
      for (int ring = (psCShape->nParts - 1); ring >= 0; ring--) {
        int rStart = psCShape->panPartStart[ring];
        int numVtx;
        if (ring == (psCShape->nParts - 1)) {
          numVtx = psCShape->nVertices - rStart;
        } else {
          numVtx = psCShape->panPartStart[ring + 1] - rStart;
        }

        printf("(shpdata) Ring(%d) (%d for %d) \n", ring, rStart, numVtx);
        SHPObject *psO = SHPClone(psCShape, ring, ring + 1);

        SHPDestroyObject(psO);
        printf("(shpdata) End Ring \n");
      } /* (ring) [0,nParts  */

    } /* by ring   */

    printf("gonna build a wkb \n");
    // int res =
    SHPWriteOGisWKB(wkbObj, psCShape);
    printf("gonna write a wkb that is %d bytes long \n", wkbObj->StreamPos);
    fwrite((void *)wkbObj->wStream, 1, wkbObj->StreamPos, wkb_file);
  }

  free(wkbObj);
  SHPClose(old_SHP);
  DBFClose(old_DBF);
  if (wkb_file)
    fclose(wkb_file);

  printf("\n");
}
