# Text Compress

Lossless text compression achieved via openmp

### Build:
```
make
```

### Usage:
From the project root folder,

#### Full pass test
```
./bin/TextCompress <Textfile> <number_of_threads>
```

>WARN ! <br>Currently only text is supported, numbers are not handled properly and will interfere with the Run-length Encoding protion of the compression/decompression

# Report / Postmortem:

# intro

### What is BWT

### What is Run-Length Encoding

### What is Paring the two


# Problems Faced

### (poor inverseBWT implementation) TableSort Vs First-Last Pair

Table sort (Rosetta Code):
```
real    18m39.432s
```

First-Last Pair Hopping (shout out to the homie carter):
```
real    0m0.493s
```

### Memory Vs Speed
(Performing BWT inplace Vs TableSorted Rotations)

Inplace BWT citation:

>Maxime Crochemore, Roberto Grossi, Juha Kärkkäinen, Gad M. Landau,
Computing the Burrows–Wheeler transform in place and in small space,
Journal of Discrete Algorithms,
Volume 32,
2015,
Pages 44-52,
ISSN 1570-8667,
https://doi.org/10.1016/j.jda.2015.01.004.
(https://www.sciencedirect.com/science/article/pii/S1570866715000052)



! Node scaling: 
Its worth noting that the more divisions of the domain the problem has, the less memory the tablesort method requires.

In fact this would be a good candidate for distributed memory, since each cluster node could easily handle its own division of memory given enough nodes, thus allowing the algorith to use the speed variant.

### Numbers with RLE
numbers in the text have yet to work, considered out of scope for the time being since the primary objective was genome sequences.



# Results:
Metrics were gathered using a python script found in the util folder.

Test cases:

| File name        | description                               | Size on disk | Character count |
| ---------------- | ----------------------------------------- | ------------ | --------------- |
| ban.txt          | banana.                                   | 4.0K         | 10              |
| lorem_medium.txt | 1,500,000~ characters of lorem ipsum.     | ?            | ?               |
| lorem.txt        | 3,000,000~ characters of lorem ipsum.     | 2.9M         | 3000635         |
| bible.txt        | The bible in plain text                   | 3.9M         | 4047392         |
| E.coli           | The full Escherichia Coli genome sequence | 4.5M         | 4638690         |


For each file a backward and forward pass was performed 10 times.

During each pass, CPU and RAM utilization was recorded.

