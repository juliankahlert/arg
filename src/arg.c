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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "arg.h"

static size_t longest_opt(const struct arg *args)
{
	size_t len;
	size_t res;
	size_t i;

	res = strlen("version");
	for (i = 0; !args[i].eol; i++) {
		if (!args[i].opt.long_opt)
			continue;

		len = strlen(args[i].opt.long_opt);
		res = (len > res) ? len : res;
	}

	return res;
}

static size_t longest_var(const struct arg *args)
{
	size_t len;
	size_t res;
	size_t i;

	res = 0;
	for (i = 0; !args[i].eol; i++) {
		if (!args[i].opt.value_name)
			continue;

		len = strlen(args[i].opt.value_name);
		res = (len > res) ? len : res;
	}

	return res;
}

static int is_arg(const char *sopt, const char *lopt, const char *arg)
{
	int is_l;
	int is_s;

	if (!arg)
		return 0;

	is_l = 0;
	if (strncmp("--", arg, 2) == 0)
		is_l = 1;

	is_s = 0;
	if (!is_l && strncmp("-", arg, 1) == 0)
		is_s = 1;

	if (!is_s && !is_l)
		return 0;

	if (is_l && lopt)
		return strcmp(arg + 2, lopt) == 0;

	if (is_s && sopt)
		return strncmp(arg + 1, sopt, 1) == 0;

	return 0;
}

static int parse_arg(struct arg_parser *parser)
{
	int res;

	if (!parser)
		return -EINVAL;

	if (!parser->arg)
		return -EINVAL;

	if (parser->arg->opt.value_name && parser->argc <= parser->i + 1) {
		if (parser->arg->opt.long_opt)
			fprintf(stderr, "Error: Option <%s> needs a value.\n\n",
				parser->arg->opt.long_opt);
		else if (parser->arg->opt.short_opt)
			fprintf(stderr, "Error: Option <%s> needs a value.\n\n",
				parser->arg->opt.short_opt);

		if (parser->arg->opt.long_opt || parser->arg->opt.short_opt)
			return 1;
	}

	if (parser->arg->opt.short_opt || parser->arg->opt.long_opt) {
		parser->key = parser->argv[parser->i];
		parser->val = NULL;

		if (parser->arg->opt.value_name) {
			parser->i++;
			parser->val = parser->argv[parser->i];
		}
	} else {
		parser->key = NULL;
		parser->val = parser->argv[parser->i];
	}

	res = 0;

	/* if there is a dedicaded parser for this argument use it*/
	if (parser->arg && parser->arg->parse)
		res = parser->arg->parse(parser);
	else if (parser->parse)
		res = parser->parse(parser);

	parser->key = NULL;
	parser->val = NULL;

	return res;
}

static const struct arg *find_positional(size_t idx, struct arg_parser *parser)
{
	const struct arg *args = NULL;
	size_t cnt;
	size_t i;

	if (!parser)
		return NULL;

	args = parser->args;
	if (!args)
		return NULL;

	for (i = 0, cnt = 0; !args[i].eol; i++) {
		if (args[i].opt.long_opt || args[i].opt.short_opt)
			continue;

		if (cnt == idx)
			return &args[i];

		cnt++;
	}

	return NULL;
}

static size_t print_postional(const struct arg_parser *parser)
{
	const struct arg *args = NULL;
	size_t cnt;
	size_t i;

	if (!parser)
		return 0;

	args = parser->args;
	if (!args)
		return 0;

	for (cnt = 0, i = 0; !args[i].eol; i++) {
		if (args[i].opt.long_opt || args[i].opt.short_opt)
			continue;

		cnt++;
		if (args[i].opt.value_name)
			fprintf(stderr, "%s ", args[i].opt.value_name);
	}

	return cnt;
}

static size_t count_postional(const struct arg_parser *parser)
{
	const struct arg *args = NULL;
	size_t cnt;
	size_t i;

	if (!parser)
		return 0;

	args = parser->args;
	if (!args)
		return 0;

	for (cnt = 0, i = 0; !args[i].eol; i++) {
		if (args[i].opt.long_opt || args[i].opt.short_opt)
			continue;

		cnt++;
	}

	return cnt;
}

static void print_short_opt(const struct arg *arg)
{
	int has_l;

	if (!arg)
		return;

	has_l = arg->opt.long_opt != NULL;

	if (arg->opt.short_opt)
		fprintf(stderr, "  -%s%s ", arg->opt.short_opt,
			(has_l) ? "," : "");
	else
		fprintf(stderr, "      ");
}

static void print_long_opt(const struct arg *arg, int pad)
{
	int has_l;

	if (!arg)
		return;

	has_l = arg->opt.long_opt != NULL;

	if (arg->opt.long_opt)
		fprintf(stderr, "--%-*s ", pad, arg->opt.long_opt);
	else if (pad)
		fprintf(stderr, "  %-*s%s ", pad, "", (!has_l) ? " " : "");
}

static void print_value_name(const struct arg *arg, int pad)
{
	if (!arg)
		return;

	if (arg->opt.value_name)
		fprintf(stderr, "%-*s ", pad, arg->opt.value_name);
	else if (pad)
		fprintf(stderr, "%-*s ", pad, "");
}

static void print_descr(const struct arg *arg)
{
	if (!arg)
		return;

	if (!arg->descr)
		return;

	fprintf(stderr, "%s.", arg->descr);
}

static void print_accepted_values(const struct arg *arg, int pad)
{
	if (!arg)
		return;

	if (!arg->opt.accepted_values)
		return;

	if (strncmp(arg->opt.accepted_values, "type::", 6) == 0)
		return;

	fprintf(stderr, "\n        %-*s  Accept: %s", pad, "",
		arg->opt.accepted_values);
}

static void print_default_value(const struct arg *arg, int pad)
{
	if (!arg)
		return;

	if (!arg->opt.default_value)
		return;

	fprintf(stderr, "\n        %-*s  Default: %s", pad, "",
		arg->opt.default_value);
}

static int print_copyright(const struct arg_parser *parser)
{
	const struct arg_app_info *info;

	if (!parser)
		return 0;

	info = &parser->info;

	if (!info->author && !info->license && !info->year && !info->email)
		return 0;

	fprintf(stderr, "\n");

	if (info->license)
		fprintf(stderr, "%s License Copyright (c)", info->license);

	if (info->year) {
		if (info->license)
			fprintf(stderr, " ");

		fprintf(stderr, "%s", info->year);
	}

	if (info->author) {
		if (info->license || info->year)
			fprintf(stderr, " ");

		fprintf(stderr, "%s", info->author);
	}

	if (info->email) {
		if (info->license || info->year || info->author)
			fprintf(stderr, " ");

		fprintf(stderr, "<%s>", info->email);
	}

	fprintf(stderr, ".\n");

	return 1;
}

static int print_url(const struct arg_parser *parser)
{
	const struct arg_app_info *info;

	if (!parser)
		return 0;

	info = &parser->info;

	if (!info->url)
		return 0;

	fprintf(stderr, "Visit %s for more details.\n", info->url);

	return 1;
}

static int print_description(const struct arg_parser *parser)
{
	const struct arg_app_info *info;

	if (!parser)
		return 0;

	info = &parser->info;

	if (!info->description)
		return 0;

	fprintf(stderr, "\n%s\n", info->description);

	return 1;
}

static int print_version(const struct arg_parser *parser)
{
	const struct arg_app_info *info;

	if (!parser)
		return 0;

	info = &parser->info;

	/* print this to stdout to make it easily usable */
	fprintf(stdout, "%zu.%zu", info->version.major, info->version.minor);

	if (info->version.patch)
		fprintf(stdout, ".%zu", info->version.patch);

	fprintf(stdout, "\n");

	return 1;
}

static void print_options(const struct arg *args, int padl, int padv)
{
	size_t i;

	if (!args)
		return;

	fprintf(stderr, "\nOptions:\n");

	for (i = 0; !args[i].eol; i++) {
		if (!args[i].opt.long_opt && !args[i].opt.short_opt)
			continue;

		print_short_opt(&args[i]);
		print_long_opt(&args[i], padl);
		print_value_name(&args[i], padv);
		print_descr(&args[i]);
		print_accepted_values(&args[i], padl + padv);
		print_default_value(&args[i], padl + padv);
		fprintf(stderr, "\n");
	}
}

static void print_positionals(const struct arg *args, int padl, int padv)
{
	size_t i;

	if (!args)
		return;

	fprintf(stderr, "\nPositionals:\n");

	for (i = 0; !args[i].eol; i++) {
		if (args[i].opt.long_opt || args[i].opt.short_opt)
			continue;

		fprintf(stderr, "         %-*s", padl, "");
		print_value_name(&args[i], padv);
		print_descr(&args[i]);
		print_accepted_values(&args[i], padl + padv);
		print_default_value(&args[i], padl + padv);
		fprintf(stderr, "\n");
	}
}

static int handle_buildin(const struct arg_parser *parser)
{
	if (!parser)
		return -EINVAL;

	if (is_arg("h", "help", parser->argv[parser->i])) {
		arg_usage(parser);
		return 1;
	}

	if (is_arg("V", "version", parser->argv[parser->i])) {
		print_version(parser);
		return 1;
	}

	return 0;
}

void arg_usage(const struct arg_parser *parser)
{
	const struct arg *args = NULL;
	int has_p;
	int padl;
	int padv;

	if (!parser)
		return;

	if (!parser->args)
		return;

	args = parser->args;
	padl = longest_opt(args);
	padv = longest_var(args);

	fprintf(stderr, "Usage: %s [options] ", parser->info.program);
	has_p = !!print_postional(parser);
	fprintf(stderr, "\n");

	print_description(parser);

	print_options(args, padl, padv);
	fprintf(stderr, "  -h, --%-*s %-*s Prints this message.\n", padl,
		"help", padv, "");
	fprintf(stderr, "  -V, --%-*s %-*s Prints the version.\n", padl,
		"version", padv, "");

	if (has_p)
		print_positionals(args, padl, padv);

	if (!print_copyright(parser))
		fprintf(stderr, "\n");

	print_url(parser);
}

int arg_parse(struct arg_parser *parser)
{
	int parsed;
	size_t i;
	size_t p;

	if (!parser)
		return -EINVAL;

	if (!parser->argc)
		return 0;

	if (!parser->args || !parser->argv)
		return -EINVAL;

	p = count_postional(parser);
	/*
	for (p = 0, i = 0; !parser->args[i].eol; i++) {
		if (parser->args[i].opt.long_opt ||
		    parser->args[i].opt.short_opt)
			continue;

		p++;
	}
*/
	parser->positional = p;

	for (p = 0, parser->i = 1; parser->i < parser->argc; parser->i++) {
		parsed = 0;

		for (i = 0; !parsed && !parser->args[i].eol; i++) {
			parser->arg = &parser->args[i];

			if (handle_buildin(parser))
				return 1;

			if (!is_arg(parser->arg->opt.short_opt,
				    parser->arg->opt.long_opt,
				    parser->argv[parser->i]))
				continue;

			if (parse_arg(parser)) {
				arg_usage(parser);
				return -1;
			} else {
				parsed = 1;
			}
		}

		if (!parsed && parser->positional > 0) {
			parser->positional--;
			parser->arg = find_positional(p++, parser);
			if (parse_arg(parser)) {
				arg_usage(parser);
				return -1;
			}

			continue;
		}

		if (!parsed) {
			fprintf(stderr, "Error: Option <%s> unknown!\n\n",
				parser->argv[parser->i]);
			arg_usage(parser);
			return -1;
		}
	}

	return 0;
}
