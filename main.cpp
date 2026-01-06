#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <iomanip>

// #include "testcases.h"

using Vector = std::vector<int>;
using Matrix = std::vector<std::vector<int>>;
using Answer = std::vector<std::vector<int>>;

struct MaxCliqueResult {
    std::vector<int> vertecies;
    int min_weight;
};

void Print2DVector(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

// saves Answer as json
void SaveResult(const Answer& answer, const std::filesystem::path& output_path) {
    std::ofstream file(output_path);
    file << "{";
    for (int i = 0; i < std::ssize(answer); i++) {
        file << "\n    \"" << i << "\": [";
        for (int j = 0; j < std::ssize(answer[i]); j++) {
            file << answer[i][j];
            if (j != std::ssize(answer[i]) - 1) {
                file << ", ";
            }
        }
        file << "]";
        if (i != std::ssize(answer) - 1) {
            file << ",";
        }
    }
    file << "\n}";
}

Matrix CreateMatrix(int max_size) {
    Matrix matrix(max_size, Vector(max_size, 0));
    return matrix;
}

Matrix ParseCSV(const std::filesystem::path& csv_path, int max_size) {
    Matrix matrix = CreateMatrix(max_size);

    std::ifstream file(csv_path);
    std::string line;

    // header
    file >> line;

    while (file >> line) {
        std::stringstream ss(line);
        int i, j, value;
        char sep;
        ss >> i >> sep >> j >> sep >> value;
        matrix[i][j] = value;
        matrix[j][i] = value;
    }

    return matrix;
}

// custom clique comparison metric
int GetCliqueValue(const std::vector<int>& clique, int min_weight) {
    return std::ssize(clique) * min_weight;
}

// greedy algorithm to find maximal clique (not necessarily maximum)
MaxCliqueResult FindMaxClique(const Matrix& matrix) {
    MaxCliqueResult result{{}, 0};

    for (int start = 0; start < std::ssize(matrix); start++) {
        std::vector<int> clique = {start};
        int min_weight = INT_MAX;
        for (int i = 0; i < std::ssize(matrix); i++) {
            int new_min_weight = min_weight;
            bool reachable = true;

            for (int ver : clique) {
                if (matrix[ver][i] == 0) {
                    reachable = false;
                    break;
                } else {
                    new_min_weight = std::min(new_min_weight, matrix[ver][i]);
                }
            }

            if (reachable) {
                clique.push_back(i);
                min_weight = std::min(min_weight, new_min_weight);
            }
        }

        if (min_weight == INT_MAX) {
            min_weight = 0;
        }

        if (GetCliqueValue(clique, min_weight) >
            GetCliqueValue(result.vertecies, result.min_weight)) {
            result.vertecies = std::move(clique);
            result.min_weight = min_weight;
        }
    }

    return result;
}

void RemoveClique(Matrix* matrix, const MaxCliqueResult& max_clique) {
    const auto& vertecies = max_clique.vertecies;
    for (int i = 0; i < std::ssize(vertecies); i++) {
        for (int j = i + 1; j < std::ssize(vertecies); j++) {
            (*matrix)[vertecies[i]][vertecies[j]] -= max_clique.min_weight;
            (*matrix)[vertecies[j]][vertecies[i]] -= max_clique.min_weight;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <csv_path> <max_size> <output_path>" << std::endl;
        return 1;
    }

    std::ios::sync_with_stdio(false);

    std::filesystem::path csv_path(argv[1]);
    std::filesystem::path out_path(argv[3]);
    int max_size = std::stoi(argv[2]);

    std::cout << "Parsing CSV file " << csv_path << std::endl;
    Matrix intersections = ParseCSV(csv_path, max_size);
    Answer answer(max_size);

    int min_weight = 1;
    int current_element = 0;

    std::cout << "Starting algirithm" << std::endl;
    while (min_weight > 0) {
        auto result = FindMaxClique(intersections);
        for (int ver : result.vertecies) {
            for (int inc = 0; inc < result.min_weight; inc++) {
                answer[ver].push_back(current_element + inc);
            }
        }

        std::cout << "\rclique size: " << std::setw(10) << result.vertecies.size();
        std::cout.flush();

        current_element += result.min_weight;
        min_weight = result.min_weight;

        RemoveClique(&intersections, result);
    }
    std::cout << std::endl;

    std::cout << "Answer: " << std::endl;
    Print2DVector(answer);

    std::cout << "Number of elements: " << current_element << std::endl;

    std::cout << "Saving result to " << out_path << std::endl;
    SaveResult(answer, out_path);
    return 0;
}