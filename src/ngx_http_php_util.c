/*
==============================================================================
Copyright (c) 2016-2019, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#include "ngx_http_php_module.h"
#include "ngx_http_php_util.h"

size_t
occurrences(const char *needle, const char *haystack) {
  if (NULL == needle || NULL == haystack) return -1;

  char *pos = (char *)haystack;
  size_t i = 0;
  size_t l = strlen(needle);
  if (l == 0) return 0;

  while ((pos = strstr(pos, needle))) {
    pos += l;
    i++;
  }

  return i;
}

char *
str_replace(const char *str, const char *sub, const char *replace) {
  char *pos = (char *) str;
  int count = occurrences(sub, str);

  if (0 >= count) return strdup(str);

  int size = (
        strlen(str)
      - (strlen(sub) * count)
      + strlen(replace) * count
    ) + 1;

  char *result = (char *) malloc(size);
  if (NULL == result) return NULL;
  memset(result, '\0', size);
  char *current;
  while ((current = strstr(pos, sub))) {
    int len = current - pos;
    strncat(result, pos, len);
    strncat(result, replace, strlen(replace));
    pos = current + strlen(sub);
  }

  if (pos != (str + strlen(str))) {
    strncat(result, pos, (str - pos));
  }

  return result;
}

