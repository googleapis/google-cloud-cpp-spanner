# Cloud Spanner C++ Client Library Benchmarks

This directory contains end-to-end benchmarks for the Cloud Spanner C++ client
library. The benchmarks execute experiments against the production environment.
You need a working Google Cloud Platform project and Cloud Spanner instance
to run these benchmarks. We recommend that you use an isolated instance, that is
one without any other workloads, to run each experiment, and that you do not run
more than one experiment at a time in said instance.

## Creating an instance

Assuming you have an existing Google Cloud Project you can create a Cloud
Spanner instance using the console or:

```console
GOOGLE_CLOUD_PROJECT=...                     # Your project ID
GOOGLE_CLOUD_CPP_INSTANCE_NAME=benchmarks    # Choose your instance ID
INSTANCE_CONFIG=regional-us-central1         # Choose the instance location(s)
gcloud spanner instances create ${GOOGLE_CLOUD_CPP_INSTANCE_NAME} \
    --config=${INSTANCE_CONFIG} --description="An instance to run Benchmarks" \
    --nodes=3
```

## CPU Overhead Experiment

This experiment measures the CPU overhead of the client library vs. raw gRPC.
This overhead is never expected to be zero, the library is performing useful
work, but we want it to be low, unsurprising, and (once a baseline is
established) to remain stable unless there is good reason to add overhead.

We recommend that you compile and run these experiments on a VM running on the
same region as the spanner instance you will use for the tests. Create and
configure the VM instance, and then install the development tools for whatever
platform you chose (see [INSTALL](../../../INSTALL.md) for details).

### Compiling the library

You must compile both the library and its dependencies with optimization, using
CMake this is:

```bash
git clone https://github.com/googleapis/google-cloud-cpp-spanner.git
cmake -Hsuper -Bcmake-out/si -DCMAKE_BUILD_TYPE=Release -GNinja \
     -DGOOGLE_CLOUD_CPP_EXTERNAL_PREFIX=$HOME/local-spanner
cmake --build cmake-out/si --target project-dependencies
cmake -H. -B.release -DCMAKE_BUILD_TYPE=Release -GNinja \
    -DCMAKE_PREFIX_PATH=$HOME/local-spanner
cmake --build .release
```

### Running the benchmark

By default the benchmarks run simple smoke tests, the intention is for these
benchmarks to run as part of the CI build, where we want them to finish quickly.
You must specify some options to control for how long the experiments run, for
example, to perform the experiment measuring the CPU overhead of reading columns
of type `STRING` you would run:

```bash
.release/google/cloud/spanner/benchmarks/multiple_rows_cpu_benchmark \
    --project=${GOOGLE_CLOUD_PROJECT} \
    --instance=${GOOGLE_CLOUD_CPP_INSTANCE} \
    --table-size=1000000 \
    --maximum-clients=8 \
    --maximum-threads=16 \
    --iteration-duration=5 \
    --samples=60 --experiment=read-string
```
 `std::string` The program can run different experiments for different datatypes, to run all
of them in a loop and capture the results use:

```bash
for exp in read-bool read-bytes read-date read-float64 \
          read-int64 read-string read-timestamp; do \
  .release/google/cloud/spanner/benchmarks/multiple_rows_cpu_benchmark \
    --project=${GOOGLE_CLOUD_PROJECT} \
    --instance=${GOOGLE_CLOUD_CPP_INSTANCE} \
    --table-size=1000000 \
    --maximum-clients=8 \
    --maximum-threads=16 \
    --iteration-duration=5 \
    --samples=60 --experiment=${exp} 2>&1 | tee mrcb-${exp}.csv; \
done
```

### Inspecting the results

At this time we have not developed scripts to analyze the benchmark results,
but some simple R commands can help, start R in your command line and then
issue the following commands:

```R
require(ggplot2) # may require install.packages("ggplot2") the first time
df <- ()
for(file in c('mrcb-read-bytes.txt', 'mrcb-read-bool.txt', 'mrcb-read-date.txt',
              'mrcb-read-float64.txt', 'mrcb-read-int64.txt',
              'mrcb-read-string.txt', 'mrcb-read-timestamp.txt')) {
    t <- read.csv(file, comment.char='#');
    name <- gsub('mrcb-([a-z-]+).*', '\\1', file);
    t$experiment = factor(name);
    df <- rbind(df, t);
}

aggregate(CpuTimePerRow ~ UsingStub + experiment, data=df, FUN=mean)

ggplot(data=df, aes(color=UsingStub, x=experiment, y=CpuTime)) + geom_boxplot()
ggsave('read-data-types.png')
```

## Single Row Throughput Experiment

This experiment measures the throughput of either single-row inserts or
single-row reads using different numbers of clients and threads. The objective
is to verify the client library scales well with more threads and those not
introduce bottlenecks.

To run the experiment reading data for approximately 5 minutes use 20 samples
of 15 seconds each:

```bash
.release/google/cloud/spanner/benchmarks/single_row_throughput_benchmark \
    --project=${GOOGLE_CLOUD_PROJECT} \
    --instance=${GOOGLE_CLOUD_CPP_INSTANCE} \
    --iteration-duration=15 \
    --table-size=10000000 \
    --maximum-clients=32 \
    --maximum-threads=1024 \
    --samples=20 2>&1 \
    --experiment=read 2>&1 | tee srtp-read.txt
```
