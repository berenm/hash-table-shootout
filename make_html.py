import sys
html_template = file('charts-template.html', 'r').read()
file('charts-%s.html' % sys.argv[1], 'w').write(html_template.replace('__CHART_DATA_GOES_HERE__', sys.stdin.read()))
