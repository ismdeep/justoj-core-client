//
// Created by L. Jiang on 2020/12/26.
//

#ifndef JUSTOJ_CORE_STR_UTILS_H
#define JUSTOJ_CORE_STR_UTILS_H

char *create_str(size_t size) {
    char *str = (char *) malloc(sizeof(char) * size);
    memset(str, 0, sizeof(char) * size);
    return str;
}

#endif //JUSTOJ_CORE_STR_UTILS_H
