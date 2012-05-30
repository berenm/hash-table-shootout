# random,1310720,google_dense_hash_map,45621248,0.344362020493
# random,2621440,glib_hash_table,109867008,1.01163601875
# random,2621440,stl_unordered_map,130715648,1.73484396935
# random,2621440,boost_unordered_map,108380160,1.11585187912
# random,2621440,google_sparse_hash_map,37015552,1.76031804085
# random,2621440,google_dense_hash_map,79175680,0.504401922226
# random,5242880,glib_hash_table,210530304,1.86031603813
# random,5242880,stl_unordered_map,250298368,3.81597208977
# random,5242880,boost_unordered_map,192184320,2.63760495186
# random,5242880,google_sparse_hash_map,62066688,3.93570995331
# random,5242880,google_dense_hash_map,146284544,1.22620105743
# random,10485760,glib_hash_table,411856896,4.16937494278
# random,10485760,stl_unordered_map,490430464,7.91806197166
# random,10485760,boost_unordered_map,359251968,7.52085900307
# random,10485760,google_sparse_hash_map,111902720,8.11318516731
# random,10485760,google_dense_hash_map,280502272,2.32930994034
# random,20971520,glib_hash_table,814510080,8.32456207275
# random,20971520,stl_unordered_map,971583488,16.1606841087
# random,20971520,boost_unordered_map,692441088,24.5845990181
# random,20971520,google_sparse_hash_map,211435520,16.2772600651
# random,20971520,google_dense_hash_map,548937728,4.85360789299
# random,41943040,glib_hash_table,1619816448,90.6313672066

import sys, json

lines = [ line.strip() for line in sys.stdin if line.strip() ]

by_benchtype = {}

for i, line in enumerate(lines):
    benchtype, nkeys, program, nbytes, runtime = line.split(',')
    nkeys = int(nkeys)
    nbytes = int(nbytes)
    runtime = float(runtime)

    by_benchtype.setdefault("%s-runtime" % benchtype, {}).setdefault(program, []).append([nkeys, runtime])
    by_benchtype.setdefault("%s-runtime-per" % benchtype, {}).setdefault(program, []).append([nkeys, runtime / nkeys])
    # if benchtype.startswith('sequential'):
    by_benchtype.setdefault("%s-memory"  % benchtype, {}).setdefault(program, []).append([nkeys, nbytes])

    if not i % 2:
        nominator = runtime
    else:
        by_benchtype.setdefault("%s-ratio"  % benchtype, {}).setdefault(program, []).append([nkeys, nominator / runtime])

proper_names = {
    'boost_unordered_map': 'Boost 1.38 unordered_map',
    'stl_unordered_map': 'GCC 4.7 std::unordered_map',
    'stl_map': 'GCC 4.7 std::map',
    'stx_bplus_tree': 'Stx Btree',
    'google_sparse_hash_map': 'Google sparsehash 1.5.2 sparse_hash_map',
    'google_dense_hash_map': 'Google sparsehash 1.5.2 dense_hash_map',
    'glib_hash_table': 'Glib 2.32 GHashTable',
    'qt_qhash': 'Qt 4.5 QHash',
    'python_dict': 'Python 2.6 (C API) dict',
    'ruby_hash': 'Ruby 1.9 (C API) Hash',
    'glib_hash_table_lite': 'Glib 2.22 GHashTable Inlined',
    'bplus_tree_16': 'BPlus Tree (order 16)',
    'bplus_tree_32': 'Tree (linear)',
    'bplus_tree_64': 'BPlus Tree (order 64)',
    'bplus_tree_128': 'BPlus Tree (order 128)',
    'bplus_tree_256': 'BPlus Tree (order 256)',
    'glib_tree': 'Glib 2.32 GTree',
    'bplus_tree_dicho_16': 'B+Tree Dichotomic (order 16)',
    'bplus_tree_dicho_32': 'Tree Dichotomic (order 32)',
    'bplus_tree_dicho_64': 'B+Tree Dichotomic (order 64)',
    'bplus_tree_dicho_128': 'B+Tree Dichotomic (order 128)',
    'bplus_tree_dicho_256': 'B+Tree Dichotomic (order 256)',
    'bplus_tree2': 'Tree v2 (dichotomic)',
    'bplus_tree2_linear': 'Tree v2 (linear)',
    'bplus_tree_generic': 'Tree v2 (generic dichotomic)',
    'bplus_tree_generic_linear': 'Tree v2 (generic linear)',
}

# do them in the desired order to make the legend not overlap the chart data
# too much
program_slugs = [
    # 'google_sparse_hash_map',
    'google_dense_hash_map',
    # 'stl_unordered_map',
    # 'stl_map',
    # 'boost_unordered_map',
    # 'python_dict',
    # 'ruby_hash',
    'glib_hash_table',
    'stx_bplus_tree',
    # 'qt_qhash',
    # 'glib_hash_table_lite',
    # 'bplus_tree_16',
    'bplus_tree_32',
    # 'bplus_tree_64',
    # 'bplus_tree_128',
    # 'bplus_tree_256',
    'glib_tree',
    # 'bplus_tree_dicho_16',
    # 'bplus_tree_dicho_32',
    # 'bplus_tree_dicho_64',
    # 'bplus_tree_dicho_128',
    # 'bplus_tree_dicho_256',
    'bplus_tree2',
    'bplus_tree2_linear',
    'bplus_tree_generic',
    'bplus_tree_generic_linear',
]

chart_data = {}

for i, (benchtype, programs) in enumerate(by_benchtype.items()):
    chart_data[benchtype] = []
    for j, program in enumerate(program_slugs):
        if program in programs:
            data = programs[program]
            chart_data[benchtype].append({
                'label': proper_names[program],
                'data': [],
            })

            for k, (nkeys, value) in enumerate(data):
                chart_data[benchtype][-1]['data'].append([nkeys, value])

print 'chart_data = ' + json.dumps(chart_data)
