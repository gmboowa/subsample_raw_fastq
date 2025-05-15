C script for logical subsampling of FASTQ reads

## Features

Maintains Read Pairs: Always keeps or discards both R1 and R2 reads together to maintain pairing.

Configurable Subsampling: You can specify any subsampling fraction from 1% to 100%.

Random Seed Control: Allows setting a specific random seed for reproducible results.

Efficient Processing: Reads and writes FASTQ records in blocks of 4 lines.

Error Handling: Checks for file opening errors and unpaired reads at the end of files.

Verbose Output: Optionally reports statistics about the subsampling process.




C Script for Subsampling Paired-End FASTQ Reads

Compilation:

You'll need to link with zlib:

'''bash

gcc -o subsample_paired_fastq subsample_paired_fastq.c -lz

....

Usage:

'''bash

./subsample_paired_fastq -a input_R1.fastq.gz -b input_R2.fastq.gz -x output_R1.fastq -y output_R2.fastq -f 10 -z

...

C Script for Subsampling ONT Single-End FASTQ Reads

Here's a C script specifically designed for subsampling Oxford Nanopore Technologies (ONT) single-end FASTQ reads:





This version will:

Properly handle your gzipped input files

Give you a clear error if the files are wrongly paired

Allow compressed output with -z

Provide more detailed information about any issues
