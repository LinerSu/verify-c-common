import pandas as pd
import numpy as np
import copy, os
from matplotlib.backends.backend_pdf import PdfPages
import seaborn as sns
import matplotlib.pyplot as plt
from termcolor import colored
from scipy.stats import gaussian_kde
import argparse
###########################################
#          Configuration variables        #
###########################################
# do not limit number of rows displayed
pd.set_option('display.max_rows', None)
# Set display options to show all columns
pd.set_option('display.max_columns', None)
# Set a wider max width for columns
pd.set_option('display.width', 1000)
# Left align column headers
pd.set_option('display.colheader_justify', 'center')
# Limit the decimal places to 2 for a cleaner display
pd.set_option('display.float_format', '{:.2f}'.format)
TIMEOUT_THRESHOLD = 900
SHOW_Y2 = False
Y2 = "Yices2" 
Z3 = "Z3"
PROJT_ROOT_DIR = os.path.abspath(os.pardir)
DATA_DIR = f'{PROJT_ROOT_DIR}/res/vmcai/data'
RESULT_DIR = f'{PROJT_ROOT_DIR}/res/vmcai/paper_results'
SHOW_DETAILS = False
# pd.set_option('use_inf_as_na', True)

###########################################
#              Args functions             #
###########################################
def parse_arguments():
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('--details', action='store_true', help='Show more details of the results in table format')
    return parser.parse_args()

def print_centered_title(title, single="*", width=34):
    """
    Prints a centered title within asterisks of the given width.

    :param title: The title to be centered and printed.
    :param width: The total width of the line (default is 34).
    """
    # Calculate the padding needed to center the title
    padding = (width - len(title)) // 2

    # Adjust padding for titles with odd lengths
    if len(title) % 2 != 0 and width % 2 == 0:
        title += " "

    # Print the formatted output
    print(single * width)
    print(" " * padding + title + " " * padding)
    print(single * width)


###########################################
#              Plot functions             #
###########################################
def show_graph(fig, time_plot, a, b, title):
    # # Perform KDE on data
    # values = np.vstack([time_plot[a], time_plot[b]])
    # kernel = gaussian_kde(values)

    # # Evaluate density on the data points
    # time_plot['density'] = kernel(values)
    # print(time_plot['density'])
    # time_plot['size'] = 1 / (time_plot['density'] * 3000)
    # print(time_plot['size'])
    # Assuming 'figsize' is known
    fig_width, fig_height = plt.gcf().get_size_inches()
    # You might base size on some function of plot dimensions
    point_size = (fig_width * fig_height) * 1  # Example formula
    # plt.title(title, fontsize=24)
    x_padding = (time_plot[a].max() - 0) * 0.009  # % padding
    y_padding = (time_plot[b].max() - 0) * 0.009  # % padding
    plt.xlim(0, time_plot[a].max()+ x_padding)
    plt.ylim(0, time_plot[b].max()+ y_padding)
    plt.xlabel(a + ' (second)', fontsize=36);
    plt.ylabel(b + ' (second)', fontsize=36);
    pp_g = sns.scatterplot(x=a, y=b, data=time_plot, s=point_size)
    sns.despine()
    pp_g.set_xticklabels(pp_g.get_xticks(), size=28)
    pp_g.set_yticklabels(pp_g.get_yticks(), size=28)
    x = np.linspace(0, max(time_plot[a].max()+ x_padding, time_plot[b].max()+ x_padding), 2)
    pp_g.plot(x, x)
    pp_g.grid(True)
    pp_g.vlines(x=time_plot[a].max(), ymin=0, ymax=time_plot[b].max(), color='black', linestyle='-', alpha=0.7)
    pp_g.hlines(y=time_plot[b].max(), xmin=0, xmax=time_plot[a].max(), color='black', linestyle='-', alpha=0.7)
    plt.tight_layout()
    # plt.show()

def show_graph_2(time_plot):
  fig_width, fig_height = plt.gcf().get_size_inches()
  # You might base size on some function of plot dimensions
  point_size = (fig_width * fig_height) * 0.75  # Example formula
  # plt.title(title, fontsize=24)
  x_padding = (TIMEOUT_THRESHOLD - 0) * 0.009  # % padding
  y_padding = (TIMEOUT_THRESHOLD - 0) * 0.009  # % padding
  plt.rc('legend', fontsize=20, title_fontsize=26)
  plt.xlim(-x_padding, TIMEOUT_THRESHOLD + x_padding)
  plt.ylim(-y_padding, TIMEOUT_THRESHOLD + y_padding)
  plt.xlabel('AI4BMC (seconds)', fontsize=36)
  plt.ylabel('BMC (seconds)', fontsize=36)
  x = np.linspace(0, TIMEOUT_THRESHOLD, TIMEOUT_THRESHOLD, 2)
  plt.plot(x, x, color='red')
  configurations = {
      'Z3': {'color': 'tab:blue', 'marker': 'o'},
      'Yices2': {'color': 'tab:green', 'marker': 'x'}
  }
  for solver_name, props in configurations.items():
    pp_g = sns.scatterplot(x=time_plot['AI4BMC_' + solver_name], y=time_plot['BMC_' + solver_name], color=props['color'], label=solver_name, alpha=0.8, s=point_size, marker=props['marker'])
  sns.despine()
  pp_g.set_xticks([0, 100, 300, 500, 700, 900])
  pp_g.set_yticks([0, 100, 300, 500, 700, 900])
  pp_g.spines['left'].set_position('zero')
  line_width = pp_g.spines['left'].get_linewidth()
  pp_g.spines['left'].set_bounds(0, TIMEOUT_THRESHOLD)
  pp_g.spines['bottom'].set_position('zero')
  pp_g.spines['bottom'].set_bounds(0, TIMEOUT_THRESHOLD)
  pp_g.set_xticklabels(pp_g.get_xticks(), size=28)
  pp_g.set_yticklabels(pp_g.get_yticks(), size=28)
  # plt.setp(pp_g.get_legend().get_texts(), fontsize=34) # for legend text
  # plt.setp(pp_g.get_legend().get_title(), fontsize='28') # for legend title
  pp_g.legend(title="Solver", alignment='left')
  sns.move_legend(pp_g, "lower right", bbox_to_anchor=(TIMEOUT_THRESHOLD/(TIMEOUT_THRESHOLD + x_padding), y_padding/(TIMEOUT_THRESHOLD + y_padding)))
  # Manually add gridlines up to x_max
  for x_tick, y_tick in zip(pp_g.get_xticks(), pp_g.get_yticks()):
    if x_tick != 0 and x_tick < TIMEOUT_THRESHOLD:
      pp_g.vlines(x_tick, ymin=0, ymax=TIMEOUT_THRESHOLD, color='gray', linewidth=line_width, linestyle='--', alpha=0.7)  # Lighter gridline
    if y_tick != 0 and y_tick < TIMEOUT_THRESHOLD:
      pp_g.hlines(y_tick, xmin=0, xmax=TIMEOUT_THRESHOLD, color='gray', linewidth=line_width, linestyle='--', alpha=0.7)  # Lighter gridline
  # pp_g.grid(True)
  pp_g.vlines(x=TIMEOUT_THRESHOLD, ymin=0, ymax=TIMEOUT_THRESHOLD, color='black', linewidth=line_width, linestyle='-', alpha=0.7)
  pp_g.hlines(y=TIMEOUT_THRESHOLD, xmin=0, xmax=TIMEOUT_THRESHOLD, color='black', linewidth=line_width, linestyle='-', alpha=0.7)
  plt.tight_layout()


def dump_graph(res, res2):
    time_plot1 = res[['job_name', 'BMC', 'AI4BMC']]
    time_plot2 = res2[['job_name', 'BMC', 'AI4BMC']]
    time_plot = pd.merge(time_plot1, time_plot2, on='job_name', suffixes=['_'+(Y2 if SHOW_Y2 else Z3), '_'+(Y2 if not SHOW_Y2 else Z3)])
    # print(time_plot[:1])
    fig = plt.figure(figsize=(15, 12))
    show_graph_2(time_plot)
    pic_path = os.path.join(RESULT_DIR, 'SP_case_study.png')
    plt.savefig(pic_path)
    print(f"Please find the graph in {pic_path}.")

###########################################
#             Column functions            #
###########################################
def rename_time_col(csv, col_name):
  csv['opsem_crab_time'] = csv['opsem_crab_time'].astype(float)
  csv['pp_crab_time'] = csv['pp_crab_time'].astype(float)
  return csv[['job_name', 'opsem_crab_time', 'pp_crab_time']].rename(columns = {'opsem_crab_time': (col_name+' unrolled time (s)'), 'pp_crab_time': (col_name+' orginal time (s)')})

def highlight_cols(s):
    return ['border-right: 2px solid black' if col == 'bmc_solve_time' or col == 'opsem_loc' else '' for col in s.index]

def set_failed_cases(df):
  # df = df.fillna(0)
  df['result'] = np.where(~df['seahorn_total_time'].isna(), "TRUE", df['result'])
  after = df.columns.get_loc('result') + 1
  df.iloc[:, after:] = df.iloc[:, after:].fillna(0)
  # df['result'] = np.where(df['result'] == 0, "FALSE", df['result'])
  return df

def check_timeout_cases_same(df1, df2):
  df1 = pd.DataFrame(df1)
  df2 = pd.DataFrame(df2)
  sorted_df1 = df1.sort_values(by='job_name').reset_index(drop=True)
  sorted_df2 = df2.sort_values(by='job_name').reset_index(drop=True)
  res = sorted_df1['job_name'].equals(sorted_df2['job_name'])
  diff_df1 = sorted_df1[~sorted_df1['job_name'].isin(sorted_df2['job_name'])]
  diff_df2 = sorted_df2[~sorted_df2['job_name'].isin(sorted_df1['job_name'])]
  return res, list(diff_df1['job_name']), list(diff_df2['job_name'])

def clean_format(csv):
    csv['seahorn_total_time'] = csv['seahorn_total_time'].astype(float)
    csv['bmc_solve_time'] = csv['bmc_solve_time'].astype(float)
    csv['bmc_dag_size'] = csv['bmc_dag_size'].astype(int)
    csv['bmc_circuit_size'] = csv['bmc_circuit_size'].astype(int)
    csv['opsem_crab_time'] = csv['opsem_crab_time'].astype(float)
    csv['opsem_crab_range_time'] = csv['opsem_crab_range_time'].astype(float)
    csv['pp_crab_time'] = csv['pp_crab_time'].astype(float)
    csv['pp_crab_range_time'] = csv['pp_crab_range_time'].astype(float)
    return csv

# % of crab time inside total time
def calculate_percentage(row):
    if row['AI4BMC'] > 0:
        return row['AbsInt_time'] / row['AI4BMC'] * 100
    else:
        return pd.NA
    
# Explanation dictionary
explanations = {
    'job_name': 'The name of the benchmark',
    'BMC': 'The total time for BMC',
    'AI4BMC': 'The total time for AI4BMC',
    'total_diff': 'The difference between BMC and AI4BMC (BMC - AI4BMC)',
    'BMC_solve_time': 'The solving time on the solver for BMC',
    'AI4BMC_solve_time': 'The solving time on the solver for AI4BMC',
    'solve_diff': 'The difference between BMC and AI4BMC solving time (BMC_solve_time - AI4BMC_solve_time)',
    'AbsInt_time': 'The total time spent on AbsInt in AI4BMC',
    'AI%': 'The percentage of time spent on AbsInt in AI4BMC',
    '#pp': 'The number of dereference checks solved by the AbsInt in the original program',
    'pp solved%': 'The percentage of dereference checks solved by the AbsInt in the original program',
    '#opsem': 'The number of dereference checks solved by the AbsInt in the unrolled program',
    'opsem solved%': 'The percentage of dereference checks solved by the AbsInt in the unrolled program',
}

# Function to print column names with explanations
def print_column_explanations(explanations):
    if not SHOW_DETAILS:
       return
    print(f"Explanation of columns shown in the tables:")
    max_len = max(len(col) for col in explanations.keys())
    for k, v in explanations.items():
        print(f"{k.ljust(max_len)} : {v}")  
    print('')

# CRAB_CSV = 'crab_pk.csv' # 'crab_pk.csv' 'crab_vstte.csv'

def read_data(csv_name):
   file_path = os.path.join(DATA_DIR, csv_name)
   return pd.read_csv(file_path)

def process_csv(solver_name):
    print(f'- With {solver_name}')
    POSTFIX = '_Yices2' if solver_name == "Yices2" else '_Z3'
    CRAB_RAW_NAME = 'AI4BMC'
    CRAB_NAME = CRAB_RAW_NAME + POSTFIX
    CRAB_CSV = CRAB_NAME + '.csv'
    print(f"-- Reading the result of {CRAB_RAW_NAME} from {CRAB_CSV}")
    crab_csv = set_failed_cases(read_data(CRAB_CSV))
    crab_csv = clean_format(crab_csv)
    BMC_RAW_NAME = 'SEABMC'
    BMC_NAME = BMC_RAW_NAME + POSTFIX
    BMC_CSV = BMC_NAME + '.csv'
    print(f"-- Reading the result of {BMC_RAW_NAME} from {BMC_CSV}")
    bmc_csv = set_failed_cases(read_data(BMC_CSV))
    bmc_csv = clean_format(bmc_csv)

    # Extract columns for further evaluation
    bmc = bmc_csv[['job_name', 'bmc_solve_time', 'seahorn_total_time', 'bmc_circuit_size', 'bmc_dag_size']].rename(columns = {'seahorn_total_time': 'BMC', 'bmc_solve_time': 'BMC_solve_time'})
    crab = crab_csv[['job_name', 'pp_loc', 'opsem_loc', 'pp_crab_time', 'pp_crab_range_time', 'pp.isderef.not.solve', 'pp.isderef.solve', 'opsem_crab_time', 'opsem_crab_range_time', 'opsem.isderef.not.solve', 'opsem.isderef.solve', 'bmc_solve_time', 'seahorn_total_time', 'bmc_circuit_size', 'bmc_dag_size']].rename(columns = {'bmc_solve_time': 'AI4BMC_solve_time', 'seahorn_total_time': 'AI4BMC', 'bmc_circuit_size': 'AI4BMC_circuit_size', 'bmc_dag_size': 'AI4BMC_dag_size'})
    crab['AI4BMC'] = crab['AI4BMC'] + crab['pp_crab_time'] + crab['pp_crab_range_time']
    crab['AbsInt_time'] = crab['pp_crab_time'] + crab['pp_crab_range_time'] + crab['opsem_crab_time'] + crab['opsem_crab_range_time']
    crab['pp solved%'] = round(crab['pp.isderef.solve'] / (crab['pp.isderef.not.solve'] + crab['pp.isderef.solve']) * 100, 0)
    crab['opsem solved%'] = round(crab['opsem.isderef.solve'] / (crab['opsem.isderef.not.solve'] + crab['opsem.isderef.solve']) * 100, 0)
    crab['AI%'] = crab.apply(calculate_percentage, axis=1)
    crab['#pp'] = crab['pp.isderef.not.solve'] + crab['pp.isderef.solve']
    crab['#pp'] = crab['#pp'].astype(int)
    crab['#opsem'] = crab['opsem.isderef.not.solve'] + crab['opsem.isderef.solve']
    crab['#opsem'] = crab['#opsem'].astype(int)
    pd.to_numeric(crab['AI%'], errors='coerce')
    bmc.loc[bmc['job_name'].isin(bmc_csv[bmc_csv['result'].isna()]['job_name']), 'BMC'] = TIMEOUT_THRESHOLD
    crab.loc[crab['job_name'].isin(crab_csv[crab_csv['result'].isna()]['job_name']), 'AI4BMC'] = TIMEOUT_THRESHOLD
    # bmc = bmc[~bmc['job_name'].isin(tmlst)] # exclude timeout cases
    # crab = crab[~crab['job_name'].isin(tmlst)] # exclude timeout cases
    no_is_dref = crab[(crab['pp.isderef.not.solve'] == 0.0) & (crab['pp.isderef.solve'] == 0.0)]
    no_is_dref_lst = no_is_dref['job_name'].to_list()
    print(f'-- The number of cases with no dereference checks: {len(no_is_dref_lst)}. So excluding these cases.')
    bmc = bmc[~bmc['job_name'].isin(no_is_dref_lst)] # exclude no isderef cases
    crab = crab[~crab['job_name'].isin(no_is_dref_lst)] # exclude no isderef cases
    # Merge
    res = pd.merge(bmc, crab, how='inner', on='job_name')

    # How many cases:
    print(f'-- Total cases: {len(res)}')

    is_same, diffcrab, diffbmc = check_timeout_cases_same(crab_csv[crab_csv['result'].isna()]['job_name'], bmc_csv[bmc_csv['result'].isna()]['job_name'])
    #timeout list
    jobs_with_timeout = pd.concat([
        crab_csv[crab_csv['result'].isna()]['job_name'],
        bmc_csv[bmc_csv['result'].isna()]['job_name']
    ]).unique()
    tmlst = jobs_with_timeout.tolist()
    print(f'-- The number of timeout cases for both pipelines is: {len(tmlst)}')
    print(f'\tcases: {tmlst}')
    if is_same:
       print(f"--- Both pipelines have the same timeout cases.")
    if not is_same:
        print(f"--- Each piepeline may have different timeout cases.")
        print(f"\t{CRAB_RAW_NAME} timeout cases: {diffcrab}")
        print(f"\t{BMC_RAW_NAME} timeout cases: {diffbmc}")

    return tmlst, res

def read_csv():
    z3_tmlst, z3_res = process_csv(Z3)
    print('')
    y2_tmlst, y2_res2 = process_csv(Y2)
    dump_graph(z3_res, y2_res2)

    print("\n")
    print_centered_title("PERFORMANCE: FASTER OR SLOWER?", "*", 45)
    z3_details = pre_process_csv(z3_tmlst, z3_res)
    show_performance(Z3, z3_details)
    print('')
    y2_details = pre_process_csv(y2_tmlst, y2_res2)
    show_performance(Y2, y2_details)
    print("\n")
    print_centered_title("PERFORMANCE: ABSINT Time in AI4BMC", "*", 45)
    show_absint_time(Z3, z3_details)
    print('')
    show_absint_time(Y2, y2_details)
    print("\n")
    print_centered_title("PRECISION: ABSINT Solving Rate", "*", 45)
    show_precision(z3_details)
    print("\n")

def print_table(res):
    if SHOW_DETAILS:
        print("-" * 34)
        print(res)
        print("-" * 34)

def pre_process_csv(tmlst, res):
    res = res[~res['job_name'].isin(tmlst)] # exclude timeout cases
    details = res[['job_name', 'pp_loc', 'pp.isderef.not.solve', 'pp.isderef.solve', '#pp', 'pp solved%', 'opsem.isderef.not.solve', 'opsem.isderef.solve', '#opsem', 'opsem solved%', 'opsem_loc', 'BMC_solve_time', 'AI4BMC_solve_time', 'BMC', 'AI4BMC', 'AbsInt_time', 'AI%']].copy()
    details['total_diff'] = details['BMC'] - details['AI4BMC']
    return details

def print_explanation():
    if not SHOW_DETAILS:
       return
    # file_path = os.path.join(DATA_DIR, 'explanation.md')
    file_path = os.path.join(os.getcwd(), 'explanation.md')
    explanation = """
- array_list_swap
```c
len = nd_int();
assume(0 < len && len <= 40);
item_sz = nd_int();
assume(0 < item_sz && item_sz <= 40);
max_size = len * item_sz;
list->data = malloc();

linear 

index_a = nd_int();
assume(0 <= index_a && index_a <= len);
index = index_a * item_sz; // bytes
list->data[index]
```
- hash_c_string
```c
sz = nd_int();
assume(0 < sz && sz <= 40);
alignment = 8;
size_t alloc_sz = (sz + (alignment - 1)) & ~(alignment - 1)
buf = malloc(alloc_sz);
```
- byte_cursor_trim_pred2, byte_cursor_satisfies_pred2	
```c
while (trimmed.len > 0 && predicate(*(trimmed.ptr))) {
    --trimmed.len;
    ++trimmed.ptr;
}
```
- hash_string
```c
const uint32_t *k = (const uint32_t *)key;

while (length > 12)
{
    ...
    length -= 12;
    k += 3;
}
```
Basically, in crab IR, it will becomes:
```lua
_37:
    (@V_15:region(unknown),.046.i.i.i:ref) := gep_ref(@V_15:region(unknown),_38:ref);
    /* .046.i.i.i.address |-> [17, +oo] && .046.i.i.i.offset |-> [16, 16] && .046.i.i.i.size |-> [27, 64] */
    goto _39;
_39:
    goto __@bb_378,__@bb_379;
__@bb_379:
    ...
_50:
    /* .046.i.i.i.address |-> [17, +oo] && .046.i.i.i.offset |-> [16, +oo] && .046.i.i.i.size |-> [27, 64] */
    (@V_15:region(unknown),_53:ref) := gep_ref(@V_15:region(unknown),.046.i.i.i:ref + 8);
    _55 = crab_intrinsic(is_dereferenceable,@V_15:region(unknown),_53:ref,4:int64) 
    goto __@bb_462,__@bb_463;
__@bb_462:
    assume(_55);
    goto _56;
_56:
    (@V_15:region(unknown),_84:ref) := gep_ref(@V_15:region(unknown),.046.i.i.i:ref + 12);
    (@V_15:region(unknown),.046.i.i.i:ref) := gep_ref(@V_15:region(unknown),_84:ref);
    /* .046.i.i.i.offset |-> [28, +oo] */
    goto _39;
```
unless we know `.046.i.i.i.offset - .046.i.i.i.size <= -8`
- hash_callback_c_str_eq
we are limits to reason string
```c
sz = nd_int();
assume(0 < sz && sz <= 40);
char *str = malloc(sz);
str[sz - 1] = \'\\0\';
idx = nd_int();
assume(0<= idx && idx < sz);
str[idx] // is_deref(str + idx, 1)
```
- array list properties has non-linear properties
    ```c
        len = nd_int();
        assume(0 < len && len <= 40);
        item_sz = nd_int();
        assume(0 < item_sz && item_sz <= 40);
        current_sz = nd_int();
        assume(current_sz == item_sz * len);
        array_list = malloc(current_sz);
        idx = nd_int();
        assume(0 <= idx && idx <= len);
        array_list[idx * item_sz]
    ```
"""
    with open(file_path, 'w') as f:
       f.write(explanation)
    print(f"Please find detailed explanation why these cases are hard to prove in {file_path}.")


def show_performance(solver_name, details):
    print(f"- With {solver_name}, total cases: {len(details)}")
    # Know how many cases that total time between two pipelines does not change too much
    no_big_changes = details[(details['total_diff'] > -5) & (details['total_diff'] < 5)]
    print(f"-- There are {len(no_big_changes)} cases with no significant timing changes within 5 seconds. So excluding these cases.")
    big_changes = details[~details['job_name'].isin(no_big_changes['job_name'].tolist())]
    print(f"-- Speed up in total time (when total time difference > 5s):")
    # How speed up in solver solving time
    stats = big_changes[big_changes['total_diff'] > 5].copy()
    time_stats = stats[['BMC','AI4BMC', 'total_diff']]
    time_stats.describe()
    # Percent change
    stats['Speed up%'] = (stats['BMC'] - stats['AI4BMC']) / stats['BMC'] * 100
    stats = stats.sort_values(by='Speed up%', ascending=False)
    stats['solve_diff'] = stats['BMC_solve_time'] - stats['AI4BMC_solve_time']
    print(f"--- The number of cases speed up: {len(stats)}")
    speed_up_details = stats[['job_name', 'Speed up%', 'BMC', 'AI4BMC', 'total_diff', 'BMC_solve_time', 'AI4BMC_solve_time', 'solve_diff', 'AbsInt_time', 'AI%', 'pp solved%', 'opsem solved%']]
    print_table(speed_up_details)
    print(f"--- Speed up >95% cases: {len(speed_up_details['job_name'][speed_up_details['Speed up%'] > 95])}")
    print(f"--- Speed up other cases: {len(speed_up_details['job_name'][(speed_up_details['Speed up%'] <= 95)])}")
    avg_time = speed_up_details['AbsInt_time'].mean()
    print(f"--- Overall, the average time spend on AbsInt time is {avg_time:.2f} seconds.")

    print(f"-- Slowdowns in total time (when total time difference > 5s):")
    # any big slows down on total time
    stats = big_changes[big_changes['total_diff'] < -5].copy()
    # Percent change
    stats['Slow down%'] = (stats['BMC'] - stats['AI4BMC']) / stats['BMC'] * 100
    stats = stats.sort_values(by='Slow down%', ascending=False)
    stats['solve_diff'] = stats['BMC_solve_time'] - stats['AI4BMC_solve_time']
    slow_down_details = stats[['job_name', 'Slow down%', 'BMC', 'AI4BMC', 'total_diff', 'BMC_solve_time', 'AI4BMC_solve_time', 'solve_diff', 'AbsInt_time', 'AI%', 'pp solved%', 'opsem solved%']]
    print_table(slow_down_details)
    print(f"--- Slow down <=50% cases: {len(slow_down_details['job_name'][slow_down_details['Slow down%'] >= -50])}")
    print(f"--- Slow down other cases: {len(slow_down_details['job_name'][slow_down_details['Slow down%'] < -50])}")

def show_absint_time(solver_name, details):
    print(f"- With {solver_name}, total cases: {len(details)}")
    # Are cases that BMC terminates fast that AI4BMC also terminates as quick as BMC can
    edge_cases = details[(details['BMC'] <= 50) & (details['AI4BMC'] > 50)]
    print(f"-- How many cases that total time that BMC spent less than AI4BMC spent: {len(edge_cases)}")
    if SHOW_DETAILS:
        print(f"\tcases: {edge_cases}")
    fast_bmc_cases = details[details['AI4BMC'] <= 50].copy()
    deeper = fast_bmc_cases[['job_name', 'AI4BMC', 'AbsInt_time', 'AI%', 'pp solved%', '#pp', 'opsem solved%', '#opsem']]
    good = deeper[(deeper['AI%'] > 40)] # & ((deeper['pp solved%'] <= 50) & (deeper['opsem solved%'] <= 50))
    print(f"-- How many cases that AI% > 40% when running time <= 50s: {len(good)}")
    avg, maxi = good['AbsInt_time'].mean(), good['AbsInt_time'].max()
    print(f"--- For these cases, the average time for AbsInt is: {avg:.2f} seconds, and the maximum time for AbsInt is: {maxi:.2f} seconds.")
    print_table(good[['job_name', 'AI4BMC', 'AbsInt_time', 'AI%', 'pp solved%', '#pp', 'opsem solved%', '#opsem']])
    not_very_good = deeper[(deeper['AI%'] <= 40)] # & (deeper['AI4BMC'] > 10)
    print(f"-- How many cases that AI% <= 40% when running time <=50s: {len(not_very_good)}")
    avg, maxi = not_very_good['AbsInt_time'].mean(), not_very_good['AbsInt_time'].max()
    print(f"--- For these cases, the average time for AbsInt is: {avg:.2f} seconds, and the maximum time for AbsInt is: {maxi:.2f} seconds.")
    print_table(not_very_good[['job_name', 'AI4BMC', 'AbsInt_time', 'AI%', 'pp solved%', '#pp', 'opsem solved%', '#opsem']])
    slow_bmc_cases = details[details['AI4BMC'] > 50].copy()
    good_on_slow_bmc = slow_bmc_cases[slow_bmc_cases['AI%']<= 40]
    print(f"-- How many cases that AI% <= 40% when running time >50s: {len(good_on_slow_bmc)}")
    avg, maxi = good_on_slow_bmc['AbsInt_time'].mean(), good_on_slow_bmc['AbsInt_time'].max()
    print(f"--- For these cases, the average time for AbsInt is: {avg:.2f} seconds, and the maximum time for AbsInt is: {maxi:.2f} seconds.")
    print_table(good_on_slow_bmc[['job_name', 'AI4BMC', 'AbsInt_time', 'AI%', 'pp solved%', '#pp', 'opsem solved%', '#opsem']])
    print(f"-- How many cases that AI% > 40% when running time > 50s: {len(slow_bmc_cases[slow_bmc_cases['AI%']> 40])}")

def show_precision(details):
    print(f"Precision does not depend on the solver used.")
    print(f"Total cases: {len(details)}")
    pp_most_solved = details[details['pp solved%'] >= 50]
    pp_most_solved = pp_most_solved.sort_values(by='pp solved%', ascending=False)
    print(f'- The number of cases that AbsInt solved >50% checks before loop unrolling: {len(pp_most_solved)}')
    print_table(pp_most_solved[['job_name', '#pp', 'pp solved%', '#opsem', 'opsem solved%', 'AI4BMC', 'AI%', 'total_diff']])
    full_pp_solved = pp_most_solved[pp_most_solved['pp solved%'] == 100]
    print(f"-- with the number of cases solved 100%: {len(full_pp_solved)}")
    print_table(full_pp_solved[['job_name', '#pp', 'pp solved%', '#opsem', 'opsem solved%', 'AI4BMC', 'AI%', 'total_diff']])

    opsem_most_solved = details[(details['pp solved%'] < 50) & (details['opsem solved%'] >= 50)]
    opsem_most_solved = opsem_most_solved.sort_values(by='opsem solved%', ascending=False)
    print(f'- The number of cases that AbsInt solved >50% checks after loop unrolling: {len(opsem_most_solved)}')
    print_table(opsem_most_solved[['job_name', '#pp', 'pp solved%', '#opsem', 'opsem solved%', 'AI4BMC', 'AI%', 'total_diff']])
    full_opsem_solved = opsem_most_solved[opsem_most_solved['opsem solved%'] == 100]
    print(f"-- with the number of cases solved 100%: {len(full_opsem_solved)}")
    print_table(full_opsem_solved[['job_name', '#pp', 'pp solved%', '#opsem', 'opsem solved%', 'AI4BMC', 'AI%', 'total_diff']])

    # The rest: hard to prove
    hard_to_prove = details[(details['pp solved%'] < 50) & ((details['opsem solved%'] < 50) | (details['opsem solved%'].isnull()))]
    hard_to_prove = hard_to_prove.sort_values(by=['pp solved%', 'opsem solved%'], ascending=[True, True])
    print(f'- The number of cases that AbsInt hard to prove: {len(hard_to_prove)}')
    print_table(hard_to_prove[['job_name', '#pp', 'pp solved%', '#opsem', 'opsem solved%', 'BMC', 'AI4BMC', 'AI%', 'total_diff']])
    print_explanation()

if __name__ == "__main__":

    args = parse_arguments()
    if args.details:
        SHOW_DETAILS = True
    print_centered_title("STATISTICS", "=")
    print(f"\nNote that the timeout threshold is {TIMEOUT_THRESHOLD} seconds.\n")
    print_column_explanations(explanations)
    read_csv()
    if not args.details:
        print(f"\nTo show more details, please run the script ({__file__}) with --details option.")
