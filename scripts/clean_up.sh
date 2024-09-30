#!/bin/bash
# Get the full path of the directory containing the script
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
PARENT_DIR=$(dirname "$SCRIPT_DIR")
DEBUG_LEVEL=${1:-info}  # Default to info if not provided
echo "Cleanup results for the last run ..."

# Function to print debug messages
debug() {
  if [ "$DEBUG_LEVEL" == "debug" ]; then
    echo "$@"
  fi
}

OUTPUT_DIR_LST=(
  "${PARENT_DIR}/res/vmcai/outputs/z3"
  "${PARENT_DIR}/res/vmcai/outputs/y2"
)

DATA_LST=(
    "${PARENT_DIR}/res/vmcai/data/AI4BMC_Z3.csv"
    "${PARENT_DIR}/res/vmcai/data/AI4BMC_Yices2.csv"
    "${PARENT_DIR}/res/vmcai/data/SEABMC_Z3.csv"
    "${PARENT_DIR}/res/vmcai/data/SEABMC_Yices2.csv"
)

paper_res="${PARENT_DIR}/res/vmcai/paper_results"

for outdir in "${OUTPUT_DIR_LST[@]}"; do
    debug "rm -rf $outdir"
    rm -rf $outdir
done

for data in "${DATA_LST[@]}"; do
    debug "rm -f $data"
    rm -f $data
done

debug "Remove paper results on $paper_res"
find "$paper_res" -mindepth 1 ! -name ".gitkeep" -exec rm -rf {} +