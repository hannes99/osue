/**
 * @author Hannes Laimer <e11808227@student.tuwien.ac.at>
 * @date 24.10.2019
 *
 * @brief File contains an implementation of mydiff, which compares two files line by line
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief compairs the next line of two files and returns the amount of differences
 *
 * Takes one char form both files and compairs it, if they're not equal
 * increment the differences counter, until at least one of the files reaches a
 * '\n'. Then, if both files did not reach EOF, call fgetc() until both files
 * reached a '\n', this has to be done since we have to be sure that when
 * compair_line() is called again both files are in the same line.
 *
 * @param file1 the first file.
 * @param file1 the seconds file.
 * @param case_insensitive if 1 we should ignore the case when compairing,
 *                         otherwise the case should not be ignored
 */
int compair_line(FILE *file1, FILE *file2, int case_insensitive) {
    int differences = 0;
    char c1 = fgetc(file1);
    char c2 = fgetc(file2);
    while (c1 != '\n' && c2 != '\n' && !feof(file1) && !feof(file2)) {
        if (case_insensitive) {
            if (strncasecmp(&c1, &c2, 1) != 0)
                differences++;
        } else {
            if (strncmp(&c1, &c2, 1) != 0)
                differences++;
        }
        c1 = fgetc(file1);
        c2 = fgetc(file2);
    }

    if (feof(file1) || feof(file2))
        return differences;

    while (c1 != '\n' || c2 != '\n') {
        if (c1 != '\n') c1 = fgetc(file1);
        if (c2 != '\n') c2 = fgetc(file2);
    }
    return differences;
}

/**
 * @brief prints the usage info
 */
void usage (void) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", "mydiff");
}

int main(int argc, char *argv[]) {
    FILE *out = fdopen(STDOUT_FILENO, "w");
    FILE *stderr = fdopen(STDERR_FILENO, "w");

    if (out == NULL) {
        fprintf(stderr, "%s: could not open STDOUT\n", argv[0]);
    }
    int case_insensitive = 0;
    int output_to_file = 0;
    char c;
    while ((c = getopt(argc, argv, "o:i")) != -1 ){
        switch (c) {
            case 'o':
                out = fopen(optarg, "w");
                if ( out == NULL ) {
                    fprintf(stderr, "%s: %s could not be opened\n", argv[0], optarg);
                    exit(EXIT_FAILURE);
                }
                output_to_file = 1;
                break;
            case 'i': case_insensitive=1; break;
            default:
                usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (out == NULL) {
        usage();
        exit(EXIT_FAILURE);
    }

    FILE *file1 = fopen(argv[optind], "r");
    if (file1 == NULL) {
        if (argv[optind]) {
            fprintf(stderr, "%s: %s: could not be loaded\n", argv[0], argv[optind]);
        }
        if (output_to_file) {
            fclose(out);
        }
        usage();
        exit(EXIT_FAILURE);
    }

    FILE *file2 = fopen(argv[optind+1], "r");
    if (file2 == NULL) {
        if (argv[optind+1]) {
            fprintf(stderr, "%s: %s: could not be loaded\n", argv[0], argv[optind+1]);
        }
        if (output_to_file) {
            fclose(out);
        }
        fclose(file1);
        usage();
        exit(EXIT_FAILURE);
    }

    int line = 1;
    while (!(feof(file1) || feof(file2))) {
        int diffs = compair_line(file1, file2, case_insensitive);
        if (diffs) {
            fprintf(out, "Line: %d, characters: %d\n", line, diffs);
        }
        line++;
    }
    fclose(file1);
    fclose(file2);
    if (output_to_file) fclose(out);

    exit(EXIT_SUCCESS);
}
