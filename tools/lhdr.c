/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Generates a license header for MPLv2 to be used in source code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Change this if you use this program for yourself! */
static const char *default_author = "therealblue24";

int main(int argc, char *argv[])
{
    long copyright = -1;
    char *authors = NULL;
    char *desc = NULL;
    int argc2 = 1;
    while(argc2 < argc) {
        const char *arg = argv[argc2];

        if(strcmp(arg, "-c") == 0) {
            copyright = strtol(argv[++argc2], NULL, 10);
        } else if(strcmp(arg, "-a") == 0) {
            authors = argv[++argc2];
        } else {
            desc = argv[argc2];
        }

        argc2++;
    }

    /* User didn't supply copyright year */
    if(copyright == -1) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        copyright = tm->tm_year + 1900;
    }

    /* User didn't supply authors */
    if(authors == NULL) {
        /* just use the default */
        authors = (char *)default_author;
    }

    printf("/*\n");
    printf(" * Copyright (C) %ld %s.\n", copyright, authors);
    printf(" *\n");
    printf(
        " * This Source Code Form is subject to the terms of the Mozilla Public\n");
    printf(
        " * License, v. 2.0. If a copy of the MPL was not distributed with this\n");
    printf(" * file, You can obtain one at http://mozilla.org/MPL/2.0/.\n");
    if(desc) {
        printf(" *\n");
        printf(" * %s\n", desc);
    }
    printf(" */\n");
}
