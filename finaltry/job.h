#ifndef JOB_H
#define JOB_H

#include "parser.h"

void execute_job(struct Job *job);
void free_job(struct Job *job);

#endif
