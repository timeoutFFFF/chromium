/*
 * Copyright (C) 2008-2009  Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Richard Li <RichardZ.Li@amd.com>, <richardradeon@gmail.com>
 */

#ifndef _EVERGREEN_STATE_H_
#define _EVERGREEN_STATE_H_

#include "main/mtypes.h"

#include "r600_context.h"

extern void evergreenUpdateStateParameters(GLcontext * ctx, GLuint new_state);
extern void evergreenUpdateShaders(GLcontext * ctx);
extern void evergreenUpdateShaderStates(GLcontext * ctx);

extern void evergreeUpdateShaders(GLcontext * ctx);

extern void evergreenUpdateViewportOffset(GLcontext * ctx);

extern void evergreenInitState(GLcontext * ctx);
extern void evergreenInitStateFuncs (radeonContextPtr radeon, struct dd_function_table *functions);

extern void evergreenSetScissor(context_t *context);

#endif	/* _EVERGREEN_STATE_H_ */
