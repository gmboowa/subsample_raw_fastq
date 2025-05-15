#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <zlib.h>  // For gzip support

#define MAX_LINE_LEN 1024

typedef struct {
    gzFile r1_in;
    gzFile r2_in;
    FILE *r1_out;
    FILE *r2_out;
    int subsample_fraction;
    int seed;
    int verbose;
    int compress_output;
} Config;

void print_help() {
    printf("Usage: subsample_paired_fastq [OPTIONS]\n");
    printf("Subsamples paired FASTQ files while maintaining read pairs.\n\n");
    printf("Options:\n");
    printf("  -a FILE    Input R1 FASTQ file (required, can be .gz)\n");
    printf("  -b FILE    Input R2 FASTQ file (required, can be .gz)\n");
    printf("  -x FILE    Output R1 FASTQ file (required)\n");
    printf("  -y FILE    Output R2 FASTQ file (required)\n");
    printf("  -f INT     Subsampling fraction (1-100, default: 10)\n");
    printf("  -s INT     Random seed (default: current time)\n");
    printf("  -z         Compress output with gzip\n");
    printf("  -v         Verbose output\n");
    printf("  -h         Print this help message\n");
}

void parse_args(int argc, char *argv[], Config *config) {
    int opt;
    config->r1_in = NULL;
    config->r2_in = NULL;
    config->r1_out = NULL;
    config->r2_out = NULL;
    config->subsample_fraction = 10;
    config->seed = time(NULL);
    config->verbose = 0;
    config->compress_output = 0;

    while ((opt = getopt(argc, argv, "a:b:x:y:f:s:zvh")) != -1) {
        switch (opt) {
            case 'a':
                config->r1_in = gzopen(optarg, "rb");
                if (!config->r1_in) {
                    fprintf(stderr, "Error: Could not open R1 input file %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
                config->r2_in = gzopen(optarg, "rb");
                if (!config->r2_in) {
                    fprintf(stderr, "Error: Could not open R2 input file %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'x':
                if (config->compress_output) {
                    char command[512];
                    snprintf(command, sizeof(command), "gzip -c > %s", optarg);
                    config->r1_out = popen(command, "w");
                } else {
                    config->r1_out = fopen(optarg, "w");
                }
                if (!config->r1_out) {
                    fprintf(stderr, "Error: Could not open R1 output file %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'y':
                if (config->compress_output) {
                    char command[512];
                    snprintf(command, sizeof(command), "gzip -c > %s", optarg);
                    config->r2_out = popen(command, "w");
                } else {
                    config->r2_out = fopen(optarg, "w");
                }
                if (!config->r2_out) {
                    fprintf(stderr, "Error: Could not open R2 output file %s\n", optarg);
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

    if (!config->r1_in || !config->r2_in || !config->r1_out || !config->r2_out) {
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

void subsample_paired_fastq(Config *config) {
    char r1_lines[4][MAX_LINE_LEN];
    char r2_lines[4][MAX_LINE_LEN];
    long total_pairs = 0;
    long kept_pairs = 0;
    long r1_count = 0, r2_count = 0;
    
    srand(config->seed);
    
    if (config->verbose) {
        printf("Subsampling paired FASTQ files at %d%%\n", config->subsample_fraction);
        printf("Random seed: %d\n", config->seed);
    }
    
    while (1) {
        int r1_success = read_fastq_record(config->r1_in, r1_lines);
        int r2_success = read_fastq_record(config->r2_in, r2_lines);
        
        if (!r1_success && !r2_success) break;
        
        if (!r1_success || !r2_success) {
            fprintf(stderr, "Error: Unpaired reads detected - R1 has %ld reads, R2 has %ld reads\n",
                   r1_count + (r1_success ? 1 : 0),
                   r2_count + (r2_success ? 1 : 0));
            exit(EXIT_FAILURE);
        }
        
        r1_count++;
        r2_count++;
        total_pairs++;
        
        if ((rand() % 100) < config->subsample_fraction) {
            write_fastq_record(config->r1_out, r1_lines);
            write_fastq_record(config->r2_out, r2_lines);
            kept_pairs++;
        }
    }
    
    if (config->verbose) {
        printf("Total read pairs processed: %ld\n", total_pairs);
        printf("Read pairs kept: %ld (%.2f%%)\n", kept_pairs, 
               (double)kept_pairs / total_pairs * 100);
    }
}

void cleanup(Config *config) {
    if (config->r1_in) gzclose(config->r1_in);
    if (config->r2_in) gzclose(config->r2_in);
    if (config->r1_out) {
        if (config->compress_output) {
            pclose(config->r1_out);
        } else {
            fclose(config->r1_out);
        }
    }
    if (config->r2_out) {
        if (config->compress_output) {
            pclose(config->r2_out);
        } else {
            fclose(config->r2_out);
        }
    }
}

int main(int argc, char *argv[]) {
    Config config;
    
    parse_args(argc, argv, &config);
    subsample_paired_fastq(&config);
    cleanup(&config);
    
    return EXIT_SUCCESS;
}
