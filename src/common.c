/*
 * Copyright (C) 2018,2019 Cybertrust Japan Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <string.h>
#include "common.h"

void chomp(char *string){
    int length;

    length = strlen(string);
    if(length > 0 && string[length - 1] == '\n'){
        string[length - 1] = '\0';
    }
}

/*
 * Split string by delimiters to 2 strings.
 * @param (*src)        source string
 * @param (*dels)       delimiters
 * @param (*dst1)       destination of string1
 * @param (size1)       size of string1
 * @param (*dst1)       destination of string2
 * @param (size2)       size of string2
 * @return              return 0 if successfully split string. otherwise, return negative number if overflow occurred at dst1 or dst2. in case of dst1, return -1. in case of dst2, return -2.
*/
int strsplit (const char *src, const char *dels, char *dst1, unsigned int size1, char *dst2, unsigned int size2) {
	char *p, *d, *l;

	for (d = (char *)dels; *d != '\0'; d++) {
		if ( (p = strchr (src, *d)) != NULL ) {
			/* calculate the last adddress */
			l = (char *)src + strlen (src) + 1;

			/* check buffer overflow */
			if (p - src > size1 - 1)
				return -1;
			strncpy (dst1, src, p - src);
			dst1[p-src] = '\0';

			/* skip continuous delimiters */
			while ( strchr (p, *d) != NULL ) p++;

			/* check buffer overflow */
			if (l - p > size2 - 1)
				return -2;
			strncpy (dst2, p, l - p);
			dst2[l-p] = '\0';

			return 0;
		}
	}
	return -1;
}
