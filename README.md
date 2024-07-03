# libarg

libarg is a lightweight and flexible library for argument parsing in C, designed to provide an alternative to the GNU `argp` library.
It offers a straightforward API for defining and parsing command-line arguments, supporting both short and long options, as well as positional arguments.
> Less globals more callbacks.

## Features

- **Simple API**: Easily define command-line arguments with a clear and concise structure.
- **Flexible Parsing**: Supports short options, long options, positional arguments.
- **Custom Callbacks**: Associate custom parsing functions with specific arguments.
- **Application Metadata**: Include version, author, and other metadata about the application.

## Installation

### Build and Install

To build and install the `libarg` library, use the provided `Makefile`.
Optionally, specify a `BUILD_DIR` environment variable to place build artifacts in a separate directory.

```sh
make
sudo make install
```

To specify a custom build directory:

```sh
make BUILD_DIR=build
sudo make BUILD_DIR=build install
```

### Usage

Include the `arg.h` header in your project and link against `libarg`:

```c
#include <arg.h>
```

Compile your project with the `pkg-config` tool to include the necessary flags:

```sh
gcc $(pkg-config --cflags arg) -o myprogram myprogram.c $(pkg-config --libs arg)
```

## Example

Below is a simple example demonstrating how to use `libarg` in an application:

```c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <arg.h>

#define container_of(ptr, type, member)                                \
	({                                                             \
		const __typeof__(((type *)0)->member) *__mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member));     \
	})

struct cfg {
	struct arg_parser parser;
	char date[11];
	int flag;
};

static int get_current_date(char *buffer, size_t buffer_size)
{
	struct tm *tm_info = NULL;
	time_t t;

	if (!buffer)
		return -EINVAL;

	t = time(NULL);
	tm_info = localtime(&t);

	strftime(buffer, buffer_size, "%Y-%m-%d", tm_info);

	return 0;
}

static int parse_flag(const struct arg_parser *parser)
{
	struct cfg *cfg = NULL;

	cfg = container_of(parser, struct cfg, parser);

	cfg->flag = 1;

	return 0;
}

static int parse_date(const struct arg_parser *parser)
{
	struct cfg *cfg = NULL;

	cfg = container_of(parser, struct cfg, parser);

	strncpy(cfg->date, parser->val, sizeof(cfg->date) - 1);

	return 0;
}

/* Define arguments */
static const struct arg appargs[] = {
	{
		.opt = {
			.short_opt = "d",
			.long_opt = "date",
			.value_name = "DATE",
			.default_value = "today",
		},
		.descr = "The date in YYYY-MM-DD",
		.parse = parse_date,
	},
	{
		.opt = {
			.short_opt = "f",
			.long_opt = "flag",
		},
		.descr = "Some boolean flag",
		.parse = parse_flag,
	},
	{ .eol = 1 },
};

int main(int argc, char *argv[])
{
	struct cfg cfg = { 0 };
	int res;

	/* Set up application metadata */
	cfg.parser.info.description = "A simple example program.";
	cfg.parser.info.url = "https://example.example.com";
	cfg.parser.info.email = "john.doe@example.com";
	cfg.parser.info.author = "John Doe";
	cfg.parser.info.program = "example";
	cfg.parser.info.license = "MIT";
	cfg.parser.info.year = "2024";

 	/* Set up version info */
	cfg.parser.info.version.major = 1;
	cfg.parser.info.version.minor = 0;
	cfg.parser.info.version.patch = 12;

	/* Set up git info */
	cfg.parser.info.git.url = "https://example.github.com";
	cfg.parser.info.git.sha = "f548ea62";

	/* Initialize parser */
	cfg.parser.args = appargs;
	cfg.parser.argc = argc;
	cfg.parser.argv = argv;

	/* Setup default state */
	get_current_date(cfg.date, sizeof(cfg.date));

	/* Parse arguments */
	res = arg_parse(&cfg.parser);
	if (res > 0)
		return EXIT_SUCCESS;
	if (res < 0)
		return EXIT_FAILURE;

	printf("Date: %s\n", cfg.date);
	printf("Flag: %s\n", cfg.flag ? "true" : "false");

	return EXIT_SUCCESS;
}
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Contributing

Contributions are welcome. Please submit pull requests or open issues to discuss any changes.
