#include <sys/inotify.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv)
{
	int ifd, wd, temp;
	ssize_t length;
	char buffer[4096];
	char* ep; /* Pointer to event in buffer */
	const struct inotify_event *event;
	const char cmd_template[] = "pdflatex -interaction nonstopmode %s";
	char* command;
	const char mupdf_update_cmd[] = "pkill -HUP mupdf";

	command = malloc(sizeof cmd_template + strlen(argv[1]) + 1);

	temp = sprintf(command, cmd_template, argv[1]);

	if (!temp) {
		printf("Problem with command string, exiting...\n");
		return 1;
	}

	ifd = inotify_init();

	wd = inotify_add_watch(ifd, argv[1], (IN_MOVE_SELF | IN_IGNORED));

	printf("Watching %s\n", argv[1]);

	for(;;) {
		length = read(ifd, buffer, sizeof buffer);

		if (length == -1 && errno != EAGAIN) {
			perror("read"); /* Reports errors in system funcs */
			exit(EXIT_FAILURE);
		}

		if (length <= 0) {
			printf("Length %ld\n", length);
			continue;
		}

		for(ep = buffer; ep < (buffer + length); 
				ep += sizeof (struct inotify_event) + event->len) {
			event = (const struct inotify_event *) ep;

			if (event->wd == wd) {
				if (event->mask & (IN_MOVE_SELF | IN_IGNORED)) {
					system("clear");
					system(command);
					system(mupdf_update_cmd);
				}

				if (event->mask & IN_IGNORED) {
					inotify_rm_watch(ifd, wd);
					wd = inotify_add_watch(ifd, argv[1], (IN_MOVE | IN_IGNORED));
				}
			}
		}
	}
}
