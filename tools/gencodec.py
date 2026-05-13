#!/usr/bin/env python3


import argparse
import sys
import struct



def perror(str):
	sys.stderr.write("gencodec: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()
	sys.exit(1)



def pwarn(str):
	sys.stderr.write("gencodec: ")
	sys.stderr.write(str)
	sys.stderr.write("\n")
	sys.stderr.flush()



def parse_command_line():

	argp = argparse.ArgumentParser(description="Generate codepage translation tables")
	argp.add_argument("-b", "--benchmark", action='store_true', help="Run benchmark")
	argp.add_argument("-o", "--output",    type=str,            help="Output file (default: -, stdout)")	
	argp.add_argument("-g", "--group",     type=int,            help="Select group size (default: 8, 256 symbols)")
	argp.add_argument("-d", "--undefined", type=int,            help="Set undefined symbol (default: 255)")
	argp.add_argument("-u", "--unify",     action='store_true', help="Generate a unified symbol table for all codepages")
	argp.add_argument("-v", "--verbose",   action='store_true', help="Summarize table stats")
	argp.add_argument('codepages', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.output:  argv.output  = "-"
	if not argv.group:   argv.group   = 8
	if not argv.undefined: argv.undefined = 255

	if not argv.codepages:
		perror("No codepages to process")

	if argv.group < 0 or argv.group > 15:
		perror(f"Group size {argv.group} out of range (0-15)")

	if argv.undefined < 0 or argv.undefined > 255:
		perror(f"Undefined character {argv.undefined} out of range (0-255)")

	return argv



def generate_symbols(codepage, undefined):
	codepoints = [ ord(struct.pack('B', ch).decode(codepage, errors='replace')) for ch in range(256) ]
	symbols    = [ codepoints.index(code) if code in codepoints else undefined for code in range(65536) ]
	symbols[0xfffd] = undefined

	return codepoints, symbols



def condense_codepoints(symbols, group, symtable):

	indices = []

	for n in range(65536 >> group):

		syms  = symbols[(n + 0) << group:(n + 1) << group]
		index = -1

		for m in range(0, len(symtable), len(syms)):
			if all((a == b) for a, b in zip(syms, symtable[m:m+len(syms)])):
				index = m
				break

		if index < 0:
			index    = len(symtable)
			symtable += syms

		indices += [ index >> group ]

	return indices, symtable



def compile_tables(codepages, group, undefined, unify):

	symtable = []
	pages    = {}

	for codepage in codepages:

		pwarn(f"Processing codepage {codepage}...")

		if not unify:
			symtable = []

		codepoints, symbols  = generate_symbols(codepage, undefined)
		grptable,   symtable = condense_codepoints(symbols, group, symtable)

		pages[codepage] = {
			'cname':      "".join(c if c in "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz" else "_" for c in codepage),
			'codepoints': codepoints,
			'grptable':   grptable,
			'symtable':   (None if unify else symtable)
		}

	return {
		'codepages': pages,
		'symtable':  (symtable if unify else None),
		'group':     group,
		'undefined': undefined
	} 



def generate_output(pages, output):

	with open("/dev/stdout" if output == "-" else output, 'w') as out:

		def p(s=""):
			print(s, file=out)

		def d(datatype, fmt, name, table):
			if not table: return
			p(f"static {datatype} {name}[{len(table)}]={{")
			p(",\n".join([ "\t" + ", ".join([
				fmt % c for c in table[r:r+16] ]) for r in range(0, len(table), 16)
			]))
			p("};")
			p()

		sym_shift = 0
		grp_shift = pages['group']
		sym_mask  = (1     << grp_shift) - 1
		grp_mask  = (65536 >> grp_shift) - 1

		p()
		p()
		p("#ifndef CODEC_PAGETABLES_H")
		p("#define CODEC_PAGETABLES_H")
		p()
		p()
		p("enum {")
		p(f"\tCODEC_SYMBOL_SHIFT = {sym_shift},")
		p(f"\tCODEC_SYMBOL_MASK  = {sym_mask},")
		p(f"\tCODEC_GROUP_SHIFT  = {grp_shift},")
		p(f"\tCODEC_GROUP_MASK   = {grp_mask},")
		p(f"\tCODEC_UNDEF_CHAR   = {pages['undefined']}")
		p("};")
		p()
		p()

		d("const u8", "%3d", "codec_global_symbol_table", pages['symtable'])

		for cc in pages['codepages']:

			pp = pages['codepages'][cc]
			cn = pp['cname']
			cp = pp['codepoints']
			gt = pp['grptable']
			st = pp['symtable']

			p()
			d("const u16", "%5d", f"codec_{cn}_codepoints",   cp)
			d("const u8",  "%3d", f"codec_{cn}_group_table",  gt)
			d("const u8",  "%3d", f"codec_{cn}_symbol_table", st)

		p()
		p("struct codec_table_entry {")
		p("\tconst char *name;")
		p("\tconst u16  *codepoints;")
		p("\tconst u16  *group_table;")
		p("\tconst u8   *symbol_table;")
		p("};")
		p()
		p()

		p("static const codec_table_entry codec_table[] = {")

		for cc in pages['codepages']:
			pp = pages['codepages'][cc]
			cn = pp['cname']
			cp = f"codec_{cn}_codepoints"
			gt = f"codec_{cn}_group_table"
			st = f"codec_{cn}_symbol_table" if pp['symtable'] else "codec_global_symbol_table"
			p(f"\t{{ \"{cn}\", {cp}, {gt}, {st} }},")

		p("\t{ NULL, NULL, NULL, NULL }")
		p("};")
		p()
		p()
		p("#endif")
		p()
		p()



def generate_report(pages):
	pass



def benchmark_unified(codepages):

	pwarn("Unified benchmark")

	for group in range(16):

		symtable = []

		idx_num = 0
		sym_num = 0

		for codepage in codepages:

			codepoints, symbols  = generate_symbols(codepage, 255)
			indices,    symtable = condense_codepoints(symbols, group, symtable)

			sym_num += len(codepoints)
			idx_num += len(indices)
	
			print(f"{codepage}, {group}, {len(indices)}, {len(symtable)}, {len(indices)+len(symtable)}")

		pwarn(f"Level totals: {group}: {idx_num}, {len(symtable)}, {sym_num}, {idx_num+len(symtable)}")



def benchmark_split(codepages):

	pwarn("Split benchmark")

	for group in range(16):

		tbl_num = 0
		idx_num = 0
		sym_num = 0

		for codepage in codepages:

			codepoints, symbols  = generate_symbols(codepage, 255)
			indices,    symtable = condense_codepoints(symbols, group, [])

			tbl_num += len(symtable)
			sym_num += len(codepoints)
			idx_num += len(indices)
	
			print(f"{codepage}, {group}, {len(indices)}, {len(symtable)}, {len(indices)+len(symtable)}")

		pwarn(f"Level totals: {group}: {idx_num}, {tbl_num}, {sym_num}, {idx_num+tbl_num}")



argv = parse_command_line()


if argv.benchmark:
	if argv.unify: benchmark_unified(argv.codepages)
	else:          benchmark_split(argv.codepages)

else:
	pages = compile_tables(argv.codepages, argv.group, argv.undefined, argv.unify)

	generate_output(pages, argv.output)

	if argv.verbose:
		generate_report(pages)


