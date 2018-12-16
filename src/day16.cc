// Wrong answer: 33
// Wrong answer: 2

#include "puzzles.h"

#include <array>
#include <iostream>
#include <numeric>
#include <optional>
#include <set>
#include <vector>

namespace {

enum class Op : std::int8_t {
  kAddr,
  kAddi,
  kMulr,
  kMuli,
  kBanr,
  kBani,
  kBorr,
  kBori,
  kSetr,
  kSeti,
  kGtir,
  kGtri,
  kGtrr,
  kEqir,
  kEqri,
  kEqrr,
};

constexpr Op& operator++(Op& op) {
  return op = static_cast<Op>(static_cast<int>(op) + 1);
}

using Register = std::int16_t;
using Registers = std::array<Register, 4>;

template <typename T>
struct Instruction {
  T op;
  std::int8_t a, b, c;
};

struct Sample {
  Instruction<std::int8_t> instruction;
  Registers before, after;
};

// map[op][x] where op is the Op and x is the code.
using Mapping = std::array<std::array<bool, 16>, 16>;

// map[x] is the op which is encoded by x.
using Assignment = std::array<Op, 16>;

std::ostream& operator<<(std::ostream& output, Op op) {
  switch (op) {
    case Op::kAddr:
      return output << "addr";
    case Op::kAddi:
      return output << "addi";
    case Op::kMulr:
      return output << "mulr";
    case Op::kMuli:
      return output << "muli";
    case Op::kBanr:
      return output << "banr";
    case Op::kBani:
      return output << "bani";
    case Op::kBorr:
      return output << "borr";
    case Op::kBori:
      return output << "bori";
    case Op::kSetr:
      return output << "setr";
    case Op::kSeti:
      return output << "seti";
    case Op::kGtir:
      return output << "gtir";
    case Op::kGtri:
      return output << "gtri";
    case Op::kGtrr:
      return output << "gtrr";
    case Op::kEqir:
      return output << "eqir";
    case Op::kEqri:
      return output << "eqri";
    case Op::kEqrr:
      return output << "eqrr";
    default:
      return output << "badop";
  }
}

constexpr Register CheckBounds(int x) {
  assert(std::numeric_limits<Register>::min() <= x);
  assert(x <= std::numeric_limits<Register>::max());
  return static_cast<Register>(x);
}

Registers Run(Instruction<Op> instruction, Registers registers) {
  auto [op, a, b, c] = instruction;
  switch (op) {
    case Op::kAddr:
      registers[c] = CheckBounds(registers[a] + registers[b]);
      return registers;
    case Op::kAddi:
      registers[c] = CheckBounds(registers[a] + b);
      return registers;
    case Op::kMulr:
      registers[c] = CheckBounds(registers[a] * registers[b]);
      return registers;
    case Op::kMuli:
      registers[c] = CheckBounds(registers[a] * b);
      return registers;
    case Op::kBanr:
      registers[c] = CheckBounds(registers[a] & registers[b]);
      return registers;
    case Op::kBani:
      registers[c] = CheckBounds(registers[a] & b);
      return registers;
    case Op::kBorr:
      registers[c] = CheckBounds(registers[a] | registers[b]);
      return registers;
    case Op::kBori:
      registers[c] = CheckBounds(registers[a] | b);
      return registers;
    case Op::kSetr:
      registers[c] = CheckBounds(registers[a]);
      return registers;
    case Op::kSeti:
      registers[c] = CheckBounds(a);
      return registers;
    case Op::kGtir:
      registers[c] = a > registers[b] ? 1 : 0;
      return registers;
    case Op::kGtri:
      registers[c] = registers[a] > b ? 1 : 0;
      return registers;
    case Op::kGtrr:
      registers[c] = registers[a] > registers[b] ? 1 : 0;
      return registers;
    case Op::kEqir:
      registers[c] = a == registers[b] ? 1 : 0;
      return registers;
    case Op::kEqri:
      registers[c] = registers[a] == b ? 1 : 0;
      return registers;
    case Op::kEqrr:
      registers[c] = registers[a] == registers[b] ? 1 : 0;
      return registers;
    default:
      assert(false);
  }
}

Instruction<std::int8_t> GetInstruction(std::size_t offset) {
  Instruction<std::int8_t> instruction;
  instruction.op = strtol(kPuzzle16.data() + offset, nullptr, 10);
  offset = kPuzzle16.find(' ', offset);
  assert(offset != std::string_view::npos);
  instruction.a = kPuzzle16[offset + 1] - '0';
  assert(0 <= instruction.a && instruction.a < 4);
  instruction.b = kPuzzle16[offset + 3] - '0';
  assert(0 <= instruction.a && instruction.a < 4);
  instruction.c = kPuzzle16[offset + 5] - '0';
  assert(0 <= instruction.a && instruction.a < 4);
  return instruction;
}

std::vector<Sample> GetSamples() {
  std::vector<Sample> samples;
  for (auto offset = kPuzzle16.find("Before: [");
       offset != std::string_view::npos;
       offset = kPuzzle16.find("Before: [", offset)) {
    // std::cout << "Reading record:\n" << kPuzzle16.substr(offset, 55) <<
    // "...\n";
    Sample sample = {};
    // Read the before state.
    for (int i = 0; i < 4; i++) {
      sample.before[i] = kPuzzle16[offset + 9 + 3 * i] - '0';
      assert(0 <= sample.before[i] && sample.before[i] < 4);
    }
    // Read the instruction.
    offset = kPuzzle16.find('\n', offset);
    sample.instruction = GetInstruction(offset);
    // Read the after state.
    offset = kPuzzle16.find("After:  [", offset);
    assert(offset != std::string_view::npos);
    for (int i = 0; i < 4; i++) {
      sample.after[i] = kPuzzle16[offset + 9 + 3 * i] - '0';
      assert(0 <= sample.after[i] && sample.after[i] < 4);
    }
    // std::cout << "Sample: [" << int{sample.before[0]} << ", "
    //          << int{sample.before[1]} << ", " << int{sample.before[2]} << ",
    //          "
    //          << int{sample.before[3]} << "] -> [" << int{sample.after[0]}
    //          << ", " << int{sample.after[1]} << ", " << int{sample.after[2]}
    //          << ", " << int{sample.after[3]} << "] (??? "
    //          << int{sample.instruction.a} << " " << int{sample.instruction.b}
    //          << " " << int{sample.instruction.c} << ")\n";
    // std::cin.get();
    samples.push_back(sample);
  }
  return samples;
}

std::vector<Instruction<Op>> GetProgram(const Assignment& assignment) {
  auto offset = kPuzzle16.find("\n\n\n\n");
  assert(offset != std::string_view::npos);
  offset += 4;
  std::vector<Instruction<Op>> program;
  do {
    Instruction<std::int8_t> raw_instruction = GetInstruction(offset);
    program.push_back(Instruction<Op>{assignment[raw_instruction.op],
                                      raw_instruction.a, raw_instruction.b,
                                      raw_instruction.c});
    offset = kPuzzle16.find('\n', offset + 1);
  } while (offset != kPuzzle16.length() - 1);
  return program;
}

// Search for an assignment for each of the codes [prefix, 16) which is
// consistent with the options that were ruled out. The assignment will only use
// a permutation of ops already found in [prefix, 16).
void FindAllAssignmentsImpl(const Mapping& ruled_out, Assignment& assignment,
                           int prefix, std::set<Assignment>* output) {
  if (prefix == 16) {
    output->insert(assignment);
    return;
  }
  // Pick an assignment for the code at assignment[prefix].
  for (int i = prefix; i < 16; i++) {
    std::swap(assignment[prefix], assignment[i]);
    if (ruled_out[static_cast<int>(assignment[prefix])][prefix]) continue;
    FindAllAssignmentsImpl(ruled_out, assignment, prefix + 1, output);
  }
}

std::set<Assignment> FindAllAssignments(const Mapping& ruled_out) {
  Assignment assignment;  // map from opcode to op.
  std::iota(begin(assignment), end(assignment), Op{0});
  std::set<Assignment> results;
  FindAllAssignmentsImpl(ruled_out, assignment, 0, &results);
  return results;
}

}  // namespace

int Solve16A() {
  int count = 0;
  int num = 0;
  for (const Sample& sample : GetSamples()) {
    num++;
    // std::cout << "Sample " << num << ": [" << int{sample.before[0]} << ", "
    //          << int{sample.before[1]} << ", " << int{sample.before[2]} << ",
    //          "
    //          << int{sample.before[3]} << "] -> [" << int{sample.after[0]}
    //          << ", " << int{sample.after[1]} << ", " << int{sample.after[2]}
    //          << ", " << int{sample.after[3]} << "] (??? "
    //          << int{sample.instruction.a} << " " << int{sample.instruction.b}
    //          << " " << int{sample.instruction.c} << ")\n";
    int possible_interpretations = 0;
    for (int i = 0; i < 16; i++) {
      auto op = static_cast<Op>(i);
      Instruction<Op> instruction{op, sample.instruction.a,
                                  sample.instruction.b, sample.instruction.c};
      auto result = Run(instruction, sample.before);
      if (result == sample.after) {
        // std::cout << "Sample " << num << " works for op=" << op << "\n";
        possible_interpretations++;
      } else {
        // std::cout << "Sample " << num << " does not work for op=" << op <<
        // "\n";
      }
    }
    if (possible_interpretations >= 3) count++;
  }
  return count;
}

int Solve16B() {
  // If we assume that operation op has code x and we subsequently see a sample
  // with code x that doesn't agree with what op should produce, we know that
  // our assumption was wrong. We can use this to rule out possible assignments.

  // ruled_out[op][x] is true if op can't have code x.
  Mapping ruled_out = {};

  auto samples = GetSamples();

  // Draw observations from the samples.
  for (const Sample& sample : samples) {
    for (int i = 0; i < 16; i++) {
      Instruction<Op> instruction{static_cast<Op>(i), sample.instruction.a,
                                  sample.instruction.b, sample.instruction.c};
      if (Run(instruction, sample.before) != sample.after) {
        ruled_out[i][sample.instruction.op] = true;
      }
    }
  }

  auto assignments = FindAllAssignments(ruled_out);
  assert(assignments.size() == 1);
  auto assignment = *assignments.begin();

#ifndef NDEBUG
  // Check that the assignment is consistent.
  for (const Sample& sample : samples) {
    Instruction<Op> instruction{assignment[sample.instruction.op],
                                sample.instruction.a, sample.instruction.b,
                                sample.instruction.c};
    assert(Run(instruction, sample.before) == sample.after);
  }
#endif  // NDEBUG

  // std::cout << "Assignment:\n";
  // for (const Assignment& assignment : assignments) {
  //   for (int i = 0; i < 4; i++) {
  //     for (int j = 0; j < 4; j++) {
  //       int id = 4 * i + j;
  //       std::cout << "[" << id << "] = " << assignment[id] << ",\t";
  //     }
  //     std::cout << "\n";
  //   }
  // }

  Registers registers = {};
  for (const auto& instruction : GetProgram(assignment))
    registers = Run(instruction, registers);

  return registers[0];
}
