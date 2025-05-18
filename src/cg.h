/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Code generator header file
 */
#ifndef CG_H_
#define CG_H_

#include "util.h"
#include "lex.h"

/* emit code to file `to` */
int cg_emit(lex_t *lex, FILE *to);

#endif /* CG_H_ */
