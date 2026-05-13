

import subprocess


Import("env")



def generate_autodoc(env, **kwargs):

	config   = env.GetProjectConfig()
	template = config.get("autodoc", "template")
	exts     = config.get("autodoc", "exts")
	sources  = config.get("autodoc", "sources")
	output   = config.get("autodoc", "output")

	subprocess.run([
		"tools/gendocs.py",
		"-e", str(exts),
		"-o", str(output),
		"-r", str(template), str(sources)
	])



def generate_codecs(env, **kwargs):

	config    = env.GetProjectConfig()
	codepages = config.get("codec", "codepages")
	unify     = config.get("codec", "unify")
	group     = config.get("codec", "group")
	undefined = config.get("codec", "undefined")
	output    = config.get("codec", "output")

	subprocess.run([
		"tools/gencodec.py",
		"-o", str(output),
		"-g", str(group),
		"-d", str(undefined)
	] + (["-u"] if unify else []) + list(codepages.split()))



env.AddCustomTarget("gendocs",   None, generate_autodoc, title="Generate inline documentation")
env.AddCustomTarget("gencodecs", None, generate_codecs,  title="Generate unicode codecs")


