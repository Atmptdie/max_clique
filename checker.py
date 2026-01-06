import argparse
import csv
from collections import defaultdict
import json
import tqdm


def main():
    argparser = argparse.ArgumentParser()

    argparser.add_argument(
        "--answer", help=".json file with answers to check", type=str, required=True
    )
    argparser.add_argument(
        "--inter", help=".csv file with intersections", type=str, required=True
    )

    args = argparser.parse_args()

    actual_intersections = defaultdict(dict)
    with open(args.inter, "r") as f:
        reader = csv.reader(f, lineterminator="\n")
        rows = list(reader)[1:]
        for ij, value in tqdm.tqdm(rows, desc="loading intersections from csv"):
            i, j = map(int, ij.split("_"))
            actual_intersections[i][j] = int(value)
            actual_intersections[j][i] = int(value)

    num_vertecies = len(actual_intersections[0]) + 1

    intersections = defaultdict(dict)
    with open(args.answer, "r") as f:
        data = json.load(f)

    assert (
        len(data) == num_vertecies
    ), "Number of vertecies in answer is not equal to number of vertecies in intersections file"

    max_element = 0
    for key, vertecies in tqdm.tqdm(data.items(), desc="keys loaded from answers"):
        max_element = max(max_element, max(vertecies))
        for other_key, other_vertecies in data.items():
            if key == other_key:
                continue

            intersected = len(set(vertecies) & set(other_vertecies))
            intersections[int(key)][int(other_key)] = intersected
            intersections[int(other_key)][int(key)] = intersected


    for key in tqdm.tqdm(actual_intersections, desc="keys checked"):
        for other_key in actual_intersections[key]:
            if other_key not in intersections[key]:
                assert (
                    False
                ), f"Intersection not found for {key} and {other_key}. Expected {actual_intersections[key][other_key]}"
            assert (
                intersections[key][other_key] == actual_intersections[key][other_key]
            ), f"Intersections are not equal for {key} and {other_key}"

    print("All checks passed!")
    print(f"Solution power: {max_element}")


if __name__ == "__main__":
    main()
