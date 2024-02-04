//
// Created by MCS51_M2 on 2023/11/19.
//

#ifndef SIMPLE_DATABASE_INPUTS_H
#define SIMPLE_DATABASE_INPUTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "main.h"
#include "prompt.h"

void read_input(prompt_buf_t *prompt_buf);
prompt_buf_t* get_prompt_raw(void);

#ifdef __cplusplus
}
#endif

#endif //SIMPLE_DATABASE_INPUTS_H
