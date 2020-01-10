#!/usr/bin/env python3
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Summarize the results from running throughput_vs_cpu_benchmark"""

# %%
import argparse
import numpy as np
import pandas as pd
import plotnine as p9


# %%
def load_benchmark_output(file):
    """Loads the output from the storage_throughput_vs_cpu_benchmark."""
    df = pd.read_csv(file, comment='#', header=0)
    # The elapsed time is in microseconds, normalize throughput to events/s
    df['Throughput'] = df.EventCount * 1000000 / df.ElapsedTime
    return df


parser = argparse.ArgumentParser()
parser.add_argument('--input-file', type=argparse.FileType('r'), required=True,
                    help='the benchmark output file to load')
parser.add_argument('--output-file', type=str, required=True,
                    help='the name for the output plot')
args = parser.parse_args()


# %%
data = load_benchmark_output(args.input_file)

# %%
print(data.head())

# %%
print(data.describe())

# %%
(p9.ggplot(data=data,
           mapping=p9.aes(x='ThreadCount', y='Throughput'))
 + p9.geom_point()).save(args.output_file)
