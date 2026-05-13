#!/usr/bin/env python3


import argparse
import contextlib
import jinja2
import os
import sys
import types


DOC_MARKERS = [ "*:", "/:", ";:", "#:" ]
DOC_TOPIC   = "@topic"
DOC_WIDTH   = 120



def parse_command_line():

	argp = argparse.ArgumentParser(description="Generate inline documentation")
	argp.add_argument("-o", "--output",     type=str,            help="Output file (default: -)")	
	argp.add_argument("-e", "--extensions", type=str,            help="List of file extensions to accept (c;cpp;h)")
	argp.add_argument("-r", "--recursive",  action='store_true', help="Use tabs for indentation (8)")
	argp.add_argument('template')
	argp.add_argument('paths', nargs=argparse.REMAINDER)

	argv = argp.parse_args()

	if not argv.output:     argv.output     = "-"
	if not argv.extensions: argv.extensions = "c;cpp;h;s"

	return argv



def gather_files(paths, exts, recurse):

	files = []
	exts  = [ f".{x}" for x in exts.split(";") ]

	for path in paths:
		for root, _, names in os.walk(path):
			for file in [ os.path.join(root, name) for name in names if any(name.endswith(ext) for ext in exts)]:

				if not file in files:
					files.append(file)

			if not recurse:
				break

	return files

	

def extract_text(files):

	texts  = []
	origin = -1


	for filename in files:
		with open(filename, 'r') as src:
			for lineno, line in enumerate(src.readlines()):

				if not any(m in line for m in DOC_MARKERS):
					origin = -1
					continue

				if '@topic' in line:

					origin = line.index(DOC_TOPIC)

					texts += [
						types.SimpleNamespace(
							file   = filename,
							line   = lineno + 1,
							topics = line[origin + len(DOC_TOPIC):].split(),
							docs   = []
						)
					]
							
				elif origin >= 0:
					texts[-1].docs += [ line[origin:].rstrip() ]

	return texts



def collate_topics(texts):

	table  = ""
	topics = []


	def table_add(s):
		nonlocal table
		if s not in table:
			table += s
		return table.index(s), len(s)


	for text in texts:

		dptr, dlen = table_add("\n".join(text.docs))

		for topic in text.topics:

			tptr, tlen = table_add(topic)

			topics += [ types.SimpleNamespace(
				name    = topic,
	      			file    = text.file,
	      			line    = text.line,
	      			topic_p = tptr,
	      			topic_l = tlen,
	      			text_p  = dptr,
	      			text_l  = dlen
	      		)]


	return types.SimpleNamespace(table=table,topics=topics)



def generate_output(topics, template, out):


	def escape(s, n):

		lines = []
		text  = ""

		for ch in s:

			text += ch.encode('unicode-escape').decode('utf-8')

			if (n > 0) and (len(text) > n):
				lines += [ text ]
				text = ""

		if text:
			   lines += [ text ]

		return lines
			

	env = jinja2.Environment(loader=jinja2.FileSystemLoader("."))
	env.globals['escape'] = escape

	res = env.get_template(template).render(topics=topics.topics, stringtable=topics.table)

	with contextlib.redirect_stdout(out):
		print(res)



argv   = parse_command_line()
files  = gather_files(argv.paths, argv.extensions, argv.recursive)
texts  = extract_text(files)
topics = collate_topics(texts)


if argv.output == "-":
	generate_output(topics, argv.template, sys.stdout)

else:
	with open(argv.output, 'w') as out:
		generate_output(topics, argv.template, out)


