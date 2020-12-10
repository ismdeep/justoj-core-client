//
// Created by Del Cooper on 2020/12/10.
//

#ifndef JUSTOJ_CORE_VERSION_H
#define JUSTOJ_CORE_VERSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ismdeep-c-utils/time.h>

char *get_version() {
    char *ans = (char *) malloc(sizeof(char) * 65535);
    sprintf(ans, "version");

#ifdef BRANCH_NAME
    strcat(ans, "-");
    strcat(ans, BRANCH_NAME);
#endif

#ifdef RELEASE_DATE
    strcat(ans, "-");
    strcat(ans, unix_time_to_date(RELEASE_DATE, "[%Y-%m-%d %H:%M:%S]"));
#endif

#ifdef COMMIT_HASH
    strcat(ans, "-");
    strcat(ans, COMMIT_HASH);
#endif

    return ans;
}

#endif //JUSTOJ_CORE_VERSION_H
