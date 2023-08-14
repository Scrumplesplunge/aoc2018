#include "puzzles.h"

#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <set>
#include <vector>

namespace {

enum class Op : std::int8_t {
  kAddr, kAddi,
  kMulr, kMuli,
  kBanr, kBani,
  kBorr, kBori,
  kSetr, kSeti,
  kGtir, kGtri, kGtrr,
  kEqir, kEqri, kEqrr,
};

constexpr Op& operator++(Op& op) {
  return op = static_cast<Op>(static_cast<int>(op) + 1);
}

using Register = std::int16_t;
using Registers = std::array<Register, 4>;

template <typename T>
struct Instruction { T op; std::int8_t a, b, c; };

struct Sample {
  Instruction<std::int8_t> instruction;
  Registers before, after;
};

// map[op][x] where op is the Op and x is the code.
using Mapping = std::array<std::array<bool, 16>, 16>;

// map[x] is the op which is encoded by x.
using Assignment = std::array<Op, 16>;

constexpr Register CheckBounds(int x) {
  assert(std::numeric_limits<Register>::min() <= x);
  assert(x <= std::numeric_limits<Register>::max());
  return static_cast<Register>(x);
}

Registers Run(Instruction<Op> instruction, Registers r) {
  auto [op, a, b, c] = instruction;
  switch (op) {
    case Op::kAddr: r[c] = CheckBounds(r[a] + r[b]); return r;
    case Op::kAddi: r[c] = CheckBounds(r[a] + b); return r;
    case Op::kMulr: r[c] = CheckBounds(r[a] * r[b]); return r;
    case Op::kMuli: r[c] = CheckBounds(r[a] * b); return r;
    case Op::kBanr: r[c] = CheckBounds(r[a] & r[b]); return r;
    case Op::kBani: r[c] = CheckBounds(r[a] & b); return r;
    case Op::kBorr: r[c] = CheckBounds(r[a] | r[b]); return r;
    case Op::kBori: r[c] = CheckBounds(r[a] | b); return r;
    case Op::kSetr: r[c] = CheckBounds(r[a]); return r;
    case Op::kSeti: r[c] = CheckBounds(a); return r;
    case Op::kGtir: r[c] = a > r[b] ? 1 : 0; return r;
    case Op::kGtri: r[c] = r[a] > b ? 1 : 0; return r;
    case Op::kGtrr: r[c] = r[a] > r[b] ? 1 : 0; return r;
    case Op::kEqir: r[c] = a == r[b] ? 1 : 0; return r;
    case Op::kEqri: r[c] = r[a] == b ? 1 : 0; return r;
    case Op::kEqrr: r[c] = r[a] == r[b] ? 1 : 0; return r;
    default: assert(false);
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
  for (const Sample& sample : GetSamples()) {
    int possible_interpretations = 0;
    for (int i = 0; i < 16; i++) {
      auto op = static_cast<Op>(i);
      Instruction<Op> instruction{op, sample.instruction.a,
                                  sample.instruction.b, sample.instruction.c};
      auto result = Run(instruction, sample.before);
      if (result == sample.after) possible_interpretations++;
    }
    if (possible_interpretations >= 3) count++;
  }
  return count;
}

int Solve16B() {
  auto samples = GetSamples();

  // If we assume that operation op has code x and we subsequently see a sample
  // with code x that doesn't agree with what op should produce, we know that
  // our assumption was wrong. We can use this to rule out possible assignments.
  // ruled_out[op][x] is true if op can't have code x.
  Mapping ruled_out = {};

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

  Registers registers = {};
  for (const auto& instruction : GetProgram(assignment))
    registers = Run(instruction, registers);

  return registers[0];
}
