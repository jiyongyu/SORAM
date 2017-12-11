
import commands, glob, os

# ------------------------------------------------------------------------------
# Make params
# ------------------------------------------------------------------------------

CXX = "g++"
CXXFLAGS = ["-Wall", "-Wextra", "-Wno-unused"]
LDFLAGS = ["-lrt", "-lcryptopp", "-pthread"]

#rm ecb; g++ -g3 -ggdb -O0 -Wall -Wextra -Wno-unused -o ecb enc_ecb_test.cpp -lcryptopp -pthread; ./ecb

TARGET = "virtual_ascend"
SRC = ["util.cpp", "cache.cpp", "dram.cpp", "virtual_ascend.cpp", "sys/ktiming.c", "globals.cpp", "encryption.cpp"]

# ------------------------------------------------------------------------------
# Cachegrind params
# ------------------------------------------------------------------------------

# Nehalem 980 L1/L2
# DO NOT CHANGE unless you also change defs.h
RUN_VALGRIND = "valgrind --tool=cachegrind --I1=32768,4,64 --D1=32768,8,64 --L2=8388608,16,64 ./virtual_ascend"
LOG_FILE = "cache_miss.log"

# Check the log at these locations for cache misses
# assumption: valgrind lists misses in the following order:
#             Ir I1mr ILmr        Dr    D1mr  DLmr        Dw  D1mw  DLmw
#             0  1    2           3     4     5           6   7     8
CHECK_AT_INDEX = [2, 5, 8]
WHITE_LIST = ["dram_read", "dram_write", "cache_initialize"]
THRESHOLD = 10

# ------------------------------------------------------------------------------
# Other constants
# ------------------------------------------------------------------------------

FAIL = "FAIL"
PASS = "PASS"

SUPRESS_OUTPUT = "SUPRESS_OUTPUT"

# ------------------------------------------------------------------------------
# Helpers
# ------------------------------------------------------------------------------

GLOBALS = {}
GLOBALS[SUPRESS_OUTPUT] = False

def run_command(cmd, kill_on_fail=True):
   if GLOBALS[SUPRESS_OUTPUT] is False: 
      print cmd.strip()
   ret, txt = commands.getstatusoutput(cmd)
   if txt != '' and GLOBALS[SUPRESS_OUTPUT] is False: 
      print txt.strip()
   if ret != 0 and kill_on_fail:
      print FAIL
      assert False
   return txt.strip()

def build_param_list(dic):
   return ["-D" + k + "=" + str(v) for k,v in dic.items()]

# ------------------------------------------------------------------------------
# Make functions
# ------------------------------------------------------------------------------

def make_clean():
   run_command("rm -f " + TARGET)
   run_command("rm -f *.o")

def make_debug(dflags=[]):
   oflags = ["-O0", "-g", "-DDEBUG=2"] + dflags
   return make_target(oflags)

def make_cachegrind(dflags=[]):
   oflags = ["-O2", "-g"] + dflags
   return make_target(oflags)

def make_performance(dflags=[]):
   oflags = ["-O2", "-DTIME=1"] + dflags
   return make_target(oflags)

def make_target(oflags=[]):
   if len(oflags) == 0: oflags = ["-O0", "-g", "-DDEBUG=1"]
   return run_command(  " ".join([CXX, "-o", TARGET] + \
                        CXXFLAGS + oflags + SRC + LDFLAGS))

def run_target():
   return run_command("./" + TARGET)

# ------------------------------------------------------------------------------
# Grind functions
# ------------------------------------------------------------------------------

def grind_clean():
   run_command("rm -f cachegrind*")

def grind_run(args=[]):
   grind_clean()
   make_clean()
   args += ["-DCACHE_GRIND"]
   make_cachegrind(args)
   run_command(RUN_VALGRIND)
   # get the verbose report
   cgfiles = glob.glob("cachegrind.*")
   assert len(cgfiles) == 1
   run_command("cg_annotate " + cgfiles[0] + " > " + LOG_FILE)
   run_command("cat " + LOG_FILE)

def grind_check():
   # get the verbose report
   cgfiles = glob.glob("cachegrind.*")
   assert len(cgfiles) == 1
   run_command("cg_annotate " + cgfiles[0] + " > " + LOG_FILE)
   run_command("cat " + LOG_FILE)
   print "************ LOG_FILE end ***************"
   fails = 0
   # check for security problems
   for line in open(LOG_FILE, "r+").readlines():
      line = line.strip()
      if "AssertionFailure" in line or FAIL in line:
         print FAIL + ": Assertion problem with line: " + line
         assert False
      stat_line = filter(lambda x: len(x) > 0, line.split(" ")) # filter out spaces
      # is this a line with useful data?
      if len(stat_line) == 10:
         violations = []
         for i in range(0, len(stat_line) - 1):
            cnum = -1
            try: cnum = int(stat_line[i].replace(",",""))
            except:
               print "Skipping line: " + line
               break
            assert cnum >= 0
            if cnum > THRESHOLD and i in CHECK_AT_INDEX: violations += [cnum]
         clabel = stat_line[-1]
         # was there an LLC cache miss violation?
         if len(violations) > 0:
            is_in_whitelist = [wle in clabel for wle in WHITE_LIST] != \
                              [False] * len(WHITE_LIST)
            if is_in_whitelist:
               print "Benign cache misses in: " + line + ": Violations: " + str(violations)
            else:
               # import pdb; pdb.set_trace()
               print FAIL + ": " + line + ": Violations: " + str(violations)
               fails += 1
   return fails

def grind_verbose():
   cgfiles = glob.glob("cachegrind.*")
   assert len(cgfiles) == 1
   run_command("cg_annotate " + cgfiles[0] + " " + os.getcwd() + "/" + "*.cpp > " + LOG_FILE)
   run_command("cat " + LOG_FILE)


