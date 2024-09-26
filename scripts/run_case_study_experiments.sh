#!/bin/bash

# Set the seahorn directory
TOOL_DIR=$1
# Set the debug level (info, debug)
DEBUG_LEVEL=${2:-info}  # Default to info if not provided

# Function to print debug messages
debug() {
    if [ "$DEBUG_LEVEL" == "debug" ]; then
        echo "$@"
    fi
}

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
PARENT_DIR=$(dirname "$SCRIPT_DIR")
VMCAI_RES_DIR="$PARENT_DIR/res/vmcai"
DATA_DIR="$PARENT_DIR/data"
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
    if [ "$solver" == "$Z3" ]; then
        if [ "$DEBUG_LEVEL" == "debug" ]; then
            python3 get_exper_res.py --seahorn --seahorn-root '$TOOL_DIR' --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
        else
            python3 get_exper_res.py --seahorn --seahorn-root '$TOOL_DIR' --timeout 900 --bleed_edge --large_bounds $extra_flags
        fi
        # python3 get_exper_res.py --seahorn --seahorn-root $TOOL_DIR --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
    elif [ "$solver" == "$Y2" ]; then
        if [ "$DEBUG_LEVEL" == "debug" ]; then
            VERIFY_FLAGS=\"--horn-bmc-logic=QF_AUFBV --horn-bmc-solver=smt-y2\" python3 get_exper_res.py --seahorn --seahorn-root '$TOOL_DIR' --timeout 900 --bleed_edge --debug --large_bounds $extra_flags
        else
            VERIFY_FLAGS=\"--horn-bmc-logic=QF_AUFBV --horn-bmc-solver=smt-y2\" python3 get_exper_res.py --seahorn --seahorn-root '$TOOL_DIR' --timeout 900 --bleed_edge --large_bounds $extra_flags
        fi
    fi
    cp $DATA_DIR/brunch_stat.csv $VMCAI_RES_DIR/${tool}_${solver}.csv
    echo "Done running $tool with $solver."
    printf $DIVIDER
}

# Run experiments
run_experiment $Z3 $SEABMC ""
run_experiment $Z3 $AIBMC "--crab"
run_experiment $Y2 $SEABMC ""
run_experiment $Y2 $AIBMC "--crab"

# Gether results
python3 get_paper_results.py
# For more detailed results, use the following command
# python3 get_paper_results.py --details