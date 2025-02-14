/*
 * anbernic_favorites_tool
 *
 * Tool for shuffling or sorting the favorites list on Anbernic consoles.
 * Not affiliated with or endorsed by Anbernic.
 *
 * Usage:
 *   anbernic_favorites_tool [--sort|--shuffle] [FILE]
 *
 * Options:
 *   --sort        Sort the favorites list (default)
 *   --shuffle     Shuffle the favorites list
 *   --help        Display this help message
 *
 * If FILE is not provided, the default "/mnt/data/misc/.favorite" is used.
 * The file is modified in-place; no backups are made. Use at your own risk!
 *
 * MIT License
 * Copyright 2024 Ori Livneh <ori.livneh@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

constexpr uint32_t kPolynomial = 0xedb88320;
constexpr size_t kCrcTableSize = 256;

enum class Operation { kShuffle, kSort };

std::array<uint32_t, kCrcTableSize> GenerateCrc32Table() {
  std::array<uint32_t, kCrcTableSize> table;
  for (size_t i = 0; i < kCrcTableSize; ++i) {
    uint32_t crc = static_cast<uint32_t>(i);
    for (size_t j = 0; j < 8; ++j) {
      crc = (crc & 1) ? (crc >> 1) ^ kPolynomial : crc >> 1;
    }
    table[i] = crc;
  }
  return table;
}

// Compute CRC32/JAMCRC checksum.
// JAMCRC is the standard CRC32 with the final bits inverted.
uint32_t ComputeJamCrc(const std::string &data) {
  static const std::array<uint32_t, kCrcTableSize> kCrc32Table =
      GenerateCrc32Table();
  uint32_t crc = 0;
  for (uint8_t byte : data) {
    crc = kCrc32Table[(crc ^ byte) & 0xff] ^ (crc >> 8);
  }
  return crc;
}

// Reads a CSV-like file of arcade name fixes and returns a mapping of arcade
// titles to their proper names.
std::map<std::string, std::string> ParseArcadeNameFixesCsv(
    const std::string &file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " + file_path);
  }

  std::map<std::string, std::string> result;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }

    std::istringstream line_stream(line);
    std::string key, field;
    std::getline(line_stream, key, ',');

    std::string last_value;
    while (std::getline(line_stream, field, ',')) {
      if (field.starts_with('"') && field.ends_with('"')) {
        last_value = field.substr(1, field.size() - 2);  // Remove quotes
      }
    }

    result[key] = last_value;
  }

  return result;
}

std::string RemoveFileExtension(const std::string &filename) {
  auto pos = filename.find_last_of('.');
  if (pos == std::string::npos) {
    return filename;
  }
  return filename.substr(0, pos);
}

// GetSortKey returns the key used for sorting. By default, it uses the title
// from the favorite file. If the title appears in the arcade name fixes file,
// the corrected title from the fixes file is used instead.
std::string GetSortKey(
    const std::string &line,
    const std::map<std::string, std::string> &arcade_titles) {
  auto first_colon = line.find(':');
  if (first_colon == std::string::npos) {
    return line;
  }

  std::string key = line.substr(0, first_colon);
  key = RemoveFileExtension(key);

  auto it = arcade_titles.find(key);
  if (it != arcade_titles.end()) {
    return it->second;
  }
  return key;
}

void ShuffleOrSortFavorites(const std::string &file_path, Operation operation) {
  std::ifstream infile(file_path, std::ios::binary);
  if (!infile.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " + file_path);
  }

  std::vector<std::string> lines;
  std::string line;

  if (std::getline(infile, line)) {
    if (line != "Version=1") {
      throw std::runtime_error(
          "Invalid file format: first line must be 'Version=1'");
    }
    while (std::getline(infile, line)) {
      lines.push_back(std::move(line));
    }
  }
  infile.close();

  if (!lines.empty()) {
    lines.pop_back();  // Discard the checksum
  }

  if (operation == Operation::kShuffle) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lines.begin(), lines.end(), g);
  } else {
    std::map<std::string, std::string> arcade_titles;
    try {
      constexpr char kArcadePlusCsv[] = "/mnt/vendor/bin/arcade-plus.csv";
      arcade_titles = ParseArcadeNameFixesCsv(kArcadePlusCsv);
      std::cerr << "Info: Loaded " << arcade_titles.size()
                << " arcade title fixes.\n";
    } catch (const std::exception &e) {
      std::cerr << "Warning: " << e.what()
                << ". Not applying arcade title fixes.\n";
    }
    std::sort(lines.begin(), lines.end(),
              [&arcade_titles](const std::string &a, const std::string &b) {
                return GetSortKey(a, arcade_titles) <
                       GetSortKey(b, arcade_titles);
              });
  }

  // Update the index field in each processed line
  for (size_t i = 0; i < lines.size(); ++i) {
    std::string &entry = lines[i];
    size_t last_colon = entry.rfind(':');
    size_t second_last_colon = entry.rfind(':', last_colon - 1);

    if (second_last_colon != std::string::npos) {
      entry.replace(second_last_colon + 1, last_colon - second_last_colon - 1,
                    std::to_string(i + 1));
    }
  }

  std::string output_content = "Version=1\n";
  for (const auto &l : lines) {
    output_content += l + "\n";
  }

  // Compute the CRC32/JAMCRC checksum.
  uint32_t crc = ComputeJamCrc(output_content);

  std::ofstream outfile(file_path, std::ios::binary);
  if (!outfile.is_open()) {
    throw std::runtime_error("Failed to open file for writing: " + file_path);
  }

  outfile << output_content;
  outfile.write(reinterpret_cast<const char *>(&crc), sizeof(crc));
  outfile.close();
  std::cerr << "Info: Wrote " << lines.size() << " favorites.\n";
}

void PrintUsage() {
  std::cout << "Usage: anbernic_favorites_tool [--sort|--shuffle] [FILE]\n"
            << "\nOptions:\n"
            << "  --sort        Sort the favorites list (default)\n"
            << "  --shuffle     Shuffle the favorites list\n"
            << "  --help        Display this help message\n"
            << "\nIf FILE is not provided, the default "
               "\"/mnt/data/misc/.favorite\" is used.\n"
            << "The file is modified in-place; no backups are made. Use at "
               "your own risk!\n";
}

int main(int argc, char *argv[]) {
  std::string file_path = "/mnt/data/misc/.favorite";
  Operation operation = Operation::kSort;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help") {
      PrintUsage();
      return 0;
    } else if (arg == "--shuffle") {
      operation = Operation::kShuffle;
    } else if (arg == "--sort") {
      operation = Operation::kSort;
    } else {
      file_path = arg;
    }
  }

  try {
    ShuffleOrSortFavorites(file_path, operation);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
