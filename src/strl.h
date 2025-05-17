/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Header for string literal parsing
 */
#ifndef STRL_H_
#define STRL_H_

#include "util.h"

/* Parses a string literal `src` into an actual string `dst`,
 * given size of the string literal `size`. Assumes that
 * dst is sized enough to fit the output of `src`.
 * Returns zero on failure, else size of `dst`. */
size_t strl_parse(const uint8_t *src, uint8_t *dst, size_t size);

#endif /* STRL_H_ */
