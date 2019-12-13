#include <sys/inotify.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#ifdef DEBUG

#define DEBUG_CODE "\x1b[36m"
#define RESET_COLOR "\x1b[0m"

#endif

struct Arguments {
	char* output;
	char* input;
};

struct Arguments args;

/* Takes program arguments and 
 * 	parses the important information, 
 * 	placing it in the global args variable.
 *
 * Returns 0 on success. */
int parse_args(int argc, char** argv) {
	int i;

	args.output = NULL;
	args.input = NULL;

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'o' && i+1 < argc) {
				args.output = argv[++i];
			}
		} else if (i+1 == argc) {
			args.input = strdup(argv[i]);
		}
	}

	if (args.input && args.output == NULL) {
		strtok(args.input, "/");
		if (strtok(NULL, "/") != NULL) {
			args.output = strdup(args.input);
			*(strrchr(args.output, '/') + 1) = '\0';
		}
	}

	return 0;
}

/* If path ends with a file name,
 * 	it is removed.
 * 	
 * 	Assumes path is '\0' terminated. */
char* parse_directory(char* path) {
	char* directory;

	directory = malloc(strlen(path));

	directory = strdup(path);
}

/* Runs the pdflatex command,
 * 	and forces mupdf to refresh.
 * 
 * @param input - the LaTeX file to be compiled
 * @param output - the output dir for the compilation
 *
 * Return values:
 * 0 - Success.
 * 1 - Failure (Probably due to the TeX compile string). */
int compile_tex(char* input, char* output) {
	static const char cmd_template_nooutdir[] = "pdflatex %s -interaction nonstopmode %s";
	static const char cmd_template_outdir[] = "pdflatex -output-directory %s -interaction nonstopmode %s";
	const char* cmd_template = NULL;

	static char* compile_tex_cmd = NULL;
	static size_t compile_cmd_size = 0;

	const int cname_len  = 8,
		  	  cflags_len = 24,

	const char compile_name[]  = "pdflatex",
			   compile_flags[] = "-interaction nonstopmode",
			   compile_out[] = "-output-directory %s",


	const char mupdf_update_cmd[] = "pkil -HUP mupdf";

	/* Used to save time forming the command string.
	 *
	 * Non-zero if the command string is formed. */
	static int cmd_finalized = 0;

	static int cmd_len;
	static char* compile_cmd;

	if (!cmd_finalized) {
		cmd_len = cname_len + cflags_len;
		compile_cmd = malloc(cmd_len + 1);

		if (output == NULL) {
			output
		} else {
			output = parse_directory(output);
		}

		cmd_finalized = 0;
	}

#ifdef DEBUG
	printf(DEBUG_CODE "DEBUG: Compiling LaTeX document...\n" RESET_COLOR);
#endif

	if (compile_tex_cmd == NULL) {
		if (args.output == NULL) {
			args.output = strdup("");
			cmd_template = cmd_template_nooutdir;
		} else {
			cmd_template = cmd_template_outdir;
		}

		if (args.output == NULL) {
			return 1;
		}

		if (compile_cmd_size == 0) {
			compile_cmd_size = sizeof(cmd_template) + 18 + strlen(args.output) + strlen(args.input);
		}

		compile_tex_cmd = malloc(compile_cmd_size + 1);

		if (compile_tex_cmd == NULL) {
			return 1;
		}

		if (cmd_template == NULL) {
			return 1;
		}

		if (sprintf(compile_tex_cmd, cmd_template, args.output, args.input) < compile_cmd_size) {
			return 1;
		}
	}

	system("clear");
	printf("PDFLaTeX Output\n===============\n");
	system(compile_tex_cmd);
	system(mupdf_update_cmd);

	return 0;
}

/* Takes a buffer filled with inotify events 
 * 	and checks if they match the event wd.
 *
 * If matching, calls func().
 *
 * TODO: More generalized handler function, better parameters. */
void handle_watch_events(int ifd, int wd, uint32_t masks, char* buffer, size_t length) {
		const struct inotify_event * event;

		char* eptr;

#ifdef DEBUG
		printf(DEBUG_CODE "DEBUG: Processing watch events...\n" RESET_COLOR);
#endif

		for(eptr = buffer; eptr < (buffer + length); 
			eptr += sizeof(struct inotify_event) + event->len) {

			event = (const struct inotify_event *) eptr;

			if (event->wd == wd) {
				if (event->mask & masks) {
#ifdef DEBUG
					printf(DEBUG_CODE "DEBUG: Running func()\n" RESET_COLOR);
#endif

					compile_tex();
				}

				/* Not sure why this is needed,
				 * 	but Vim-like editors cause stupid events. */
				if (event->mask & (IN_IGNORED | IN_MOVE_SELF | IN_DELETE_SELF)) {
#ifdef DEBUG
					printf(DEBUG_CODE "DEBUG: Readding watched file.\n" RESET_COLOR);
#endif
					inotify_rm_watch(ifd, wd);
					wd = inotify_add_watch(ifd, args.input, masks);
				}
			}
		}
}

int main(int argc, char** argv)
{
	int ifd, /* inotify file descriptor */
		wd; /* Watch file descriptor */

	ssize_t length;
	char buffer[4096]; /* TODO: Should this be 4096??? */

	uint32_t wm; /* Masks to be watched. */
	
	const char usage[] = "Usage: %s [-o output directory] <input>\n";

	wm = (IN_MOVE_SELF | IN_DELETE_SELF | IN_IGNORED | IN_MODIFY);

	parse_args(argc, argv);

#ifdef DEBUG
	printf(DEBUG_CODE "DEBUG: argv[1]: %s\nDEBUG: Input: %s\nDEBUG: Output: %s\n\n" RESET_COLOR, argv[1], args.input, args.output);
#endif

	if (argc < 2 || args.input == NULL) {
		printf(usage, argv[0]);
		return 0;
	}

	ifd = inotify_init();
	wd = inotify_add_watch(ifd, args.input, wm);

	printf("Watching %s\n", args.input);

	for(;;) {
		length = read(ifd, buffer, sizeof buffer);

		if (length == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("read"); /* Reports errors in system funcs */
			exit(EXIT_FAILURE);
		}

		if (length <= 0) continue;

		handle_watch_events(ifd, wd, wm, buffer, length);
	}

	return 0;
}
