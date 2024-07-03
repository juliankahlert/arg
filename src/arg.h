/*
 * MIT License
 *
 * Copyright (c) 2024 Julian Kahlert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _ARG_H_
#define _ARG_H_

#include <stddef.h>

struct arg_opt {
	const char *accepted_values;
	const char *default_value;
	const char *value_name;
	const char *short_opt;
	const char *long_opt;
};

struct arg_parser;
struct arg {
	int (*parse)(const struct arg_parser *);
	struct arg_opt opt;
	const char *descr;
	int eol;
};

struct args_app_info_git {
	const char *url;
	const char *sha;
};

struct args_app_info_version {
	size_t major;
	size_t minor;
	size_t patch;
};

struct arg_app_info {
	struct args_app_info_version version;
	struct args_app_info_git git;
	const char *description;
	const char *license;
	const char *program;
	const char *commit;
	const char *author;
	const char *email;
	const char *year;
	const char *url;
};

struct arg_parser {
	int (*parse)(const struct arg_parser *);
	struct arg_app_info info;
	const struct arg *args;
	const struct arg *arg;
	size_t positional;
	const char *val;
	const char *key;
	char **argv;
	size_t argc;
	size_t i;
};

void arg_usage(const struct arg_parser *parser);
int arg_parse(struct arg_parser *parser);

#endif
