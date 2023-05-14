#!/usr/bin/env python
import subprocess
import argparse
import time
from typing import List
import json
from tqdm import tqdm


def run_command(filename: str, arguments: List[str]):
    """
        spawns a process to run a command. This should be blocking.

        @param filename: name of the program to run
        @param arguments: List of strings to be passed to the program as command line arguments.
    """
    result = subprocess.run(
        [f"./{filename}"] + arguments, stdout=subprocess.PIPE)
    result = result.stdout.decode().strip()
    # print(
    # f"Received Output [{result}] from program [{filename}] with args [{arguments}]")
    return result


if __name__ == '__main__':
    threadcounts = [1, 2, 4, 8, 12, 16]
    input_tests = [
        "tests/ban.txt",
        "tests/onemeglorem.txt",
        "tests/lorem.txt",
        "tests/bible.txt",
        "tests/E.coli"
    ]
    iterations = [1, 2, 3]
    print("Running Evaluation.")
    tests = []
    for input_test in input_tests:
        for threadcount in threadcounts:
            for iteration in iterations:
                tests.append([input_test, threadcount, iteration, ""])

    pbar = tqdm(range(len(tests)))
    for i in pbar:
        pbar.set_description(f"Processing {i}: {str(tests[i][0])}")
        tests[i][3] = run_command(
            "bin/TextCompress", [str(tests[i][1]), str(tests[i][0])])

    with open("evalres.json", 'w') as f:
        json.dump(tests, f, indent=2)
