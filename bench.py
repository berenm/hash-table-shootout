import sys, os, subprocess, signal

programs = [
    'glib_hash_table',
    'google_dense_hash_map',
    'stx_bplus_tree',
    'bplus_tree2',
    'bplus_tree_generic',
    # 'stl_unordered_map',
    # 'stl_map',
    # 'boost_unordered_map',
    # 'google_sparse_hash_map',
    # 'qt_qhash',
    # 'python_dict',
    # 'ruby_hash',
    # 'glib_hash_table_lite',
    # 'bplus_tree_16',
    # 'bplus_tree_64',
    # 'bplus_tree_128',
    # 'bplus_tree_256',
    # 'bplus_tree_dicho_16',
    # 'bplus_tree_dicho_32',
    # 'bplus_tree_dicho_64',
    # 'bplus_tree_dicho_128',
    # 'bplus_tree_dicho_256',
    # 'bplus_tree2_linear',
    # 'bplus_tree_generic_linear',
]

minkeys  =  64*1024
maxkeys  =   1*1024*1024
interval =  2
best_out_of = 3

#minkeys  = 16000
#maxkeys  = 512*1000
#interval = 2
#best_out_of = 3

# for the final run, use this:

outfile = open('output', 'w')

if len(sys.argv) > 1:
    benchtypes = sys.argv[1:]
else:
    benchtypes = ('random-usage', 'insert', 'delete', 'lookup', 'random-insert', 'random-delete', 'random-lookup')

for benchtype in benchtypes:
    nkeys = minkeys
    while nkeys <= maxkeys:
        for program in programs:
            fastest_attempt = 1000000
            fastest_attempt_data = ''

            for attempt in range(best_out_of):
                proc = subprocess.Popen(['./build/'+program, str(nkeys), benchtype], stdout=subprocess.PIPE)

                # wait for the program to fill up memory and spit out its "ready" message
                try:
                    runtime = float(proc.stdout.readline().strip())
                except:
                    runtime = 0

                ps_proc = subprocess.Popen(['ps up %d | tail -n1' % proc.pid], shell=True, stdout=subprocess.PIPE)
                nbytes = int(ps_proc.stdout.read().split()[4]) * 1024
                ps_proc.wait()

                os.kill(proc.pid, signal.SIGKILL)
                proc.wait()

                if nbytes and runtime: # otherwise it crashed
                    line = ','.join(map(str, [benchtype, nkeys, program, nbytes, "%0.6f" % runtime]))

                    if runtime < fastest_attempt:
                        fastest_attempt = runtime
                        fastest_attempt_data = line

            if fastest_attempt != 1000000:
                print >> outfile, fastest_attempt_data
                print fastest_attempt_data

        nkeys *= interval
