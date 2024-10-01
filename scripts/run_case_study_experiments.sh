#!/bin/bash

# Set the seahorn directory
TOOL_DIR=$1
# Set the debug level (info, debug)
DEBUG_LEVEL=${2:-info}  # Default to info if not provided

# by default, run: ./run_case_study_experiments.sh $SEAHORN_ROOT

# Cleanup all previous runs
# ./clean_up.sh

# Function to print debug messages
debug() {
    if [ "$DEBUG_LEVEL" == "debug" ]; then
        echo "$@"
    fi
}

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
PARENT_DIR=$(dirname "$SCRIPT_DIR")
VMCAI_RES_DIR="$PARENT_DIR/res/vmcai"
if [ ! -d "$VMCAI_RES_DIR" ]; then
    mkdir -p "$VMCAI_RES_DIR"
fi

mkdir -p /tmp/results/case_study

DIVIDER="\n================================================\n\n"
SEABMC="SEABMC"
AIBMC="AI4BMC"
Z3="Z3"
Y2="Yices2"

# Function to run experiments
run_experiment() {
    local solver=$1
    local tool=$2
    local extra_flags=$3
    printf $DIVIDER
    echo "Running $tool with $solver..."
    printf $DIVIDER
    if [ "$solver" == "$Z3" ]; then
        if [ "$DEBUG_LEVEL" == "debug" ]; then
            python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
        else
            python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --large_bounds $extra_flags
        fi
        # python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
    elif [ "$solver" == "$Y2" ]; then
        if [ "$DEBUG_LEVEL" == "debug" ]; then
            env VERIFY_FLAGS="--horn-bmc-logic=QF_AUFBV --horn-bmc-solver=smt-y2" python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
        else
            env VERIFY_FLAGS="--horn-bmc-logic=QF_AUFBV --horn-bmc-solver=smt-y2" python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --large_bounds $extra_flags
        fi
    fi
    printf $DIVIDER
    echo "Done running $tool with $solver."
    printf $DIVIDER
}

# Run experiments
run_experiment $Z3 $SEABMC ""
run_experiment $Z3 $AIBMC "--crab"
run_experiment $Y2 $SEABMC ""
run_experiment $Y2 $AIBMC "--crab"

# Gather results
printf "\n\n================================================\n"
echo "                 STATISTICS                 "
printf "================================================\n\n"
python3 get_paper_results.py > $VMCAI_RES_DIR/paper_results/results.txt
# For more detailed results, use the following command
# python3 get_paper_results.py --details