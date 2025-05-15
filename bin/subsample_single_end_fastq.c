#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <zlib.h>  // For gzip support

#define MAX_LINE_LEN 1024

typedef struct {
    gzFile in_file;
    FILE *out_file;
    int subsample_fraction;
    int seed;
    int verbose;
    int compress_output;
} Config;

void print_help() {
    printf("Usage: subsample_single_end_fastq [OPTIONS]\n");
    printf("Subsamples a single-end FASTQ file.\n\n");
    printf("Options:\n");
    printf("  -i FILE    Input FASTQ file (required, can be .gz)\n");
    printf("  -o FILE    Output FASTQ file (required)\n");
    printf("  -f INT     Subsampling fraction (1-100, default: 10)\n");
    printf("  -s INT     Random seed (default: current time)\n");
    printf("  -z         Compress output with gzip\n");
    printf("  -v         Verbose output\n");
    printf("  -h         Print this help message\n");
}

void parse_args(int argc, char *argv[], Config *config) {
    int opt;
    config->in_file = NULL;
    config->out_file = NULL;
    config->subsample_fraction = 10;
    config->seed = time(NULL);
    config->verbose = 0;
    config->compress_output = 0;

    while ((opt = getopt(argc, argv, "i:o:f:s:zvh")) != -1) {
        switch (opt) {
            case 'i':
                config->in_file = gzopen(optarg, "rb");
                if (!config->in_file) {
                    fprintf(stderr, "Error: Could not open input file %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                if (config->compress_output) {
                    char command[512];
                    snprintf(command, sizeof(command), "gzip -c > %s", optarg);
                    config->out_file = popen(command, "w");
                } else {
                    config->out_file = fopen(optarg, "w");
                }
                if (!config->out_file) {
                    fprintf(stderr, "Error: Could not open output file %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                config->subsample_fraction = atoi(optarg);
                if (config->subsample_fraction < 1 || config->subsample_fraction > 100) {
                    fprintf(stderr, "Error: Subsampling fraction must be between 1 and 100\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                config->seed = atoi(optarg);
                break;
            case 'z':
                config->compress_output = 1;
                break;
            case 'v':
                config->verbose = 1;
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Error: Unknown option\n");
                print_help();
                exit(EXIT_FAILURE);
        }
    }

    if (!config->in_file || !config->out_file) {
        fprintf(stderr, "Error: Missing required arguments\n");
        print_help();
        exit(EXIT_FAILURE);
    }
}

int read_fastq_record(gzFile file, char lines[4][MAX_LINE_LEN]) {
    for (int i = 0; i < 4; i++) {
        if (!gzgets(file, lines[i], MAX_LINE_LEN)) {
            return 0;
        }
        lines[i][strcspn(lines[i], "\n")] = '\0';
    }
    return 1;
}

void write_fastq_record(FILE *file, char lines[4][MAX_LINE_LEN]) {
    for (int i = 0; i < 4; i++) {
        fprintf(file, "%s\n", lines[i]);
    }
}

void subsample_single_end_fastq(Config *config) {
    char lines[4][MAX_LINE_LEN];
    long total_reads = 0;
    long kept_reads = 0;
    
    srand(config->seed);
    
    if (config->verbose) {
        printf("Subsampling single-end FASTQ file at %d%%\n", config->subsample_fraction);
        printf("Random seed: %d\n", config->seed);
    }
    
    while (read_fastq_record(config->in_file, lines)) {
        total_reads++;
        
        if ((rand() % 100) < config->subsample_fraction) {
            write_fastq_record(config->out_file, lines);
            kept_reads++;
        }
    }
    
    if (config->verbose) {
        printf("Total reads processed: %ld\n", total_reads);
        printf("Reads kept: %ld (%.2f%%)\n", kept_reads, 
               (double)kept_reads / total_reads * 100);
    }
}

void cleanup(Config *config) {
    if (config->in_file) gzclose(config->in_file);
    if (config->out_file) {
        if (config->compress_output) {
            pclose(config->out_file);
        } else {
            fclose(config->out_file);
        }
    }
}

int main(int argc, char *argv[]) {
    Config config;
    
    parse_args(argc, argv, &config);
    subsample_single_end_fastq(&config);
    cleanup(&config);
    
    return EXIT_SUCCESS;
}
