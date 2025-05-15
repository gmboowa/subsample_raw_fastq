### C script for logical subsampling of FASTQ reads

## Features

**Maintains Read Pairs**: Always keeps or discards both R1 & R2 reads together to maintain pairing.

**Configurable Subsampling**: You can specify any subsampling fraction from 1% to 100%.

**Random Seed Control**: Allows setting a specific random seed for reproducible results.

**Efficient Processing**: Reads and writes FASTQ records in blocks of 4 lines.

**Error Handling**: Checks for file opening errors & unpaired reads at the end of files.

**Verbose Output**: Optionally reports statistics about the subsampling process.




### 1. C Script for Subsampling Paired-End FASTQ Reads

Compilation:

You'll need to link with zlib:

```bash

gcc -o subsample_paired_fastq subsample_paired_fastq.c -lz


```

Usage:

```bash

./subsample_paired_fastq -a input_R1.fastq.gz -b input_R2.fastq.gz -x output_R1.fastq -y output_R2.fastq -f 10 -z


```

### 2. C Script for Subsampling Single-End FASTQ Reads

Compilation:

You'll need to link with zlib:

```bash


gcc -o subsample_single_end_fastq.c -o subsample_single_end_fastq -lz

```

Usage:

```bash

# Subsample 20% with gzip-compressed output and verbose logs

./subsample_single_end_fastq -i input.fastq -o subsampled.fastq.gz -f 20 -z -v

```
### 3. These versions will:

Properly handle your gzipped input files

Give you a clear error if the files are wrongly paired

Allow compressed output with -z

Provide more detailed information about any issues
