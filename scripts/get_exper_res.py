import xml.etree.ElementTree as ET
import sys
import os
import csv
import glob
import subprocess
import argparse
from get_exper_brunch_stat import *
BUILDABSPATH = os.path.abspath('../build/')
DATAABSPATH = os.path.abspath('../') + "/data"
SEAHORN_ROOT = '/home/yusen/seawork/seahorn/build_z3_fixed/run'  # Put your seahorn root dir here
FILE_DICT = {
    "": "seahorn.csv",
    "--vac": "seahorn(vac).csv",
    "--horn-bmc-solver=smt-y2": "seahorn(smt-y2).csv",
    "--cex": "seahorn(cex).csv",
    "--cex --horn-bmc-solver=smt-y2": "seahorn(cex, smt-y2).csv",
    "klee": "klee.csv", "symbiotic": "symbiotic.csv", "clam": "clam.csv"}
LLVM_VERSION=14

def extra_to_filename(extra, suffix='.csv', prefix=''):
    '''extra: --a=b --c=d to filename: a.b.c.d.csv'''
    if (len(extra) == 0):
        return f'base{suffix}'
    parts = []
    for flag in extra:
        if flag.startswith('--'):
            flag = flag[2:]
        parts.extend(flag.split('='))
    return f'{prefix}{"_".join(parts)}{suffix}'


def make_build_path(extra, build_path='../build'):
    build_path += extra_to_filename(extra, '', '_') if extra else '_base'
    print(f'[Build Path] {build_path}')
    if not os.path.exists(build_path):
        os.mkdir(build_path)
    global BUILDABSPATH
    BUILDABSPATH = os.path.abspath(build_path)


def get_output_level():
    if args.debug:
        output_level = sys.stdout
    else:
        output_level = subprocess.PIPE
    return output_level


def make_new_cmake_conf():
    use_klee = "ON" if args.klee else "OFF"
    use_symbiotic = "ON" if args.symbiotic else "OFF"
    use_bleeding_edge = "ON" if args.bleed_edge else "OFF"
    use_large_bounds = "ON" if args.large_bounds else "OFF"
    # use_crab = "ON" if "--crab" in extra else "OFF"
    if args.smack_parser:
        use_smack = "ON" if args.smack else "OFF"
        smack_enable_no_mem_split = "ON" if args.mem_no_split else "OFF"
        smack_args = f'-DSEA_ENABLE_SMACK={use_smack} -DSMACK_ENCODING={args.precise} -DSMACK_PROPERTY_CHECK={args.checks}\
        -DSMACK_ENABLE_NO_MEM_SPLIT={smack_enable_no_mem_split}'
    else:
        smack_args = ""
    return f'cmake -DSEA_LINK=llvm-link-{LLVM_VERSION} -DCMAKE_C_COMPILER=clang-{LLVM_VERSION}\
    -DCMAKE_CXX_COMPILER=clang++-{LLVM_VERSION} -DSEA_ENABLE_KLEE={use_klee} {smack_args}\
    -DSEA_WITH_BLEEDING_EDGE={use_bleeding_edge} -DSEA_ENABLE_CRAB=ON\
    -DSEA_USE_LARGE_BOUNDS={use_large_bounds}\
    -DSEA_ENABLE_SYMBIOTIC={use_symbiotic} -DSEAHORN_ROOT={SEAHORN_ROOT} ../ -GNinja'


def read_data_from_xml(res_data):
    xml_file = glob.glob(
        "{path}/**/Test.xml".format(path=BUILDABSPATH), recursive=True)[0]
    tree = ET.parse(xml_file)
    root = tree.getroot()
    testlist = root.find('Testing')
    for test in testlist.findall('Test'):
        bench_name = test.find('Name').text
        res = test.get('Status')
        for name_measure_tag in test.find('Results').findall(
                'NamedMeasurement'):
            if name_measure_tag.attrib['name'] == 'Execution Time':
                timing = name_measure_tag.find('Value').text
        res_data.append([bench_name, timing, res])


def write_data_into_csv(file_name, res_data):
    with open(file_name, 'w+', newline='') as csvfile:
        # create the csv writer object
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(["Name", "Timing", "Result"])
        for row in res_data:
            csvwriter.writerow(row)


def collect_res_from_ctest(file_name):
    res_data = []
    read_data_from_xml(res_data)
    write_data_into_csv(
        "{dir}/{file}".format(dir="../data", file=file_name), res_data)
    print(f'[Results] Please find result csv file at: {os.path.abspath("../data")}')


def run_ctest_for_seahorn():
    print("[SeaHorn] Start making SeaHorn results...")
    set_env = ''
    if extra and len(extra) > 0:
        verify_flags = " ".join(extra)
        print(f'Run SeaHorn with extra configs: {verify_flags} ')
        if "--crab" in extra:
            verify_flags += " --domain " + args.domain
        set_env = f'env VERIFY_FLAGS=\"{verify_flags}\"'
    cmake_conf = make_new_cmake_conf()
    command_lst = ["rm -rf *", cmake_conf, "ninja",
                   f'{set_env} ctest -j 1 -D ExperimentalTest -R . --timeout {args.timeout}']
    make_build_path(extra)
    cddir = "cd " + BUILDABSPATH
    for strcmd in command_lst:
        cddir += " ; " + strcmd
    if args.debug:
        print(f'[Command] {cddir}')
    process = subprocess.Popen(
        '/bin/bash',
        shell=True,
        stdin=subprocess.PIPE,
        stdout=get_output_level(),
        stderr=subprocess.PIPE)
    _ = process.communicate(cddir.encode())
    collect_res_from_ctest(extra_to_filename(extra))
    collect_stat_from_ctest_log(extra_to_filename(extra, suffix='.brunch.csv'),
                                True if "--crab" in extra else False)

def run_ctest_for_clam():
    print("[Clam] Start making Clam results...")
    cmake_conf = make_new_cmake_conf()
    command_lst = ["rm -rf *", cmake_conf, "ninja",
                   f'ctest -j{os.cpu_count()} -D ExperimentalTest -R smack_ --timeout 200']
    make_build_path(["--clam"])
    for strcmd in command_lst:
        cddir += " ; " + strcmd
    if args.debug:
        print(f'[Command] {cddir}')
    process = subprocess.Popen(
        '/bin/bash',
        stdin=subprocess.PIPE,
        stdout=get_output_level())
    _ = process.communicate(cddir.encode())
    collect_res_from_ctest(FILE_DICT["klee"])

def collect_stat_from_ctest_log(outfile, use_crab):
    test_tmpdir = os.path.join(BUILDABSPATH, 'Testing', 'Temporary')
    logfiles = [i for i in os.listdir(test_tmpdir)if os.path.isfile(
        os.path.join(test_tmpdir, i)) and i.startswith("LastTest_")]
    latest_log = logfiles[0]
    print("collecting brunch stat from " + logfiles[0])
    if args.seahorn:
        output_file_name = f'AI4BMC_{args.domain}' if "--crab" in extra else "SEABMC"
        output_dir_name = 'y2' if os.getenv('VERIFY_FLAGS') else 'z3'
        vstte_log_path = os.path.join(os.path.abspath('../'), 'res/vstte/outputs', output_dir_name)
        os.makedirs(vstte_log_path, exist_ok=True)
        file_ext_name = 'Yices2' if os.getenv('VERIFY_FLAGS') else 'Z3'
        new_log_name = f'{output_file_name}_{file_ext_name}.log'
        vstte_log_path = os.path.join(vstte_log_path, new_log_name)
        copy_ctest_results(os.path.join(test_tmpdir, latest_log), vstte_log_path)
        vstte_log_path = os.path.join(vstte_log_path, latest_log)
        data = read_brunchstat_from_log(os.path.join(test_tmpdir, latest_log))
        new_csv_name = f'{output_file_name}_{file_ext_name}.csv'
        vstte_csv_path = os.path.join(os.path.abspath('../'), 'res/vstte/data', new_csv_name)
        write_brunchstat_into_csv(data, vstte_csv_path)
    elif args.symbiotic:
        data = read_symbiotic_bruchstat_from_log(os.path.join(
            test_tmpdir, latest_log), BUILDABSPATH, args.timeout)
        outpath = os.path.join(DATAABSPATH, outfile)
        write_symbiotic_bruchstat_into_csv(data, outpath)

def copy_ctest_results(source, destination):
    import shutil
    if args.debug:
        print(f'[Copy] {source} to {destination}')
    if os.path.exists(source):
        shutil.copy(source, destination)
    elif args.debug:
        print(f"Source file does not exist: {source}")

def run_ctest_for_klee():
    cmake_conf = make_new_cmake_conf()
    command_lst = ["rm -rf *", cmake_conf, "ninja",
                   f'ctest -j{os.cpu_count()} -D ExperimentalTest -R klee_ --timeout {args.timeout}']
    print("[KLEE] Start making KLEE results...")
    make_build_path(["--klee"])
    cddir = "cd " + BUILDABSPATH
    for strcmd in command_lst:
        cddir += " ; " + strcmd
    if args.debug:
        print(f'[Command] {cddir}')
    process = subprocess.Popen(
        '/bin/bash',
        stdin=subprocess.PIPE,
        stdout=get_output_level())
    _ = process.communicate(cddir.encode())
    collect_res_from_ctest(FILE_DICT["klee"])


def run_ctest_for_smack():
    cmake_conf = make_new_cmake_conf()
    command_lst = ["rm -rf *", cmake_conf, "ninja",
                   f'ctest -j{os.cpu_count()} -D ExperimentalTest -R smack_ --timeout 200']
    print("[SMACK] Start making SMACK results...")
    make_build_path(["--smack"])
    cddir = "cd " + BUILDABSPATH
    for strcmd in command_lst:
        cddir += " ; " + strcmd
    if args.debug:
        print(f'[Command] {cddir}')
    process = subprocess.Popen(
        '/bin/bash',
        stdin=subprocess.PIPE,
        stdout=get_output_level())
    _ = process.communicate(cddir.encode())
    mem_split = 'mem_no_split' if args.mem_no_split else 'mem_split'
    collect_res_from_ctest(extra_to_filename(
        [str(args.precise), str(args.checks), mem_split]))

def run_ctest_for_symbiotic():
    cmake_conf = make_new_cmake_conf()
    command_lst = ["rm -rf *", cmake_conf, "ninja",
                   f'ctest -j{os.cpu_count()} -D ExperimentalTest -R symbiotic_ --timeout {args.timeout}']
    print("Start making Symbiotic results...")
    cddir = "cd " + BUILDABSPATH
    for strcmd in command_lst:
        cddir += " ; " + strcmd
    if args.debug:
        print(cddir)
    process = subprocess.Popen(
        '/bin/bash',
        stdin=subprocess.PIPE,
        stdout=get_output_level())
    _ = process.communicate(cddir.encode())
    collect_stat_from_ctest_log(FILE_DICT["symbiotic"])

def main():
    os.makedirs(DATAABSPATH, exist_ok=True)
    os.makedirs(BUILDABSPATH, exist_ok=True)
    if args.seahorn:
        run_ctest_for_seahorn()
    if args.klee:
        run_ctest_for_klee()
    if args.smack_parser:
        run_ctest_for_smack()
    if args.symbiotic:
        run_ctest_for_symbiotic()
    if args.clam:
        run_ctest_for_clam()


if __name__ == "__main__":
    # Args for this script
    parser = argparse.ArgumentParser(
        description='Present flags to decide which tool will be tested.')
    parser.add_argument('--seahorn', action='store_true', default=True)
    parser.add_argument('--seahorn-root', dest="seahorn_root", type=str, default=SEAHORN_ROOT)
    parser.add_argument('--klee', action='store_true', default=False)
    parser.add_argument('--symbiotic', action='store_true', default=False)
    parser.add_argument('--clam', action='store_true', default=False)
    parser.add_argument(
        "--domain", type=str,
        choices=["fixed-tvpi-dbm", "zones", "pk-pplite", "pk"],
        dest='domain', default="zones",
        help="Choose an abstract domain: fixed-tvpi-dbm, zones, pk-pplite, or pk.",
    )
    parser.add_argument('--bleed_edge', action='store_true', default=False)
    parser.add_argument('--debug', action='store_true', default=False)
    parser.add_argument('--timeout', type=int, default=2000,
                        help='Seconds before timeout for each test')
    parser.add_argument('--large_bounds', action='store_true', default=False)
    subparsers = parser.add_subparsers(
        help='sub-commands help', dest="smack_parser")
    smack_parser = subparsers.add_parser('smack', help="smack help")
    smack_parser.add_argument('--smack', action='store_true', default=False)
    smack_parser.add_argument(
        '--precise',
        type=str,
        default='unbounded-integer',
        help='Set Smack precise: unbounded-integer or bit vector')
    smack_parser.add_argument(
        '--checks',
        type=str,
        default='assertions',
        help='Set Smack property checks: assertions or valid-deref')
    smack_parser.add_argument(
        '--mem_no_split',
        action='store_true',
        default=False,
        help='Set Smack uses no memory split')
    args, extra = parser.parse_known_args()
    if args.klee:
        args.seahorn = False
    if args.smack_parser:
        args.seahorn = False
    if args.symbiotic:
        args.seahorn = False
    if args.seahorn_root:
        SEAHORN_ROOT = args.seahorn_root # reset to new root path
    main()