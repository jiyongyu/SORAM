
from util import *

''' 
   Sweep a parameter space to collect performance measurements
   Prerequisites:
   1.) Change all params in defs.h to be -1 by default (or undefined)
'''

# take average to reduce experiment noise
TESTS_PER_CONFIG = 10

MMEM_UTILIZATION_LG = "MMEM_UTILIZATION_LG"
WORKING_SET_LG = "WORKING_SET_LG"
WORK_LG = "WORK_LG"

space = {
   MMEM_UTILIZATION_LG : [1, 2, 3, 4],
   WORKING_SET_LG : range(10, 26),
}

# ------------------------------------------------------------------------------
# Run some tests to make sure we don't have security problems
# ------------------------------------------------------------------------------

'''
params = {}
params[WORKING_SET_LG] = 25
params[WORK_LG] = 25
for mutil in space[MMEM_UTILIZATION_LG]:
   params[MMEM_UTILIZATION_LG] = mutil
   grind_run(build_param_list(params))
   output = grind_check()
   if output > 0: assert False
'''

# ------------------------------------------------------------------------------
# Run the performance tests
# ------------------------------------------------------------------------------

GLOBALS[SUPRESS_OUTPUT] = True

params = {}
params[WORK_LG] = 20
for mutil in space[MMEM_UTILIZATION_LG]:
   params[MMEM_UTILIZATION_LG] = mutil
   for ws in space[WORKING_SET_LG]:
      params[WORKING_SET_LG] = ws
      make_clean()
      make_performance(build_param_list(params))
      avg_slowdown = 0.0
      for test in range(0, TESTS_PER_CONFIG):         
         output = run_target()
         print output
         avg_slowdown += float(output.split(",")[-1])
      avg_slowdown = avg_slowdown / TESTS_PER_CONFIG
      stats = ",".join(output.split(",")[0:4]) + "," + str(avg_slowdown)
      print "stat," + stats

GLOBALS[SUPRESS_OUTPUT] = False

