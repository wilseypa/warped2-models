#include <fstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_map>

#include "re2/re2.h"
#include "tclap/ValueArg.h"
#include "utility/memory.hpp"
#include "warped.hpp"

#include "Component.hpp"
#include "DFlipFlop.hpp"
#include "InputObject.hpp"
#include "LogicGate.hpp"

std::string get_file_contents(std::string filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return (contents);
    }
    throw std::runtime_error("Cannot open bench file");
}

int main(int argc, char const* argv[]) {
    unsigned int propagation_delay = 1;
    unsigned int clock_period = 1000;
    std::string bench_file_name = "";

    TCLAP::ValueArg<int> delay_arg("", "propagation-delay",
                                   "the time for a signal to travel between components",
                                   false, propagation_delay, "int");
    TCLAP::ValueArg<int> clock_period_arg("", "clock-period", "the delay between clock ticks",
                                          false, clock_period, "int");
    TCLAP::ValueArg<std::string> bench_file_name_arg("f", "bench-file", "bench file name",
                                                     true, bench_file_name, "file");

    std::vector<TCLAP::Arg*> cmd_line_args = {&delay_arg, &clock_period_arg, &bench_file_name_arg};

    warped::Simulation simulation {"ISCAS Simulation", argc, argv, cmd_line_args};

    propagation_delay = delay_arg.getValue();
    clock_period = clock_period_arg.getValue();
    bench_file_name = bench_file_name_arg.getValue();

    std::unordered_map<std::string, std::unique_ptr<Component>> components_by_name;

    auto contents = get_file_contents(bench_file_name);
    re2::StringPiece input(contents);

    // Create inputs
    std::string group1;
    std::string group2;
    re2::RE2 regex1("INPUT\\(([^)]+)\\)");
    while (RE2::FindAndConsume(&input, regex1, &group1)) {
        components_by_name.emplace(group1, make_unique<InputObject>(group1, propagation_delay,
                                                                    clock_period));
    }

    // Create unconnected components
    input = re2::StringPiece(contents);
    re2::RE2 regex2("(\\w+) = (\\w+)");
    while (RE2::FindAndConsume(&input, regex2, &group1, &group2)) {
        if (group2 == "DFF") {
            components_by_name.emplace(group1, make_unique<DFlipFlop>(
                                           group1, propagation_delay, clock_period));
        } else if (group2 == "NOT") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::NOT));
        } else if (group2 == "AND") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::AND));
        } else if (group2 == "OR") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::OR));
        } else if (group2 == "XOR") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::XOR));
        } else if (group2 == "NAND") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::NAND));
        } else if (group2 == "NOR") {
            components_by_name.emplace(group1, make_unique<LogicGate>(
                                           group1, propagation_delay, clock_period,
                                           LogicGate::Type::NOR));
        } else {
            std::cout << "Invalid component type: " << group2 << '\n';
        }
    }

    // // Connect all components
    input = re2::StringPiece(contents);
    re2::RE2 regex3("(\\w+) = \\w+\\(([^)]+)\\)");
    re2::RE2 regex4("(\\w+)");
    while (RE2::FindAndConsume(&input, regex3, &group1, &group2)) {
        re2::StringPiece input2(group2);
        std::string arg;
        while (RE2::FindAndConsume(&input2, regex4, &arg)) {
            Component::input_index_t input_index = 0;
            if (auto logic_gate = dynamic_cast<LogicGate*>(components_by_name.at(group1).get())) {
                input_index = logic_gate->addInput();
            }
            components_by_name[arg]->addOutput(group1, input_index);
        }
    }


    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& it : components_by_name) {
        object_pointers.push_back(it.second.get());
    }

    simulation.simulate(object_pointers);

    return 0;
}